#ifndef RECT_H
#define RECT_H

struct Rect {
    union {
        struct { float x, y; };
        Vec2 origin; //Bottom left
    };
    float w, h;
};

Rect relative_to_screen(Rect parent, Rect rect);
Rect screen_to_relative(Rect parent, Rect rect);

Rect cut_top   (Rect* to_cut, float percentage);
Rect cut_bottom(Rect* to_cut, float percentage);
Rect cut_left  (Rect* to_cut, float percentage);
Rect cut_right (Rect* to_cut, float percentage);

Rect pad_rect     (Rect to_pad, float percentage);
Rect pad_left_right(Rect to_pad, float percentage);
Rect pad_top_bottom(Rect to_pad, float percentage);

#endif