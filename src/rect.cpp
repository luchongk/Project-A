#include "rect.h"

Rect relative_to_screen(Rect parent, Rect rect) {
    Rect result;
    result.x = parent.x + rect.x * parent.width;
    result.y = parent.y + rect.y * parent.height;
    result.width  = parent.width * rect.width;
    result.height = parent.height * rect.height;

    return result;
}

Rect screen_to_relative(Rect parent, Rect rect) {
    Rect result;
    result.x = (rect.x - parent.x) / parent.width;
    result.y = (rect.y - parent.y) / parent.height;
    result.width  = rect.width / parent.width;
    result.height = rect.width / parent.height;

    return result;
}

Rect cut_top(Rect* to_cut, float percentage) {
    Rect result;
    result.x      = to_cut->x;
    result.y      = to_cut->y;
    result.width  = to_cut->width;
    result.height = to_cut->height * percentage;
    
    to_cut->y      += result.height;
    to_cut->height -= result.height;

    return result;
}

Rect cut_bottom(Rect* to_cut, float percentage) {
    Rect result;
    result.height  = to_cut->height * percentage;
    to_cut->height -= result.height;
    
    result.x      = to_cut->x;
    result.y      = to_cut->y + to_cut->height;
    result.width  = to_cut->width;
    
    return result;
}

Rect cut_left(Rect* to_cut, float percentage) {
    Rect result;
    result.x      = to_cut->x;
    result.y      = to_cut->y;
    result.width  = to_cut->width * percentage;
    result.height = to_cut->height;
    
    to_cut->x     += result.width;
    to_cut->width -= result.width;

    return result;
}

Rect cut_right(Rect* to_cut, float percentage) {
    Rect result;
    result.width   = to_cut->width * percentage;
    to_cut->width -= result.width;
    
    result.x      = to_cut->x + to_cut->width;
    result.y      = to_cut->y;
    result.height = to_cut->height;
    
    return result;
}

Rect pad_rect(Rect to_pad, float percentage) {
    Rect result;
    result.width  = to_pad.width - to_pad.width * percentage;
    result.height = to_pad.height - to_pad.height * percentage;
    result.x = to_pad.x + (to_pad.width  - result.width ) / 2;
    result.y = to_pad.y + (to_pad.height - result.height) / 2;

    return result;
}

Rect pad_leftright(Rect to_pad, float percentage) {
    Rect result;
    result.width  = to_pad.width - to_pad.width * percentage;
    result.height = to_pad.height;
    result.x = to_pad.x + (to_pad.width  - result.width ) / 2;
    result.y = to_pad.y;

    return result;
}

Rect pad_topbottom(Rect to_pad, float percentage) {
    Rect result;
    result.width  = to_pad.width;
    result.height = to_pad.height - to_pad.height * percentage;
    result.x = to_pad.x;
    result.y = to_pad.y + (to_pad.height - result.height) / 2;

    return result;
}