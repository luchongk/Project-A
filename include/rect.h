#ifndef RECT_H
#define RECT_H

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

Rect relative_to_screen(Rect parent, Rect rect);
Rect screen_to_relative(Rect parent, Rect rect);

Rect cut_top   (Rect* to_cut, float percentage);
Rect cut_bottom(Rect* to_cut, float percentage);
Rect cut_left  (Rect* to_cut, float percentage);
Rect cut_right (Rect* to_cut, float percentage);

Rect pad_rect     (Rect to_pad, float percentage);
Rect pad_leftright(Rect to_pad, float percentage);
Rect pad_topbottom(Rect to_pad, float percentage);

#endif