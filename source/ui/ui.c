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
    ui->AutoLayoutStack[index].ProgressX = 0.0f;
    ui->AutoLayoutStack[index].ProgressY = 0.0f;
    ui->AutoLayoutStack[index].IsRow = 1;
    ui->AutoLayoutStack[index].ElementsInRow = 0;
    ui->AutoLayoutStack[index].MaxElementsPerRow = MaxElementsPerRow;
}

internal void CrestUIPopRow(CrestUI * ui) {
    --ui->AutoLayoutStackPosition;
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
            ui->AutoLayoutStack[index].ProgressX += rect.width;
        }
        else {
            //TODO(Zen): Support for columns?
        }

        if(++ui->AutoLayoutStack[index].ElementsInRow >= ui->AutoLayoutStack[index].MaxElementsPerRow) {
            ui->AutoLayoutStack[index].ProgressX = 0.0f;
            ui->AutoLayoutStack[index].ProgressY += rect.height;
            ui->AutoLayoutStack[index].ElementsInRow = 0;
        }
    }
    else {
        //TODO(Zen): Logging
        rect = v4(0, 0, 64, 64);
    }

    return rect;
}

//Note(Zen): UI functions
//~
internal void
CrestUIBeginFrame(CrestUI * ui, CrestUIInput * input, ui_renderer * UIRenderer) {
    ui->Count = 0;

    ui->MouseX = input->MouseX;
    ui->MouseY = input->MouseY;
    ui->LeftMouseDown = input->LeftMouseDown;
    ui->RightMouseDown = input->RightMouseDown;
    CrestUIRendererStartFrame(UIRenderer);
}

internal void
CrestUIEndFrame(CrestUI *ui, ui_renderer * Renderer) {
    for(u32 i = 0; i < ui->Count; ++i) {
        CrestUIWidget * Widget = ui->Widgets + i;
        //Note(Zen): 11px per char in LiberationMono font
        v2 TextOffset = v2(-11.0f * ((r32)strlen(Widget->Text))/2.0f,
                           -10.0f);
        switch (Widget->Type) {
            case CREST_UI_BUTTON: {
                v4 colour = CrestUIIDEquals(ui->hot, Widget->id) ? BUTTON_HOVER_COLOUR : BUTTON_COLOUR;

                CrestPushFilledRect(Renderer, colour, v2(Widget->rect.x, Widget->rect.y), v2(Widget->rect.width, Widget->rect.height));
                CrestPushBorder(Renderer, BORDER_COLOUR, v2(Widget->rect.x, Widget->rect.y), v2(Widget->rect.width, Widget->rect.height));

                CrestPushText(Renderer, v2(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x, Widget->rect.y + Widget->rect.height +TextOffset.y), Widget->Text);
            } break;

            case CREST_UI_SLIDER: {
                CrestPushFilledRect(Renderer, BUTTON_COLOUR, v2(Widget->rect.x, Widget->rect.y), v2(Widget->rect.width, Widget->rect.height));
                CrestPushFilledRect(Renderer, BUTTON_HOVER_COLOUR, v2(Widget->rect.x, Widget->rect.y), v2(Widget->rect.width * Widget->Value, Widget->rect.height));
                CrestPushBorder(Renderer, BORDER_COLOUR, v2(Widget->rect.x, Widget->rect.y), v2(Widget->rect.width, Widget->rect.height));

                CrestPushText(Renderer, v2(Widget->rect.x + Widget->rect.width/2.0f + TextOffset.x, Widget->rect.y + Widget->rect.height +TextOffset.y), Widget->Text);
            }
        }
    }
    CrestUIRender(Renderer);
}

internal b32
CrestUIButtonP(CrestUI *ui, CrestUIID ID, v4 rect, char * Text) {
    b32 Pressed = 0;

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width&&
                       ui->MouseY <= rect.y + rect.height);

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver) {
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
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown) {
            ui->active = ID;
        }
    }

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_BUTTON;
    Widget->rect = rect;
    strcpy(Widget->Text, Text);

    return Pressed;
}

internal b32
CrestUIButton(CrestUI *ui, CrestUIID ID, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    return CrestUIButtonP(ui, ID, rect, Text);
}

internal r32
CrestUISliderP(CrestUI * ui, CrestUIID ID, r32 value, v4 rect, char * Text) {
    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.width &&
                       ui->MouseY <= rect.y + rect.height);

    if(!CrestUIIDEquals(ui->hot, ID) && MouseOver) {
        ui->hot = ID;
    }
    else if(CrestUIIDEquals(ui->hot, ID) && !MouseOver) {
        ui->hot = CrestUIIDNull();
    }

    if(CrestUIIDEquals(ui->active, ID)) {
        if(ui->LeftMouseDown) {
            value = (ui->MouseX - rect.x) / rect.width;
            ui->active = CrestUIIDNull();
        }
        if(!CrestUIIDEquals(ui->hot, ID)) {
            ui->active = CrestUIIDNull();
        }
    }
    else {
        if(CrestUIIDEquals(ui->hot, ID) && ui->LeftMouseDown) {
            ui->active = ID;
        }
    }

    if(value > 1.f) value = 1.f;
    if(value < 0.f) value = 0.f;

    CrestUIWidget *Widget = ui->Widgets + ui->Count++;
    Widget->id = ID;
    Widget->Type = CREST_UI_SLIDER;
    Widget->rect = rect;
    Widget->Value = value;
    strcpy(Widget->Text, Text);

    return value;
}

internal r32
CrestUISlider(CrestUI * ui, CrestUIID ID, r32 value, char * Text) {
    v4 rect = GetNextAutoLayoutPosition(ui);
    return CrestUISliderP(ui, ID, value, rect, Text);
}
