
float ascent, descent, line_gap;

Texture* load_font_file(String path, u8* font_bitmap, stbtt_bakedchar* font_glyphs) {
    String font_file = os_read_entire_file(path);
    stbtt_BakeFontBitmap(font_file.data, 0, 48.0f, font_bitmap, 512, 512, 32, 96, font_glyphs);
    stbtt_GetScaledFontVMetrics(font_file.data, 0, 48, &ascent, &descent, &line_gap);
    return create_texture_from_bitmap(font_bitmap, 512, 512);
}