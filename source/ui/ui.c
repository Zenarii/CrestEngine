#include "ui.h"


//Note(Zen): UI id
//~
internal CrestUIID
CrestUIIDInit(u32 Primary, u32 Secondary) {
    CrestUIID id = {Primary, Secondary};
    return id;
}

internal CrestUIID
CrestUIIDNull(void) {
    CrestUIID id = {0, 0};
    return id;
}

internal b32
CrestUIIDEquals(CrestUIID ID1, CrestUIID ID2) {
    return ((ID1.Primary == ID2.Primary) && (ID2.Secondary == ID1.Secondary));
}
//Note(Zen): Layout Control
//~

internal void
CrestUIPushRow(CrestUI * ui, v2 Position, v2 Size, u32 MaxElementsPerRow) {
    Assert(ui->AutoLayoutStackPosition < CREST_UI_MAX_STACKED_ROWS);

    u32 index = ui->AutoLayoutStackPosition++;

    ui->AutoLayoutStack[index].Position = Position;
    ui->AutoLayoutStack[index].Size = Size;
    ui->AutoLayoutStack[index].ProgressX = DefaultStyle.Padding.x;
    ui->AutoLayoutStack[index].ProgressY = DefaultStyle.Padding.y;
    ui->AutoLayoutStack[index].IsRow = 1;
    ui->AutoLayoutStack[index].ElementsInRow = 0;
    ui->AutoLayoutStack[index].MaxElementsPerRow = MaxElementsPerRow;

    if(ui->PanelStackPosition) {
        ui->AutoLayoutStack[index].ProgressY += ui->PanelStack[ui->PanelStackPosition-1].Height;
    }
}

internal void CrestUIPopRow(CrestUI * ui) {
    u32 index = --ui->AutoLayoutStackPosition;
    Assert((index >= 0) && (index <= 16));

    if(ui->PanelStackPosition ) {
        //Note(Zen): If Elements in row = 0, the current row is empty
        if(ui->AutoLayoutStack[index].ElementsInRow != 0) {
            if(ui->PanelStack[ui->PanelStackPosition-1].Height <= ui->AutoLayoutStack[index].ProgressY + ui->AutoLayoutStack[index].Size.y + DefaultStyle.Padding.y) {
                ui->PanelStack[ui->PanelStackPosition-1].Height = ui->AutoLayoutStack[index].ProgressY + ui->AutoLayoutStack[index].Size.y + DefaultStyle.Padding.y;
            }
        }
        else {
            ui->PanelStack[ui->PanelStackPosition-1].Height = ui->AutoLayoutStack[index].ProgressY /*+ ui->AutoLayoutStack[index].Size.y + DefaultStyle.Padding.y*/;
        }
    }
}

internal v4
GetNextAutoLayoutPosition(CrestUI * ui) {
    v4 rect = {0};



    if(ui->AutoLayoutStackPosition > 0) {
        u32 index = ui->AutoLayoutStackPosition - 1;
        rect.x = ui->AutoLayoutStack[index].Position.x;
        rect.y = ui->AutoLayoutStack[index].Position.y;
        rect.width = ui->AutoLayoutStack[index].Size.x;
        rect.height = ui->AutoLayoutStack[index].Size.y;

        if(ui->AutoLayoutStack[index].IsRow) {
            rect.x += ui->AutoLayoutStack[index].ProgressX;
            rect.y += ui->AutoLayoutStack[index].ProgressY;
            ui->AutoLayoutStack[index].ProgressX += rect.width + DefaultStyle.Padding.x;
        }
        else {

        }

        if(ui->AutoLayoutStack[index].ElementsInRow == 0 && ui->PanelStackPosition) {
            ui->PanelStack[ui->PanelStackPosition-1].Height += rect.height;
        }

        //Note(Zen): Overflow to the next row
        if(++ui->AutoLayoutStack[index].ElementsInRow >= ui->AutoLayoutStack[index].MaxElementsPerRow) {
            ui->AutoLayoutStack[index].ProgressY += rect.height + DefaultStyle.Padding.y;
            ui->AutoLayoutStack[index].ElementsInRow = 0;

            //Note(Zen): Increase Panel Size
            if(ui->PanelStackPosition) {
                if(ui->AutoLayoutStack[index].ProgressX > ui->PanelStack[ui->PanelStackPosition-1].Width) {
                    ui->PanelStack[ui->PanelStackPosition-1].Width = ui->AutoLayoutStack[index].ProgressX;
                }

            }
            //Note(Zen): Must go here or interferes with Panel calculations
            ui->AutoLayoutStack[index].ProgressX = DefaultStyle.Padding.x;
        }
    }
    else {
        //TODO(Zen): Logging
        rect = v4(0, 0, 64, 64);
    }

    return rect;
}

internal void
CrestUIPushPanel(CrestUI * UI, v2 Position, r32 Precedence) {
    Assert(UI->PanelStackPosition < CREST_UI_MAX_PANELS);

    u32 index = UI->PanelStackPosition++;

    UI->PanelStack[index].Position = Position;
    UI->PanelStack[index].Rows = 0;
    UI->PanelStack[index].Width = 0;
    UI->PanelStack[index].Height = 0;
    UI->PanelStack[index].Precedence = Precedence;
}

internal void
CrestUIPopPanel(CrestUI * UI) {
    u32 index = --UI->PanelStackPosition;

    //calculate the rect
    //draw panel based on number of rows
    v3 Position = v3(UI->PanelStack[index].Position.x, UI->PanelStack[index].Position.y, -0.1f);
    v2 Size = v2(UI->PanelStack[index].Width, UI->PanelStack[index].Height);

    CrestUIWidget *Widget = UI->Widgets + UI->Count++;
    Widget->id = CrestUIIDNull();
    Widget->Type = CREST_UI_PANEL;
    Widget->Precedence = UI->PanelStack[index].Precedence;
    Widget->rect = v4(Position.x, Position.y, Size.x, Size.y);

    b32 MouseOver = (UI->MouseX >= Widget->rect.x &&
                       UI->MouseY >= Widget->rect.y &&
                       UI->MouseX <= Widget->rect.x + Widget->rect.width &&
                       UI->MouseY <= Widget->rect.y + Widget->rect.height);
    if(MouseOver) UI->IsMouseOver = 1;
}


//Note(Zen): UI functions
//~
internal void
CrestUIBeginFrame(CrestUI * ui, CrestUIInput * input, ui_renderer * UIRenderer) {
    ui->Count = 0;

    ui->MouseX = input->MouseX;
    ui->MouseY = input->MouseY;
    ui->MouseStartX = input->MouseStartX;
    ui->MouseStartY = input->MouseStartY;
    ui->LeftMouseDown = input->LeftMouseDown;
    ui->LeftMouseWasDown = input->LeftMouseWasDown;
    ui->RightMouseDown = input->RightMouseDown;
    ui->RightMouseWasDown = input->RightMouseWasDown;
    ui->IsMouseOver = 0;

    CrestUIRendererStartFrame(UIRenderer);
}

internal void
CrestUIEndFrame(CrestUI *ui, ui_renderer * Renderer) {
    for(u32 i = 0; i < ui->Count; ++i) {
        CrestUIWidget * Widget = ui->Widgets + i;
        //Note(Zen): 11px per char in LiberationMono font (ish)
        v2 TextOffset = v2(-11.0f * ((r32)strlen(Widget->Text))/2.0f + 5.f,
                           -10.0f);
        if(Widget->TextFloat == CREST_UI_LEFT) {
            TextOffset.x = -0.5f * Widget->rect.width;
        }

        switch (Widget->Type) {
            case CREST_UI_BUTTON: {
                v4 colour = CrestUIIDEquals(ui->hot, Widget->id) ? DefaultStyle.ButtonHotColour : DefaultStyle.ButtonColour;

                CrestPushFilledRectD(Renderer, colour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.02f), v2(Widget->rect.width, Widget->rect.height));
                CrestPushBorder(Renderer, DefaultStyle.ButtonBorderColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.03f), v2(Widget->rect.width, Widget->rect.height));

                CrestPushText(Renderer, v3(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x, Widget->rect.y + Widget->rect.height + TextOffset.y, Widget->Precedence - 0.05f), Widget->Text);
            } break;

            case CREST_UI_TOGGLE_BUTTON: {
                v4 colour = Widget->On ? DefaultStyle.ToggleButtonColour : DefaultStyle.ButtonColour;
                v2 ToggleBoxSize = v2(Widget->rect.height - DefaultStyle.Padding.y,
                                      Widget->rect.height - DefaultStyle.Padding.y);
                v2 CheckBoxPosition = v2(Widget->rect.width + Widget->rect.x - ToggleBoxSize.x,
                                         Widget->rect.y + DefaultStyle.Padding.y * 0.5f);


                CrestPushFilledRectD(Renderer, colour, v3(CheckBoxPosition.x, CheckBoxPosition.y, Widget->Precedence - 0.02f), ToggleBoxSize);
                CrestPushBorder(Renderer, DefaultStyle.ButtonBorderColour, v3(CheckBoxPosition.x, CheckBoxPosition.y, Widget->Precedence - 0.03f), ToggleBoxSize);

                CrestPushText(Renderer, v3(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x + 2.f, Widget->rect.y + Widget->rect.height + TextOffset.y, Widget->Precedence - 0.05f), Widget->Text);
            } break;

            case CREST_UI_SLIDER: {
                CrestPushFilledRectD(Renderer, DefaultStyle.ButtonColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.02f), v2(Widget->rect.width, Widget->rect.height));
                CrestPushFilledRectD(Renderer, DefaultStyle.ButtonHotColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.02f), v2(Widget->rect.width * Widget->Value, Widget->rect.height));
                CrestPushBorder(Renderer, DefaultStyle.ButtonBorderColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.03f), v2(Widget->rect.width, Widget->rect.height));

                CrestPushText(Renderer, v3(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x, Widget->rect.y + Widget->rect.height + TextOffset.y, Widget->Precedence - 0.05f), Widget->Text);
            } break;

            case CREST_UI_HEADER: {
                CrestPushFilledRectD(Renderer, DefaultStyle.HeaderColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.04f), v2(Widget->rect.width, Widget->rect.height));
                CrestPushBorder(Renderer, DefaultStyle.HeaderBorderColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.03f), v2(Widget->rect.width, Widget->rect.height));

                //Note(Zen): For now header has text float left.
                CrestPushText(Renderer, v3(Widget->rect.x + DefaultStyle.Padding.x, Widget->rect.y + Widget->rect.height + TextOffset.y, Widget->Precedence - 0.05f), Widget->Text);
            } break;

            case CREST_UI_PANEL: {
                CrestPushTransparentRect(Renderer, DefaultStyle.PanelColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.01f), v2(Widget->rect.width, Widget->rect.height));
                CrestPushBorder(Renderer, DefaultStyle.HeaderBorderColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.03f), v2(Widget->rect.width, Widget->rect.height));
            } break;

            case CREST_UI_TEXTLABEL: {
                CrestPushText(Renderer, v3(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x, Widget->rect.y + Widget->rect.height + TextOffset.y, Widget->Precedence - 0.05f), Widget->Text);
            } break;

            case CREST_UI_TEXT_EDIT: {
                CrestPushFilledRectD(Renderer, DefaultStyle.ButtonColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.02f), v2(Widget->rect.width, Widget->rect.height));
                CrestPushFilledRectD(Renderer, DefaultStyle.ButtonHotColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.02f), v2(Widget->rect.width * Widget->Value, Widget->rect.height));
                CrestPushBorder(Renderer, DefaultStyle.ButtonBorderColour, v3(Widget->rect.x, Widget->rect.y, Widget->Precedence - 0.03f), v2(Widget->rect.width, Widget->rect.height));

                CrestPushText(Renderer, v3(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x, Widget->rect.y + Widget->rect.height + TextOffset.y, Widget->Precedence - 0.05f), Widget->Text);
            } break;
        }
    }
    CrestUIRender(Renderer);
}

internal b32
CrestUIButtonP(CrestUI *ui, CrestUIID ID, v4 rect, char * Text) {
    b32 Pressed = 0;

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width &&
                       ui->MouseY <= rect.y + rect.height);
    ui->IsMouseOver = MouseOver ? 1 : ui->IsMouseOver;

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver && CrestUIIDEquals(CrestUIIDNull(), ui->hot)) {
        ui->hot = ID;
    }
    else if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    if(CrestUIIDEquals(ui->active, ID)) {
        if(!ui->LeftMouseDown) {
            Pressed = CrestUIIDEquals(ui->hot, ID);
            ui->active = CrestUIIDNull();
            ui->hot = CrestUIIDNull();
        }
        if(!CrestUIIDEquals(ui->hot, ID)) {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown && !ui->LeftMouseWasDown && CrestUIIDEquals(CrestUIIDNull(), ui->active)) {
            ui->active = ID;
        }
    }

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_BUTTON;
    Widget->TextFloat = CREST_UI_CENTRE;
    Widget->rect = rect;
    strcpy(Widget->Text, Text);

    if(ui->PanelStackPosition) {
        Widget->Precedence = ui->PanelStack[ui->PanelStackPosition-1].Precedence;
    }
    else {
        Widget->Precedence = 0.f;
    }

    return Pressed;
}

internal b32
CrestUIButton(CrestUI *ui, CrestUIID ID, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    return CrestUIButtonP(ui, ID, rect, Text);
}

internal b32
CrestUIToggleButtonP(CrestUI * ui, CrestUIID ID, v4 rect, b32 On, char * Text) {
    b32 Pressed = 0;

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width&&
                       ui->MouseY <= rect.y + rect.height);
    ui->IsMouseOver = MouseOver ? 1 : ui->IsMouseOver;

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver && CrestUIIDEquals(CrestUIIDNull(), ui->hot)) {
        ui->hot = ID;
    }
    else if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    if(CrestUIIDEquals(ui->active, ID)) {
        if(!ui->LeftMouseDown) {
            Pressed = CrestUIIDEquals(ui->hot, ID);
            ui->active = CrestUIIDNull();
        }
        if(!CrestUIIDEquals(ui->hot, ID)) {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown && CrestUIIDEquals(CrestUIIDNull(), ui->active)) {
            ui->active = ID;
        }
    }

    if(Pressed) On = !On;

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_TOGGLE_BUTTON;
    Widget->TextFloat = CREST_UI_LEFT;
    Widget->rect = rect;
    Widget->On = On;
    strcpy(Widget->Text, Text);

    if(ui->PanelStackPosition) {
        Widget->Precedence = ui->PanelStack[ui->PanelStackPosition-1].Precedence;
    }
    else {
        Widget->Precedence = 0.f;
    }


    return On;
}


internal b32
CrestUIToggleButton(CrestUI * ui, CrestUIID ID, b32 On, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    return CrestUIToggleButtonP(ui, ID, rect, On, Text);
}

internal r32
CrestUISliderP(CrestUI * ui, CrestUIID ID, r32 value, v4 rect, char * Text) {
    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width &&
                       ui->MouseY <= rect.y + rect.height);
    ui->IsMouseOver = MouseOver ? 1 : ui->IsMouseOver;

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver && CrestUIIDEquals(CrestUIIDNull(), ui->hot)) {
        ui->hot = ID;
    }
    else if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    if(CrestUIIDEquals(ui->active, ID)) {
        if(ui->LeftMouseDown) {
            value = (ui->MouseX - rect.x) / rect.width;
            //ui->active = CrestUIIDNull();
        }
        if(!CrestUIIDEquals(ui->hot, ID)) {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown && CrestUIIDEquals(CrestUIIDNull(), ui->active)) {
            ui->active = ID;
        }
    }

    if(value > 1.f) value = 1.f;
    if(value < 0.f) value = 0.f;

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_SLIDER;
    Widget->TextFloat = CREST_UI_CENTRE;
    Widget->rect = rect;
    Widget->Value = value;
    strcpy(Widget->Text, Text);

    if(ui->PanelStackPosition) {
        Widget->Precedence = ui->PanelStack[ui->PanelStackPosition-1].Precedence;
    }
    else {
        Widget->Precedence = 0.f;
    }

    return value;
}

internal r32
CrestUISlider(CrestUI * ui, CrestUIID ID, r32 value, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    return CrestUISliderP(ui, ID, value, rect, Text);
}

internal i32
CrestUISliderIntP(CrestUI * ui, CrestUIID ID, v4 rect, i32 Value, i32 Max, char * Text) {
    r32 FloatValue = (r32)Value/(r32)Max;

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width &&
                       ui->MouseY <= rect.y + rect.height);
    ui->IsMouseOver = MouseOver ? 1 : ui->IsMouseOver;

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver && CrestUIIDEquals(CrestUIIDNull(), ui->hot)) {
        ui->hot = ID;
    }
    else if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    if(CrestUIIDEquals(ui->active, ID)) {
        if(ui->LeftMouseDown) {
            FloatValue = (ui->MouseX - rect.x) / rect.width;
            //ui->active = CrestUIIDNull();
        }
        if(!CrestUIIDEquals(ui->hot, ID)) {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown && CrestUIIDEquals(CrestUIIDNull(), ui->active)) {
            ui->active = ID;
        }
    }

    if(FloatValue > 1.f) FloatValue = 1.f;
    if(FloatValue < 0.f) FloatValue = 0.f;

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_SLIDER;
    Widget->TextFloat = CREST_UI_CENTRE;
    Widget->rect = rect;

    strcpy(Widget->Text, Text);

    if(ui->PanelStackPosition) {
        Widget->Precedence = ui->PanelStack[ui->PanelStackPosition-1].Precedence;
    }
    else {
        Widget->Precedence = 0.f;
    }

    Value = (FloatValue * Max + 0.5f);
    FloatValue = Value /(r32)Max;
    Widget->Value = FloatValue;

    return Value;
}


internal i32
CrestUISliderInt(CrestUI * ui, CrestUIID ID, i32 Value, i32 Max, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    return CrestUISliderIntP(ui, ID, rect, Value, Max, Text);
}

internal v2
CrestUIDnDBoxP(CrestUI *ui, CrestUIID ID, r32 Precedence, v4 rect, char * Text) {
    v2 Position = v2(rect.x, rect.y);
    v2 MouseOffset = {0};

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width&&
                       ui->MouseY <= rect.y + rect.height);
    ui->IsMouseOver = MouseOver ? 1 : ui->IsMouseOver;

    if(CrestUIIDEquals(ui->active, ID)) {
        if(ui->LeftMouseDown) {
            MouseOffset = v2(ui->MouseStartX - rect.x, ui->MouseStartY - rect.y);
            Position.x = ui->MouseX - MouseOffset.x;
            Position.y = ui->MouseY - MouseOffset.y;
        }
        else {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown && CrestUIIDEquals(CrestUIIDNull(), ui->active)) {
            ui->active = ID;
        }
    }


    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver && CrestUIIDEquals(CrestUIIDNull(), ui->hot)) {
        ui->hot = ID;
    }

    if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown) {
        ui->active = ID;
    }

    if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_HEADER;
    Widget->TextFloat = CREST_UI_LEFT;
    Widget->rect = v4(Position.x, Position.y, rect.width, rect.height);
    Widget->Precedence = Precedence;
    strcpy(Widget->Text, Text);

    return Position;
}

internal void
CrestUITextLabelP(CrestUI * ui, CrestUIID ID, v4 rect, char * Text) {
    CrestUIWidget * Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_TEXTLABEL;
    Widget->TextFloat = CREST_UI_LEFT;
    Widget->rect = rect;
    strcpy(Widget->Text, Text);
    if(ui->PanelStackPosition) {
        Widget->Precedence = ui->PanelStack[ui->PanelStackPosition-1].Precedence;
    }
    else {
        Widget->Precedence = 0.f;
    }
}

internal void
CrestUITextLabel(CrestUI * ui, CrestUIID ID, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    CrestUITextLabelP(ui, ID, rect, Text);
}

internal void
CrestUITextEditP(CrestUI * ui, CrestUIID ID, v4 rect, char * Text) {
    b32 Pressed = 0;

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width&&
                       ui->MouseY <= rect.y + rect.height);
    ui->IsMouseOver = MouseOver ? 1 : ui->IsMouseOver;

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver && CrestUIIDEquals(CrestUIIDNull(), ui->hot)) {
        ui->hot = ID;
    }
    else if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    if(CrestUIIDEquals(ui->active, ID)) {
        if(!ui->LeftMouseDown) {
            Pressed = CrestUIIDEquals(ui->hot, ID);
            ui->active = CrestUIIDNull();
            ui->keyfocus = ID;
        }
        if(!CrestUIIDEquals(ui->hot, ID)) {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown && CrestUIIDEquals(CrestUIIDNull(), ui->active)) {
            ui->active = ID;
        }
    }

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_TEXT_EDIT;
    Widget->TextFloat = CREST_UI_LEFT;
    Widget->rect = rect;
    strcpy(Widget->Text, Text);

    if(ui->PanelStackPosition) {
        Widget->Precedence = ui->PanelStack[ui->PanelStackPosition-1].Precedence;
    }
    else {
        Widget->Precedence = 0.f;
    }
}

internal void
CrestUITextEdit(CrestUI * ui, CrestUIID ID, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    CrestUITextEditP(ui, ID, rect, Text);
}
