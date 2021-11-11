#ifndef SIMPLE_DRAW_H
#define SIMPLE_DRAW_H

void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec4 color);
void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 uv1, Vec2 uv2, Vec2 uv3, Vec2 uv4, Vec4 color);
void draw_rect(Rect rect, Vec4 color);
void draw_text(Vec2 origin, String text);

Vec4 darken(Vec4 color, float amount);

/*void sd_flush();*/

extern Texture* sd_font_texture;

#endif