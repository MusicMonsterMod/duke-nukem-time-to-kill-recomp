# 568 - D26L4: console lists that read at a glance

Part of the [D26L](../jobs/menus-ui.md#d26l---console-review-a-rock-solid-backtick-console)
console review. User, 2026-11-27: "stuff like "levels" for example, it just
feels like a brick wall of text. it shows the hidden levels, and the
multiplayer levels too".

## 1. Why the lists were a wall

Measured on the offscreen console before the change (`levels`, `help`,
`items`):

- **No real columns.** The console font is proportional, so replies padded
  with spaces (`%-17s` in `help`, three titles a line in `levels`, two items a
  line in `items`) never lined up.
- **Indents were lost.** The shared word wrap (`ttk_font.cpp wrap()`) drops
  every space at the start of a row, so `  3 GOLD AND GUNS` started at the
  left edge like a heading.
- **One colour.** Headings, labels, numbers and names were the same steel.
- **Long replies were cut.** Scrollback rows held 95 bytes, so the long
  `lamp help` lines and the `gag-lava-corner` landmark note were cut short.
- **No grouping.** `levels` listed indices in order (0 1 2 3 5 6 ...) and a
  hidden-level block mixing cut levels, test maps and arenas.

## 2. Elastic tab stops (framework console, game font)

A reply separates its columns with a tab. The console lines the columns up.

- **Blocks.** Consecutive scrollback rows that hold a tab form a block. Column
  k of the block is as wide as its widest cell k that ends in a tab, plus a
  three-space gap (`host_osd.c console_tab_stops`). One untabbed line between
  tabbed rows (a heading inside a table, `Add to any of them:`) does not end
  the block; an echoed `> ` command does. A block reaches at most 128 rows each
  way and is computed once per console redraw.
- **Font.** `ttk_font_rasterize_console_tabs` / `ttk_font_layout_console_tabs`
  take the stops (font units from the text's left edge). A tab advances to the
  next stop, or a two-space gap past the last. Console rows keep their leading
  spaces; a wrapped continuation drops them. A tabbed row that wraps goes on
  under the cell it wrapped in (a hanging indent, at most half the width).
  `ttk_font_measure_console` measures a cell in the same units. Menus, cards
  and messages lay out exactly as before (`ttk-font-test` checks).
- **Longer rows.** Scrollback rows hold 191 bytes (`CONSOLE_ROW 192`); the
  typed line keeps its 95. Row images are up to 128 pixels tall (three wrapped
  lines). Copy buffers in `main.cpp` grew to match.
- **Copy.** Copied text keeps the tabs, so a list pastes into a document or
  terminal as columns. Double-click word selection treats a tab as a space.
  Pasting into the console still turns tabs into spaces.

## 3. Colours for lists (`console_syntax.cpp`)

| Tone | Colour | Where |
| --- | --- | --- |
| heading | near white `f4f8ff` | an unindented line ending in `:`; the label cell that starts a tabbed row (`OLD WEST`); a one- or two-word capitalised label before `:` (`Levels:`, `Renderer:`) |
| number / name | green / violet | in the cells after the label: a number then a name (`12 BLOOD BATHS`, `*0 TIME TO KILL`), a name then a number (`armor 320`), a cell of numbers only (`54 155 156`), a landmark name |

Error and warning replies are never recoloured as headings. Tabs end a command
example (like a two-space column gap) and a cell's first word may start one,
so `help`'s usage column is gold and cyan as before. The `*` that marks the
current level is white.

## 4. The lists

- **`levels`**: one row per era from `level_era()` (PRESENT DAY 0 5 9, OLD
  WEST 1-3, MEDIEVAL 6-8, ANCIENT ROME 10-12, BOSSES 27-29, CHALLENGE STAGES
  21-26 on two rows, titled `STAGE N`), then `Hidden levels:` with CUT LEVELS
  (4, 13, 30), TEST MAPS (14, 31) and 2-PLAYER ARENAS (15-20 on two rows). All
  one aligned block. The hidden heading says `goto N` in Modernized and
  `level force N` in Vanilla (goto is Modernized only).
- **`level N`** for a hidden index names it and its kind (`Level 15 MESA is
  hidden (a 2-player arena): type goto 15`). `level x` gives `Try: level 3`,
  `level 99` `Level: no such level 99`, `level force` its usage.
- **`help` / `help all`**: sections TRAVEL, SPAWN, GAME, CONSOLE, then WORLD
  LABS, AIM LABS, DUKE LABS, RENDER LABS; a command no section names is listed
  under OTHER, so the table cannot drift away from help.
- **`items`**: ITEMS, CRYSTALS, WEAPONS, AMMO three to a row (`name type`),
  then `OTHER (TYPE)` numbers.
- **`enemies`**: `TYPE`, type and name, `(N placed)`.
- **`goto help`**: tabbed; the extras (`face 90`, `quiet`, `safe`) share the
  examples' column. Its last line points to `levels` for the hidden levels.
- **`goto landmarks`**: grouped by level (`3 GOLD AND GUNS`), name, note.
- **`lamp help`**: sections FIND, ADD MOVE, LOOK, PROPS, FILE, SWITCHES,
  AMBIENT, one row per subcommand (it was nine 100-200 character lines).

## 5. Answers instead of "Unknown command"

- A known command whose arguments no handler took (`warp`, `option`, `sfx`,
  `quicksave x`) says `Usage: <usage> - <summary>` from the table (an alias
  shows its command's).
- An unknown word suggests the nearest listed command (edit distance up to
  2, or a word of 3+ letters that begins one): `Unknown command: gotoo. Try:
  goto help (help: every command)`; otherwise `Type help for the list`.
- Both are game-side (`level_select.cpp`, the end of the console chain);
  `main.cpp`'s own `Unknown command` stays as a fallback.

## 6. The arenas are reachable, and run Modernized

The user asked whether the multiplayer levels can be warped to. Yes, since
D61 (note 559), not through `level N`: `goto 15` (Modernized) and `level force
15` (both profiles) load the arenas 15-20, which the original title cheat
skips.

Checked in this job (offscreen, private copies): after `goto 15` the level
overlay tag at `0x800CA968` is `0x12`, the `BASEOVR.OVR` tag that D61
authenticated for index 31 (the sound test). The arenas carry no overlay of
their own, so they pass the Modernized identity check: 0 refusals, and Duke
walked 788 units in 1 s (Modernized walking). Note 559 meant the arenas to stay
out of the single-player leases; sharing `BASEOVR.OVR` lets them in. Nothing
was changed here: it is unfinished developer content either way. If the
arenas should play with the original controls, that needs a level-index check
beside the overlay tag (a follow-up for the user to decide).

## 7. Evidence

- `ttk-console-syntax-test`: 15 new classifications (headings, tabbed cells,
  the `*` marker, landmarks, the suggestion after `Try:`, the hidden-level
  hint), `console_usage` and `console_unknown`; all checks pass, 38.8 us per
  25-row page.
- `ttk-font-test`: menus still drop leading spaces (same picture), console
  rows keep them, tabs land on their stops, a two-space gap past the last,
  rasterize and layout agree, the hanging indent, `measure`.
- Offscreen in game (Xvfb, private profile and card copies, the player's
  files hash-checked; lab `recomp/analysis/d26l4/review.py`, local): before and
  after replies of 41 commands and screenshots of `levels`, `help`, `help
  all`, `items`, `enemies`, `goto help`, `goto landmarks`, `lamp help` and a
  typo, in Modernized and Vanilla. Before/after shots in
  `research/before-after/D26L4/` (local).
- Python suite (291 tests, 2 skipped), the five CI runtime tests,
  `ttk-inventory-test`, `check_jobs.py` and `check_repo.py` pass. The pinned
  framework plus the exported patch matches all 99 framework files.

User playtest, 2026-11-27: accepted ("absolutely stunning work!! lock it in
and close it out").

## 8. Limits

- Alignment needs the tab: a reply that pads with spaces still does not line
  up. Use tabs for any new list.
- Colours of list cells are read from their shape (number and name), like the
  reply tones (note 567): a cell shaped like an entry is coloured as one.
- A block is found by rows that hold tabs; two different tabbed replies with
  only one untabbed line between them and no echoed command would share
  columns (no current command does this).
- Tab completion (the rest of D26L) is not done.
