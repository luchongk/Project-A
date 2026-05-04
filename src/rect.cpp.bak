#include "rect.h"

Rect relative_to_screen(Rect parent, Rect rect) {
    Rect result;
    result.x = parent.x + rect.x * parent.w;
    result.y = parent.y + rect.y * parent.h;
    result.w = parent.w * rect.w;
    result.h = parent.h * rect.h;

    return result;
}

Rect screen_to_relative(Rect parent, Rect rect) {
    Rect result;
    result.x = (rect.x - parent.x) / parent.w;
    result.y = (rect.y - parent.y) / parent.h;
    result.w  = rect.w / parent.w;
    result.h = rect.w / parent.h;

    return result;
}

Rect cut_left(Rect* rect, float amount) {
    float x = rect->x;
    float w = min(amount, rect->w);
    rect->x += w;
    rect->w -= w;
    return Rect{x, rect->y, w, rect->h};
}

Rect cut_right(Rect* rect, float amount) {
    float w = min(amount, rect->w);
    rect->w -= w;
    return Rect{rect->x + rect->w, rect->y, w, rect->h};
}

Rect cut_top(Rect* rect, float amount) {
    float y = rect->y;
    float h = min(amount, rect->h);
    rect->y += h;
    rect->h -= h;
    return Rect{rect->x, y, rect->w, h};
}

Rect cut_bottom(Rect* rect, float amount) {
    float h = min(amount, rect->h);
    rect->h -= h;
    return Rect{rect->x, rect->y + rect->h, rect->w, h};
}

Rect pad_rect(Rect to_pad, float amount) {
    Rect result;
    result.w = to_pad.w - 2 * amount;
    result.h = to_pad.h - 2 * amount;
    result.x = to_pad.x + amount;
    result.y = to_pad.y + amount;

    return result;
}

Rect pad_left_right(Rect to_pad, float amount) {
    Rect result;
    result.w = to_pad.w - 2 * amount;
    result.h = to_pad.h;
    result.x = to_pad.x + amount;
    result.y = to_pad.y;

    return result;
}

Rect pad_top_bottom(Rect to_pad, float amount) {
    Rect result;
    result.w = to_pad.w;
    result.h = to_pad.h - 2 * amount;
    result.x = to_pad.x;
    result.y = to_pad.y + amount;

    return result;
}