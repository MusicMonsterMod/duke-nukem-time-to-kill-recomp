# 567 - D26L2 console syntax highlighting

Job: [D26L2](../jobs/menus-ui.md#d26l2---console-syntax-highlighting), part of the
[D26L](../jobs/menus-ui.md#d26l---console-review-a-rock-solid-backtick-console)
console review. User, 2026-11-27: "another which is syntax highlighting for the
console"; then "you can make all the decisions on this one, work autonomously".

## 1. What the player sees

The backtick console colours its text by what each word is:

| Colour | ARGB | Meaning |
| --- | --- | --- |
| gold | `f4bc48` | a known command (the caret's gold) |
| cyan | `62d4ec` | a word that command knows: `room`, `face`, `quiet`, `on`, `off` |
| green | `9ee27a` | a number: `12`, `-300`, `+45`, `0.5`, `1e3`, `0x100f`, `16:9` |
| violet | `e0a4ff` | a run-time name: spawn item, goto landmark, option key or value |
| red | `ff5c4c` | a word nothing knows (a typo) |
| dim blue | `8c9cd0` | help's placeholders: `N`, `KEY`, `[radius]`, `<type>`, `x y z` |
| soft red | `ff8c7c` | the text of an error reply |
| amber | `ffcc70` | the text of a `Usage:` / `Try:` / `Note:` reply or a warning |
| dim steel | `7c8ca4` | the `> ` prompt |

Everything else keeps the font's own Console steel.

- **While typing** the command line is classified word by word, as the
  handlers read it. The word that ends at the caret is still being typed: it
  stays plain while it can still become a known word or number (`got`,
  `goto ro`, `goto -`, `spawn desert ea`), and turns red once it cannot.
- **Replies:**
  - The echoed command (`> ...`) and `history` rows are classified as typed
    lines, so a typo stays red in the scrollback.
  - `Unknown command: ...` is all error.
  - Every other line takes a tone from its wording (errors: `Cannot`,
    `Can't`, `unavailable`, `profile only`, `not understood`, `no such`, ...;
    warnings: `Usage:`, `Try:`, `Note:`, `is off in OPTIONS`, `is hidden:`).
    Command examples inside it are then coloured on top.
- **Where an example starts in a reply:** a lowercase known command at the
  line's start, after `Try:` / `type` / `or` / `also` / `usage` / `then` /
  `and` / `e.g.`, after `(`, or after a word ending in `,` `:` `;`.
  - The example runs on through that command's keywords, numbers, names and
    placeholders (pieces split at `| [ ] < >`, so `[on|off|default]` reads
    as three keywords).
  - It ends at a column gap (two spaces), at trailing punctuation, or at any
    other word.
  - An indented help row that starts with a word only one command knows
    (`  face 90`, `  quiet` in `goto help`) is that command's extra.
- `help` lists the everyday commands from the table; `help all` adds the
  developer toggles and labs. Both use the table's usage and summary columns.

Vanilla's console now looks the same as Modernized's: before D26L2 the TTK font
refused outside Modernized and Vanilla's console drew only the command line in
the 8x8 fallback, with no scrollback. Vanilla play is unchanged; its commands
still refuse what only Modernized does.

## 2. One table (`src/ttk/console_syntax.cpp`)

`table[]` holds every console first word (45 with the aliases `flash`, `crouchgo`
and the `crouchstand...` family; the framework built-ins `clear fps help history
quit` included), the later words it knows, flags for run-time names and its help
tier, usage and summary.

- **Flags:**
  - `family`: the first word only begins with the name.
  - `hex_args`: bare hex numbers (testmove's actor).
  - `note_text`: free text after `lamp note`, and lamp `style wave:3:0.5`
    pieces.
  - `spawn_rest`: the rest of the line is one item name.
  - `landmarks`, `option_kv`: run-time names.
- **Run-time names** come from the game, through four providers:
  - `console_spawn_name` and `console_landmark_name` at the end of `goto.inc`
    read `spawn_items[]` / `spawn_crystals[]` and `goto_landmarks()`. The
    landmarks file is re-read at most every 2 s.
  - `console_option_key` and `console_option_value` in `game_menu.cpp` read
    the OPTIONS rows that `option` accepts.
  - Each provider answers no, a whole name, or a name begun (for the word
    being typed).
- **Tab completion** (still a D26L item) can read the same table through
  `console_command_count()` / `console_command_name()` and the providers'
  prefix answers.

`tests/local/test_console_syntax.py` keeps the table honest. It fails when:

- a handler parses a first word (`strcmp`/`strncmp` on `line` or
  `s_console_line`, in any `src/ttk` file or `main.cpp`) that the table lacks;
- a table command has no handler;
- the fixed name lists (lamp styles, fpidle gestures, hangpose shapes) are
  missing from their commands' words;
- a hand-written `Available commands:` help list comes back in `main.cpp`.

The lamp file's `ambient` / `relit` directive parser (`relit.inc`) is exempt:
it reads the level file, not the console.

## 3. Drawing (framework and font)

- **`ttk_font.cpp`:**
  - New `ttk_font_rasterize_console` and `ttk_font_layout_console` skip the
    Modernized gate (the console only) and take a tint per source byte.
    The layout is the same one `ttk_font_rasterize` uses.
  - A tinted glyph pixel keeps its alpha and the font's shading. The steel
    ramp's luminance scales the tint up to the letters' body (about 200 of
    255), and brighter highlights lean 30% towards white.
  - The drop shadow is never tinted. Since each glyph knows its source byte,
    the italic slant never bleeds a colour into the next letter.
- **`host_osd.c` `console_draw_row`:**
  - Asks the game for tints (`ttk_console_tints`: kind 0 for the command
    line with the caret's byte, kind 1 for a scrollback row) and rasterizes
    with them.
  - The `> ` before the command line is dim steel.
  - Mouse hits, the selection and the caret use `ttk_font_layout_console`,
    so their positions match the drawing as before.
  - The `COPIED` note and `-- MORE BELOW --` use the console entry point
    untinted.
- **`main.cpp`:** `help` / `help all` call `ttk::console_help` (the old
  hand-written list is gone).

**Cost:**

- Tints are applied on the CPU while the console image is rasterized. That
  happens only when the image changes: an edit, new output, a scroll or a
  caret blink (twice a second). The image is uploaded only on a new
  generation (D23S), so the GPU sees exactly the uploads it did before.
  There is no shader or draw change.
- Classifying a full 25-row page takes about 35 us
  (`ttk-console-syntax-test`, desktop).

## 4. Evidence

- `ttk-console-syntax-test` (new, `tests/local/console_syntax_native.cpp`,
  stubbed names):
  - 40 per-byte classifications of typed lines and replies.
  - The partial word at the caret.
  - The table's names are unique, and both help listings fit the
    95-character line.
  - Tints are 0 for plain text.
  - The timing bound.
- `ttk-font-test` (new checks):
  - The console entry point untinted gives the same picture as
    `ttk_font_rasterize`.
  - A tint keeps the image size and alpha, and changes nothing right of
    the tinted word.
  - It works in Vanilla.
- **Offscreen in game** (SDL offscreen, private profile and card copies, the
  player's files hash-checked; lab `recomp/analysis/d26l2/shots.py`, port
  9547). Modernized, Vanilla and Modernized with night vision:
  - The echoed `> goto rooom 12` shows its typo red.
  - `goto help` colours every example, its extras `face 90` / `quiet` /
    `safe`, and `type levels`.
  - The landmark `gag-lava-south` and the item `desert eagle` are violet.
  - Errors are soft red (`Goto: Modernized profile only` in Vanilla).
  - The half-typed `qui` at the caret stays plain.
  - With night vision on, the console's colours are unchanged (the console
    is drawn over its own dark background after the game image).
- `test_console_syntax.py` and the Python suite (291 tests, 2 skipped), the
  five CI runtime tests, `ttk-inventory-test`, `check_jobs.py` and
  `check_repo.py` pass. The pinned framework plus the exported patch
  matches all 99 framework files.

User playtest, 2026-11-27: accepted ("lock it in and close it out").

## 5. Limits

- **Reply tones are read from wording.** A new handler whose error uses
  none of the shared phrases shows in steel. Add its phrase to
  `reply_tone()` or reuse one.
- **Keywords are a flat set per command, not positions.** `goto quiet room`
  colours as valid; the handler's reply still says what is wrong.
- **Not checked word by word:** `crouchstand`'s `block` / `free` anywhere in
  the line, `lamp note` text and `option` values the menu has not built yet
  (Vanilla: `Option: the menu is not ready yet`, so the key shows red there).
- **Prose:** a lowercase command word after a trigger counts as an example
  (`type items for the list` colours `items`).
- **The 180 Hz real-display GPU measure (D23S) was not taken.** The change
  adds no GPU work by construction; the user's playtest is the visual check.
