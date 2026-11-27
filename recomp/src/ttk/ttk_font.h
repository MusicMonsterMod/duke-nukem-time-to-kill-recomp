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
// D26L1: where ttk_font_rasterize puts each byte of text (x, advance, row, in
// pixels of its image; advance 0 for a byte it does not draw). Row r's top is
// *top + r * *line_height. Returns the row count, 0 where rasterize would fail.
int ttk_font_layout(const char* text,int style,int capacity_w,int capacity_h,int available_w,
                    int* xs,int* ws,int* rows,int max_bytes,int* line_height,int* top);
// D26L2: the console's text in either profile (the console is a developer tool
// in Vanilla too): as above, byte i of text drawn in tints[i] (ARGB, 0 keeps the
// font's colour, the tint keeps its shading; the shadow is never tinted).
int ttk_font_rasterize_console(const char* text,int style,uint32_t* pixels,int capacity_w,int capacity_h,
                               int available_w,int* width,int* height,const uint32_t* tints,int tint_count);
int ttk_font_layout_console(const char* text,int style,int capacity_w,int capacity_h,int available_w,
                            int* xs,int* ws,int* rows,int max_bytes,int* line_height,int* top);
// D26L4: the console row with tab stops: its leading spaces are kept and a tab
// advances to the next of stops[] (font units from the text's left edge), or a
// two-space gap past the last. host_osd.c lines up blocks of tabbed rows with
// them (elastic tab stops); measure gives a cell's width in the same units.
int ttk_font_rasterize_console_tabs(const char* text,int style,uint32_t* pixels,int capacity_w,int capacity_h,
                                    int available_w,int* width,int* height,const uint32_t* tints,int tint_count,
                                    const int* stops,int stop_count);
int ttk_font_layout_console_tabs(const char* text,int style,int capacity_w,int capacity_h,int available_w,
                                 int* xs,int* ws,int* rows,int max_bytes,int* line_height,int* top,
                                 const int* stops,int stop_count);
int ttk_font_measure_console(int style,const char* text,int len);
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
// D43: the level title card (TTK Big Italic, red, 1x, spacing -1, near-black shadow).
enum {TTK_FONT_LEVEL_TITLE=13};
// D20I: LEVEL STATS letters (red; also a finished count) and values (beige),
// TTK Medium Italic 1x in EDuke32's colours.
enum {TTK_FONT_STATS_LABEL=14,TTK_FONT_STATS_VALUE=15};
int ttk_font_draw(int set,const char* text,uint32_t* dst,int dst_w,int dst_h,int x,int y);
// D19T: the same with `spacing` extra pixels after each letter (one call per
// line, so its double quotes open and close in turn).
int ttk_font_draw_spaced(int set,const char* text,uint32_t* dst,int dst_w,int dst_h,int x,int y,int spacing);
// Width in pixels ttk_font_draw would draw, or -1 without a valid pack/set.
int ttk_font_text_width(int set,const char* text);
int ttk_font_line_height(int set);
#ifdef __cplusplus
}
#endif
