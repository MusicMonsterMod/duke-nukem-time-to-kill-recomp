#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Shared Modernized text surface, drawn with Time to Kill's own fonts (D24A;
// glyphs extracted from the player's disc into ttk-fonts.pack at build time):
// 0 message and 2 centered message (TTK Big Italic, Console steel, 1x, shadow),
// 1 heading (TTK Big Italic, gold, 2x), 3 console / compact text (TTK Medium
// Italic, Console steel, 1x, shadow). ARGB8888 pixels. Returns zero for Vanilla,
// missing/corrupt packs, or insufficient output space; callers retain their
// existing generic renderer in that case.
int ttk_font_rasterize(const char* text,int style,uint32_t* pixels,int capacity_w,
                      int capacity_h,int available_w,int* width,int* height);
// D24B savestate panel: draw one line from a glyph set directly (sets 3..7:
// panel title, slot title, selected slot title, text, dim text). Returns the
// width drawn, or -1 without a valid pack/set. Line height in pixels, 0 if none.
enum {TTK_FONT_PANEL_TITLE=3,TTK_FONT_PANEL_SLOT=4,TTK_FONT_PANEL_SLOT_SELECTED=5,
      TTK_FONT_PANEL_TEXT=6,TTK_FONT_PANEL_DIM=7};
// D08A5 mission row and item card: 8 FOUND (system, green), 9 "MISSION" label,
// 10 found/total count and 11 the count when all are found (2x Microfont).
// D08A19: 12 USED (system, the mockup's gold).
enum {TTK_FONT_MISSION_FOUND=8,TTK_FONT_MISSION_LABEL=9,TTK_FONT_MISSION_COUNT=10,TTK_FONT_MISSION_COMPLETE=11,
      TTK_FONT_MISSION_USED=12};
int ttk_font_draw(int set,const char* text,uint32_t* dst,int dst_w,int dst_h,int x,int y);
// Width in pixels ttk_font_draw would draw, or -1 without a valid pack/set.
int ttk_font_text_width(int set,const char* text);
int ttk_font_line_height(int set);
#ifdef __cplusplus
}
#endif
