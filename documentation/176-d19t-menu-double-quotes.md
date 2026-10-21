# D19T - Real double quotes in the menu font

Job: [D19T](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21: "that's awesome. lock it in close it out.").
Follows [D19S](175-d19s-delete-saves.md).

## Finding

The text call `0x8008b454` draws each character `0x21`-`0x60` with the glyph
call `0x8008b210`. That call reads a 4-byte entry (u, v, width) from
`0x800c3f30 + font * 0x100 + char * 4`, with the font height at
`0x800c4484 + font * 8`. The texture page is `/DATA/FONTS.RAW` (VRAM 960, 0)
with gold CLUT 225. Neither font has a double quote:

| Font | `'` entry | `"` entry |
| --- | --- | --- |
| small (0, 12 px) | u121 v98 w7: the apostrophe, ink rows 98-104 | u122 v98 w6: the same apostrophe, 1 px over |
| menu (1, 18 px) | u131 v54 w6: the apostrophe, ink rows 54-62 | u38 v83 w9: stray pixels from the small font's rows |

The font's apostrophe has the shape of a closing quote (heavy top, tail down
and to the left).

## Change (`recomp/src/ttk/game_menu.cpp`, Modernized menus only)

- `Guest::say` sends any string with `"` to `say_quoted`. The runs between
  the quotes are drawn with the original text call, left-aligned from the
  start the text call would use (centre or right as before). Each `"` is two
  of the font's own apostrophes, drawn with the glyph call
  (`QuoteMetrics`: pair step 5 small, 6 menu).
- `"` opens and closes in turn. A closing quote is the two apostrophes as
  they are. An opening quote is turned half round: in the two packets just
  added (POLY_FT4), the UVs are flipped on both axes over the apostrophe's
  ink rows (8 small, 10 menu, outline row included), and the bottom
  vertices sit at that height. The heavy end then sits low with the tail up
  and right, which is the shape of an opening quote, in the same pixels,
  gold, outline and slant.
- `Guest::width` counts a quote as pair step plus the apostrophe's width, so
  centring, right alignment and wrapping stay exact.
- `say_dim` / `say_clut` work unchanged (they post-process every glyph
  packet the string added).
- No guest memory or texture change. The glyph tables stay guarded by the
  D19 identity guard. The original's own strings never contain `"`.
- Diagnostics only: `DNTTK_TEST_MENU_TEXT=<text>` draws the text in both
  fonts on every one of our pages (small at y -110, menu at y 96).

## Console steel and the other pack fonts (first playtest)

User, first playtest: "the quotes dont seem to show on the load/delete screen?
it is showing the fallback font quotes". The DELETE SAVED GAME: name line is
Console steel (`ttk-fonts.pack`, D19S user request), whose `"` fell back to
the system 8x8 font. Now:

- `build_ttk_fonts.py` builds both quotes into the Big and Medium Italic
  fonts from their own apostrophe, as the menu font's: `"` is the closing
  quote, `` ` `` (unused, it drew `?`) the opening one. Every set on those
  fonts has them (messages, headings, console, savestate panel titles).
- `ttk_font.cpp` draws every other `"` of a string as the opening quote.
  A literal `` ` `` also draws an opening quote.
- New `ttk_font_draw_spaced` (extra pixels after each letter): the menu's
  steel lines are drawn with it in one call instead of letter by letter, so
  their quotes alternate. Same pixels and spacing as before.

## Evidence

Offscreen, OpenGL, 4x, 16:9, 120 Hz, private profile and the D19Q test
savestates (`recomp/analysis/d19k-load`, `k.py`; local). Executable
`52f5f1a3ae67040f6e9d52d544ba914e7115c1e38985a23ce066937c22aea29c`.

| Check | Result |
| --- | --- |
| Pause list, `DELETE "SLOT 01 - TIME TO KILL"? IT'S "OK"` in both sizes | opening and closing double quotes in the font's gold and outline in both sizes; apostrophe in IT'S unchanged; no neighbouring-glyph pixels at the flipped edges; centred lines centred |
| Pause list items, prompts | unchanged |
| Pause > LOAD GAME, Delete on slot 01 (second build) | `"SLOT 01 - TIME TO KILL"` in steel with real opening and closing quotes; rest of the page unchanged; no slot deleted |
| Font pack | `"` no longer in any set's system-font fallback list; `test_ttk_fonts` OK |
| Menu identity guard | passes (`identity: true`, menu owned) |
| Vanilla | boots (game menu not used there) |
| Tests | Python 162 OK (2 skipped), `ttk-input-test`, `ttk-live-env-test` |

## Limits

- `"` is the only new character. The menu font's other gaps are unchanged.
- Not run on the software renderer (flipped UVs on the same packets).
