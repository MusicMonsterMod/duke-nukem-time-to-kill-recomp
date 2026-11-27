# D26L3 - every printable ASCII character in the TTK fonts (2026-11-27)

Part of the [D26L console review](../jobs/menus-ui.md#d26l---console-review-a-rock-solid-backtick-console).
User, 2026-11-27: "work on D26L - can we also get the ttk fonts document
updated with the new expanded glyph set".

## 1. What was missing

Audit of the built pack (`build-local/ttk-fonts.json`) and the disc fonts:

- **Big and Medium Italic** (messages, headings, console, savestate panel,
  level title, level stats) have no `# $ & * + [ \ ] ^ _ { | } ~` or single
  backquote on the disc. Before D26L the first ten were drawn from the PSY-Q
  8x8 system font (1-px, upright, out of style), and `{ | } ~` drew `?`.
  The `` ` `` slot held D19T's opening double quote, so a real backquote could
  not be drawn.
- **The PSY-Q 8x8 font** (savestate panel text, mission FOUND / USED): its own
  `[` and `]` are the same bare vertical bar, its `_` is blank, and it has no
  `` ` { | } ~`` (they drew `?`).
- **The 3x5 Microfont** sheet has all of printable ASCII, but the builder read
  it only to `_`.

## 2. Change

`recomp/tools/local/build_ttk_fonts.py`, `full_ascii()` after D24C-D24E and
D19T. Everything is still built from the player's disc at build time; nothing
retail is tracked (the silhouettes below are ours).

- **Cut from the font's own strokes, where it has a part:**
  - `|` a narrowed I stem (its lit left texels and its dark right edge kept),
    one row taller than the capitals so it is not read as an I (Medium; Big's
    is drawn, below).
  - `_` the hyphen's lower rows (Medium 2, Big 4) below the baseline, re-lit for
    the new height from the I's row means (`retoned()`), its dark edge on the
    row under the capitals' outline so the drop shadow stays inside the line.
  - `\` the slash with each row's stroke moved to the mirror position, texels
    kept in order so it stays lit from the left (`mirrored()`).
  - Medium `[` the L's stem and foot narrowed to a bracket, the foot copied to
    the top and re-lit; `]` is `[` mirrored about the upright letter (unshear,
    mirror, shear again).
  - `` ` `` the apostrophe turned half round (the single form of D19T's
    opening quote).
  - `$` the font's own S with a row taken out (Medium 1, Big 2), stem ends drawn
    above it and below the baseline.
  - `&` painted texel by texel in the font's palette indices after its own `8`
    (the lower bowl opened into a tail and a kicked foot). Built from the 8 it
    read as an 8; drawn from scratch it read as an R.
- **Drawn (ours) and lit as the font:** `# + * ^ ~ { }`, Big's `[ ] |`. Each is
  an upright silhouette in `ART`, leaned at build time by the font's own slant
  (Big 4 px over 16 rows, Medium 3 over 10, as the I) and lit by
  `style_shade()`. `}` is `{` mirrored before the lean.
- **`style_shade()`** is fitted on the font's own letters and digits at build
  time: a lit texel's index is the font's mean lit index for that row, plus
  mean offsets by distance (capped at 4) from the stroke's left and right
  edges; the outline is the font's commonest outline texel, where the font puts
  it (Medium: along the right and the bottom, not on the top two rows; Big: all
  round). Re-lighting the font's own E, A and S silhouettes with it gives
  colours within one or two steps of the disc's (the disc's inner bevel lines
  are not reproduced; where a glyph needs one, the art marks it `o`). Art marks:
  `#` lit as the font would, `+` always as a stroke's inside (thin arms of `*`,
  the `$` stem end below the S), `o` dark line, a hex digit a texel painted by
  hand, `-` cut from the disc part.
- **8x8 font:** `SYSTEM_EXTRA` gives real `[ ] _` and new `` ` { | } ~`` in its
  1-px style (the pipe and the underscore 9 rows; the line is 10).
- **Microfont:** read to `~`.
- **Pack TTKFONT3:** 96 glyph records per set (ASCII 32..126, then 127), the
  opening double quote in 127. `ttk_font.cpp` draws each odd `"` as glyph 127
  (was `` ` ``). The 8x8 and Microfont sets put their `"` in 127. Pack 822,980
  bytes (the loader takes up to 1 MiB).

Console input (framework `main.cpp`, exported in the runtime patch):

- Typing: a key gives `SDL_GetKeyFromScancode(scancode, Shift, false)` when that
  is printable ASCII, lowercased; so every character of the player's keyboard
  layout, with Shift, types (US layout: `! @ # $ % ^ & * ( ) _ + { } | : " < > ?
  ~`). The keypad keeps its D26 rules. The backtick key (scancode GRAVE) stays
  the console's toggle, so `` ` `` and `~` on it come in by pasting. Ctrl, Alt
  and GUI chords still type nothing (AltGr layouts are not taken).
- Paste keeps every printable ASCII character (the pipe and backtick were left
  out before, with a "pipe, backtick" note); only non-ASCII and control
  characters are left out, with a note.

## 3. Evidence

- `python3 -m unittest discover -s recomp/tests/local`: 287 pass (2 skipped).
  New `test_full_printable_ascii`: in all 16 sets no printable character is the
  `?` glyph, every glyph fits its line, `[ ]`, `{ }`, `[ |`, `( |` and
  `` ` ' `` are different glyphs; in the italic sets there is no system-font
  fallback and glyph 127 differs from `` ` `` and `"`. Set offsets moved to
  776 bytes in the older tests.
- `ttk-font-test` (rebuilt): a line with every new character in styles 0-3 at
  three widths, the layout checks, and `` ` `` drawn unlike an opening quote.
  `ttk-inventory-test` passes.
- Offscreen in game (Xvfb 1280x720, private settings and card folder, port
  9131; local captures in `research/before-after/D26L/`): through
  `console_ui`, a pasted `echo` line holding all fifteen new characters, a
  quoted word, parentheses, `50%` and `a|b` ran and echoed in the new glyphs; keys with Shift typed `| { } _ + ! @ # $ & * ( ) :
  " < > ? '` and unshifted `[ \ ]` typed `[ ] \` (the backslash key name had to
  be sent as hex: the debug port's JSON reader does not unescape `\\`).
- Before/after showcase composed in Photon from the font page's renders:
  `research/before-after/D26L/d26l-glyphs-before-after.png` (and `.photon`).
- The pinned framework plus the exported patch matches all 99 framework files.
  The five runtime tests CI runs pass. Six other runtime tests
  (`test_fps_telemetry_opt_in`, `test_host_vertex_depth`,
  `test_interpreter_perf_guards`, `test_overlay_pair_dedup_runtime`,
  `test_present_skip_cadence_guard`, `test_runtime_perf_diag_guards`) fail, and
  fail the same way with the last committed patch: not from this job, not in CI.
- Executable `49c3ccf07d361c21484012b307a31eeec7aea9ac509526f69a26159d6dc10f24`
  (normal build-local), `ttk-fonts.pack`
  `15d223fab0693f90c25a51c8c92945820fe796f9736eb02bc35150a80911379a`.
  Vanilla draws no host text and is unaffected.

## 4. The fonts page

The local D24A font page (`research/TTK-UI-and-Font-Research.html`, a link to
`recomp/analysis/d24a-fonts/ttk-font-picker.html`, retail-derived, never
committed) is regenerated by `make_picker.py`, which now builds the italic
glyphs with the shipping `full_ascii()` and adds the 8x8 extras. New section 17
shows the console and messages before and after, both fonts in gold at 5-6x,
the 8x8 additions and a per-character source table; the glyph sheets in
section 3 list all printable ASCII as present; the old "the italic fonts have
no ..." note is corrected; the page's sample text draws `"` with glyph 127.

## 5. Revision after the user's first look (2026-11-27)

User: "the only ones to just take a little quick look at improving are ~ and ^
as they look extra skinny on the TTK italics, and then { and } dont quite land
in appearance for me either".

- `^` and `~` regenerated as strokes of a set thickness (a chevron and one sine
  period): Medium 4.2 px arms over 5 rows and a 2.6-row wave; Big 6 px arms
  over 8 rows and a 4-row wave. The silhouettes are written out in `ART`.
- `{ }` redrawn as the classic brace: straight upright sides, flat two-row
  hooks at the top and bottom, a short point in the middle (it was a slanted
  `<`-like curve); Big's stem one texel wider (7).
- On drawn Medium glyphs the dark right edge now falls only after 3 lit texels
  (`is_edge(..., drawn=True)`), so thin diagonals keep their weight; the font's
  own glyphs and the other drawn ones at full width are lit as before.
- Font tests, `ttk-font-test`, `ttk-inventory-test` pass; pack
  `2ba0f2227c45ea937927a20f23d43bb878e8fb3ee271baf540b3d90cb9bb6f18`,
  executable `688208c8966d7ab43bcac198bd75d5bad67f17875004d638e2ab1c8dea6a6bac`
  (same source; a fuller rebuild after the patch check touched framework
  timestamps). The fonts page and the Photon showcase were regenerated.

Accepted by the user on 2026-11-27: "lock it in and close it out".

## 6. D26L5: bolder ^ and ~, a redrawn ] (2026-11-27)

User: "~ and ^ are too thin still, can we thicken it up. i think you
misunderstood me last time when i said it's too thin and you horizontally made
them a little wider. dont change that anymore, but they need a bit of boldness
to them / then ] appears a bit rough looking, can you redo this one". Job
[D26L5](../jobs/menus-ui.md#d26l5---bolder--and--and-a-redrawn-).

- Cause: the section 5 strokes were thin across the stroke, not along it. The
  Medium `^` arms were 4 texels of which the font's dark-edge rule (bottom of
  every column, right edge after 3 lit) darkened one or two, leaving 2-3 lit;
  the Medium `~` was mostly one lit row over a dark one. Big's were 6-texel
  arms and a 3-4 row wave inside a full outline. The letters' diagonals are 4-5
  lit texels a row and the hyphen 2 lit rows over a dark one (Big: 6 rows).
- `^`: same widths (Medium 13, Big 21), one row taller (Medium 6, Big 9), the
  arms 5 / 8 texels a row meeting in a solid apex; a `+` keeps Medium's right
  arm's inner texel lit, so each arm row is 4 lit texels and a dark edge.
- `~`: same widths (13, 21), every column the hyphen's thickness: Medium 3-4
  rows (2-3 lit over the dark edge), Big 6 rows; the top row moved up one
  (Medium row 3, Big row 3) so the wave stays centred on the line.
- Medium `]`: drawn in `ART` (an 8-texel bar of 2 lit rows over a dark one at
  top and bottom, a straight 4-texel stem lit 3 with a dark right edge, leaned
  as the font) instead of `[` mirrored. The mirror of the L-derived `[` had a
  stem 2-3 texels wide that stepped irregularly (the per-row unshear rounding)
  and a lighter top bar than its foot. Medium `[` and Big's `[ ]` unchanged.
- Glyph widths: Medium `^ ~` 13, `]` 11 (was 10, now the width of `[`); Big
  `^ ~` 21. Every other glyph is byte-identical (only the `ART` entries
  changed); pack `42a297b19248fe805af84d266d29c1c6c88b1162340103fe022496a6c735a60e`
  (D26L3's `2ba0f222...` rebuilt from the HEAD script for the before shots).
- Evidence: texel dumps and 8x / 6x previews of `A^V~N[I]E-W` in the console's
  steel palette before and after; offscreen in game (Xvfb, private profile and
  card copies, the player's files hash-checked; lab
  `recomp/analysis/d26l5/shot.py`, local) the console line and replies with
  `^ ~ [ ]`, before (the old pack through `DNTTK_FONT_PACK`) and after.
  Before/after shots in `research/before-after/D26L5/` (local). `ttk-font-test`,
  `ttk-inventory-test`, `ttk-console-syntax-test` and the Python suite pass.
  The local D24A fonts page was regenerated from the shipping code.

Accepted by the user on 2026-11-27: "this is awesome. lock it in close it out. a solid, and aesthetically pleasing, and functional, easy to use console".

## 7. Limits and follow-ups

- Lower case is still drawn as capitals (all caps).
- Still open in D26L: the command review (help, errors, Modernized-only
  refusals), tab completion, drag auto-scroll above the top row.
