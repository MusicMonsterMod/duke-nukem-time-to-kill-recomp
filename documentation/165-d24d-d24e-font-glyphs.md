# D24D / D24E - parentheses and percent in the TTK fonts (2026-10-20)

## Finding (D24D)

- The TTK Big and Medium Italic fonts on the disc do have `<` and `>`
  (`FONTS.RAW` rows 54 and 96), and every set built from them already used
  the native glyphs. Nothing fell back for those two characters.
- The reported message is `ORIGINAL MOVEMENT (%s)` (`recomp/src/ttk/pc_input.cpp`):
  it has always used parentheses, and the italic fonts have no `(` or `)`.
  Those were the 8x8 system-font glyphs the user saw.
- Every uppercase UI string in `recomp/src/ttk/` was scanned for characters
  the italic fonts lack. Only `( )` (ORIGINAL MOVEMENT) and `%` (MEDKIT,
  D24E) are used in the italic sets. The remaining fallbacks
  (`" # $ & * + [ \ ] ^ _`) are not on the disc and are not used by our
  messages.

## Change

`recomp/tools/local/build_ttk_fonts.py`, next to D24C's `exclamation()`:

- `parenthesis()`: `(` and `)` from the font's own `O`. Each row keeps the
  stroke on that side of the hole. On the closed top and bottom rows it keeps
  the half of the bar on that side of the row's own centre, so the halves
  follow the italic slant.
- `percent()`: `./.` from the font's own `/` and two periods. The periods
  are trimmed as for `!` (Big 2 rows, Medium 1). One sits at the top left of
  the slash, the other on the baseline at the right, each one clear pixel
  from the slash on every row.
- These glyphs appear in every set built from these fonts: messages, headings,
  console, savestate panel title and slots. The 8x8 system-font fallback is now
  `" # $ & * + [ \ ] ^ _` only.
- Only the build-time font pack changes. Executable
  `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`
  is unchanged; `build-local/ttk-fonts.pack` sha256
  `2609f2e6077a080c25d327582471ca1a19ab816ca1e078a44ddadeaaafe7316d`.
  Vanilla draws no host text, so it is unaffected.

## Evidence

- `python3 -m unittest tests.local.test_ttk_fonts` (from `recomp/`): 5 pass,
  including the new `test_parentheses_and_percent_from_own_font`.
- Offscreen Xvfb run (local `analysis/pc-input/d24de-glyphs-2`): `dnstuff`
  and then M showed MEDKIT 100% with the built `%` in the message font. The
  console echo `(PAREN) 50%` drew `( ) %` in the console font. Window captures
  were taken with `xwd`, because `screenshot_file` captures the PS1 display
  without the host OSD.
- Pack previews at 1x and 2x (messages, console, headings, panel) read as
  parentheses and a percent sign.

Accepted by the user on 2026-10-20: "its perfect! lock it in close it out".
