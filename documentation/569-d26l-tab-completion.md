# 569 - D26L: tab completion in the backtick console

Part of the [D26L](../jobs/menus-ui.md#d26l---console-review-a-rock-solid-backtick-console)
console review. The job's starting points list "tab completion of command and
landmark names" as the last open part; the user selected it on 2026-11-27
("work on D26L tab completion").

## 1. Behaviour (bash with show-all-if-ambiguous)

- **Tab** completes the word that ends at the caret.
- **One candidate:** the word is replaced by it and a space follows. When a
  space is already there, the caret steps over it. With the caret inside a
  word, nothing is added after it.
- **Several candidates:** the word extends to their longest common prefix.
- **No extension possible:** the console echoes `> ` and the line up to the
  caret, then lists the candidates in tabbed rows. The D26L4 elastic tab stops
  line them up. The live command line is unchanged.
- **No candidates** (numbers, `lamp note` free text, an unknown command's
  arguments): nothing happens.
- Ctrl/Alt/GUI+Tab do nothing. A selection in the command line is dropped,
  not deleted.

## 2. Where the candidates come from

The D26L2 command table (`recomp/src/ttk/console_syntax.cpp`). The
highlighter, `help` and completion all read it, so they cannot drift apart.

| Word | Candidates |
| --- | --- |
| first | every table command (aliases `flash`, `crouchgo` included) |
| later, most commands | the command's known words (`goto room face quiet ...`, `level force`, `help all`, `lamp ...`) |
| `goto`, any later word | its words and the landmark names (`assets/atlas/landmarks.txt`, `goto_landmarks()`) |
| `option` word 2 | settings keys of live choice rows (`row_live`, as `option` accepts) |
| `option` word 3 | that key's values (the choices' `env` strings) |
| `spawn` rest of line | `enemy`, `enemies` and item names with their spaces (`desert eagle ammo`) |
| after `spawn enemy` | nothing (a type number) |
| after `lamp ... note` | nothing (free text) |

The run-time lists are new functions beside the D26L2 predicates:
`console_spawn_names` / `console_landmark_names` (`goto.inc`) and
`console_option_keys` / `console_option_values` (`game_menu.cpp`). In
Modernized play, `console_spawn_names` filters the way `items` lists: level
keys only in their level, crystals only in levels 0, 5 and 9, only models the
level has loaded. Outside play, and in Vanilla, where `spawn` refuses anyway,
it gives every name. Matching ignores case and candidates are lower case,
since the console types lower case.

## 3. Framework side (`main.cpp`, exported patch)

- `debug_console_event` handles `SDL_SCANCODE_TAB` with `ttk::console_tab`
  (the line edit) and `ttk::console_tab_list` (the rows). Before this change
  Tab typed nothing (the keycode is below 32).
- **Outer spaces are trimmed before a line runs.** A completed word ends in a
  space, and most handlers compare whole lines (`strcmp(line,"fps")`), so
  `fps ` used to answer "Unknown command". The echo still shows the line as
  typed, and history stores it trimmed. Blank lines (D26L) are checked first
  and still echo as typed. The debug port's `console` runs through the same
  path.

## 4. Listing rows and colours

- **Rows.** Each row starts with a tab, so the first cell is not taken for a
  heading. There are up to 6 columns, `84 / (widest + 3)`. At most 120
  candidates are shown, then `... and N more: type a few letters first`. Rows
  stay under the 96-byte limit (native test).
- **Echo.** The listing's echo ends in a space. In an echoed `> ` row that
  ends in a space, the classifier treats the last word as still being typed,
  so `> goto ro ` shows `ro` plain rather than as a red typo. An echoed
  command that ran (no trailing space) keeps its typo red, as accepted in
  D26L2.
- **Cell colours.** Commands are gold and landmarks violet, as before. New: a
  tabbed cell that is exactly a known keyword is cyan, and one that is a spawn
  name or option key is violet. `enemies` in the spawn list is gold, because
  it is also a command.
- `help` gains a footer line: "Tab completes a command or name; Tab again
  lists the choices".

## 5. Evidence (2026-11-27)

- `ttk-console-syntax-test` (`recomp/tests/local/console_syntax_native.cpp`):
  29 Tab cases (unique, common prefix, list, mid-word, an existing space,
  spawn names with spaces, `spawn enemy`, option keys and values, landmarks,
  `lamp note`, unknown commands), the empty-line list (every command), the
  120 cap, the listing echo's colours and the candidate-cell colours. All
  checks pass; classifying costs 36 us per 25-row page.
- Offscreen lab `recomp/analysis/d26l-tab/probe.py` (local; private profile
  and card copies, the player's files hash-checked; slot 5, level 0,
  Modernized), driven by debug-port `console_ui` keys:
  - `got` gave `goto `, `goto ro` gave `goto room`, and a second Tab listed
    `room rooms`.
  - `crouchw` gave `crouchwalk `, `spawn desert eagle a` gave
    `spawn desert eagle ammo `, `option cross` gave `option crosshair `,
    `level f` gave `level force `, and `lamp note li` stayed as it was.
  - `spawn d` listed this level's `desert eagle`, `desert eagle ammo` and
    `double duke`. `goto` Tab Tab listed its words and the nine landmarks,
    `spawn` Tab Tab level 0's items, keys and crystals, and
    `option crosshair` Tab Tab `0 1`. Tab on an empty line listed every
    command in 6 columns.
  - `fps ` and `history ` ran, and `spawn armor ` spawned armor (stored as
    `spawn armor`). On the debug port, `  fps  ` and ` goto  ` ran.
  - Screenshots in `recomp/analysis/d26l-tab/out/` (local).
- Python suite 291 tests (2 skipped), the five CI runtime tests,
  `check_jobs.py` and `check_repo.py` pass. The exported patch applies to the
  pinned framework and reproduces `main.cpp`.

## 6. Acceptance

User, 2026-11-27: "lock it in and close it out. amazing work". D26L Done.

## 7. Limits

- No cycling (Tab Tab through candidates in turn) and no Shift+Tab. Listing is
  the shell's way to see the choices.
- `option` keys come from live rows only, but the highlighter still colours
  any key (unchanged).
- Not yet seen on the real display; offscreen shots use llvmpipe.
