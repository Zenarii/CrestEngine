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
CrestUIButton(CrestUI *ui, CrestUIID ID, v4 rect, char * Text) {
    b32 Pressed = 0;

    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.x + rect.width&&
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

internal r32
CrestUISlider(CrestUI * ui, CrestUIID ID, v4 rect, r32 value, char * Text) {
    b32 MouseOver = (ui->MouseX >= rect.x &&
                       ui->MouseY >= rect.y &&
                       ui->MouseX <= rect.x + rect.x + rect.width&&
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
