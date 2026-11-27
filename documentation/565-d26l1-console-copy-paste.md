# D26L1 - console caret, selection, copy and paste

The backtick console's command line gets a caret with desktop editing, mouse
selection over the scrollback and the line, copy and paste through the system
clipboard. All profiles (the console is the same in Vanilla); nothing reaches
Duke's controls while it is open (D26J, D26K unchanged). Framework edit,
exported into `recomp/patches/time-to-kill-accepted-source.patch`.

User, 2026-11-27: "I'm a linux guy so i tend to use a lot of shift insert for
my pasting, but ctrl c, right click and so on should work, i think those basic
gnome terminal type things should be possible? ... just something simple".
Choices made on a clickable mock-up (a private artifact) before building:
right-click pastes (no menu), no middle-click, drag / double-click /
triple-click select, "add the caret in too", "we should have instant copy on
select too", "multi line paste runs each line", "add ctrl+w and ctrl+u".

## 1. Behaviour

| Input | Action |
| --- | --- |
| Left / Right, Ctrl for a word | move the caret |
| Home / End | line start / end (End also scrolls to the newest output) |
| Backspace / Delete | the character before / after the caret, or the selection |
| Ctrl+W, Ctrl+Backspace / Ctrl+U | the word before the caret / everything before it |
| Shift with Left, Right, Home, End; Ctrl+A | select in the command line |
| Drag, double-click, triple-click, Shift+click | select by character, word, line; extend |
| Mouse release with a selection | copies it at once (clipboard and X11 primary) |
| Ctrl+C, Ctrl+Shift+C, Ctrl+Insert | copy the selection, else the whole command line |
| Shift+Insert, Ctrl+V, Ctrl+Shift+V, right-click | paste at the caret |
| Click in the command line | places the caret |

Numpad with Num Lock off adds 4 / 6 (Left / Right), 0 (Insert, so Shift+KP0
pastes) and `.` (Delete) to the D26I mapping.

Paste. Lines split at `\n` (`\r` dropped). Each line: tabs and U+00A0 become
spaces, ends trimmed, one leading `>` prompt (and the spaces after it) dropped,
letters lowercased (every command is typed lowercase), and `|` (no console
glyph yet, D26L), the backtick, other control and non-ASCII characters left
out. Empty lines are skipped. The text is inserted at the caret; every line
that ends in a newline runs through `debug_console_run` as typed (so it echoes
and enters the history), the rest stays on the line with the caret before the
text that followed it. A command that closes the console (level travel) ends
the paste. Lines keep the 95-character limit. Notes go to the scrollback:
`Paste cut to 95 characters (the console line limit)` and `Left out
characters the console does not take: pipe, backtick, non-ASCII or control
characters` (named in words: the font cannot draw a pipe). Short notes
(`Copied 34 characters`, `Copied the command line`, `Line full (95
characters)`, `The clipboard is empty`) show for 1.8 s at the right end of the
command line.

The selection is a terminal's: typing with a selection in the scrollback keeps
it; a selection in the command line is replaced by typing, removed by
Backspace / Delete, and dropped by any edit that moves the line's text.

## 2. Implementation

- `recomp/src/ttk/ttk_font.cpp`: word wrap and layout are now one internal
  pass (`ascii_placed`, `wrap`, `lay_out`) shared by `ttk_font_rasterize`,
  whose output is unchanged, and the new `ttk_font_layout`, which reports
  every source byte's x, advance and row in the rasterized image (advance 0
  for a byte not drawn: a space dropped at a wrap or a row start). The
  caret, selection and mouse therefore use the exact positions of the glyphs
  drawn, wrapped lines included.
- `host_osd.c` (framework): scrollback lines get serial numbers
  (`s_console_first`) so a selection survives older lines scrolling out of
  the 512 kept. `console_draw_row` lays out and draws each row, paints the
  selection (`0xe02f5f8f`) behind the glyphs, draws the caret (2 px,
  `0xfff2b440`, blinking 530 ms, solid after each edit) and records where each
  byte landed (`s_console_vis`, up to 64 rows). `host_osd_console_hit` maps a
  window point to the nearest row and byte: the console spans the window
  width at 1280 texels, so one scale serves both axes; hits work only with the
  console fully down. Drawing and input run on the main thread (one GL
  context), so the hit table needs no lock. New API in `host_osd.h`:
  `set_caret`, `hit`, `row_text`, `rows`, `set_selection`, `flash`.
- `main.cpp` (framework): the caret, selection model (anchor, head, drag
  unit), keyboard and mouse handling, `SDL_SetClipboardText` plus
  `SDL_SetPrimarySelectionText`, `SDL_GetClipboardText`, paste cleaning and
  running. The old append-only line editing is replaced; every command is
  unchanged. `help` gains one line about paste and copy.
- Debug port `console_ui` (`debug_server.c`): drives the console as the
  keyboard and mouse do, for offscreen tests (the test-input keys never reach
  the console): `{"cmd":"console_ui","op":"open|close|paste|key|mouse|state",
  "text":..., "hex":..., "x":..., "y":..., "clicks":n, "ctrl":1, "shift":1,
  "right":1}`. `hex` carries text with newlines and tabs (the server's JSON
  reader has no escapes). Every reply is the state: window size, caret, line,
  selection text and rows. This covers D26L's "a debug-port command to open
  the console".

## 3. Evidence

Offscreen (Xvfb 1280x720, private settings and card folder, port 9131; the
probe script stayed in the job's scratch folder):

- Editing: Home, Ctrl+Right (caret 4), Ctrl+Shift+Right selected ` 2`, typing
  `x` replaced it, End, Ctrl+W, Left twice, Ctrl+U, Ctrl+A then Delete
  emptied the line.
- Paste: `> GOTO Room 12<TAB>| face 90` became `goto room 12  face 90`;
  `fps\nspawn armor\nhelp` ran two lines and left `help` on the line;
  `history\nfps\n` ran both; a 112-character line was cut to 95 with the
  note; `cafe` with an accented e and a no-break space gave `caf x` with the
  non-ASCII note; a line with a pipe and backticks named "pipe, backtick".
- Mouse: double-click selected `puts`, right-click pasted it at the caret,
  triple-click selected a whole help line (with its stored leading spaces,
  trimmed again on paste), a drag from the command line to the top selected
  all 16 rows (`rows 0:11-prompt:11`), Shift+click extended, a click in the
  line placed the caret at byte 5, Ctrl+C with nothing selected copied the
  line. Screenshots showed the highlight behind every selected glyph, the
  caret and `COPIED 830 CHARACTERS`.
- X11 clipboard, with `xclip` as the other program: text set by xclip pasted
  by right-click and by Shift+Insert (the multi-line one ran each line);
  Ctrl+A, Ctrl+C put the line on xclip's clipboard; a triple-click put it on
  both the clipboard and the primary selection.
- `ttk-font-test` (new layout assertions: positions inside the image, rows
  contiguous, wrap spaces zero-width at a row end, image size equals the
  rows' size, Vanilla refuses), `ttk-inventory-test`, the Python suite, the
  five runtime tests, `check_jobs.py` and `check_repo.py` pass. The pristine
  pinned framework plus the exported patch matches all 99 framework files.
- Executable `24cd83a04a63052bcf917338db90c860394291e1d9232b0b2ac817a645807a6d`
  (normal build-local).

User playtest on the real display, 2026-11-27: accepted ("lock it in and
close it out. im so happy with it"). Wayland was not tried
(the desktop runs X11; SDL3 has both clipboards).

## 4. Limits and follow-ups

- The console font draws capitals only; leading spaces of a line are not
  drawn (wrap rule), but copy keeps the stored text.
- Dragging above the top row does not scroll the scrollback; scroll first
  (wheel, PgUp) and drag within the page.
- Keyboard typing still takes only the D26 scancode set; pasting brings in
  other characters (`_`, `:`, `,`). Wider typing and the missing glyphs
  (pipe) belong to D26L.
