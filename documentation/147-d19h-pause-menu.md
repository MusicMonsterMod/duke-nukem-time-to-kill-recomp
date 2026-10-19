# D19H - In-game pause menu and death screen as lists

Job: [D19H](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "accepted!! this works and it's functional"); follow-ups D19I-D19L.
Builds on [D19](145-d19-game-menu.md) (our screens drawn by the game's own
calls), [D19D](144-d19d-menu-keys.md) (PC menu keys) and
[D19E](143-d19e-menu-closeout.md). Design: layout B of the D19H artboard on
the private D19 mockup canvas (user 2026-10-19: "Layout B").

## Before

Escape in gameplay opened the original pause menu: the mission briefing with
four PlayStation prompts and no cursor (Cross RESUME, Circle LOAD, Square
OPTIONS, Triangle QUIT, then RESTART / MAIN MENU / CANCEL and a YES / NO
prompt). Without knowing that C is LOAD, V is OPTIONS and Z is QUIT, a PC
player could only resume. The death screen (CONTINUE / LOAD / RESTART / QUIT)
had the same problem.

## What the player gets (Modernized)

Pause (Escape), in the original menu font and colours, RESUME highlighted:

    <LEVEL NAME>
    RESUME / MISSION BRIEFING / OPTIONS / LOAD GAME / RESTART LEVEL / MAIN MENU
    SELECT  CONFIRM  RESUME

- MISSION BRIEFING shows the original briefing (title, story, goals) with
  RETURN. It is listed only for levels that have one (levels 0-12).
- OPTIONS is D19's OPTIONS; going back returns to this list on OPTIONS.
- LOAD GAME is the original LOAD screen (one player only, as the original);
  CANCEL returns to this list on LOAD GAME.
- RESTART LEVEL (not in challenge stages 21-26, as the original) asks
  RESTART LEVEL? and MAIN MENU asks REALLY QUIT?, both YES / NO with NO
  highlighted.
- Escape on the list resumes. In a sub-page or prompt it goes back one level.

Death screen, under the original "N CONTINUES REMAINING":

    CONTINUE / LOAD GAME / RESTART LEVEL / QUIT        (SELECT  CONFIRM)
    REPLAY LEVEL / GIVE UP                             (challenge stages)

- CONTINUE (REPLAY LEVEL) asks CONFIRM CONTINUE (CONFIRM REPLAY) with YES
  highlighted, as the original asks (user 2026-10-19: keep it original for
  now; the continues system will probably be scrapped later, see the board).
- RESTART LEVEL asks RESTART LEVEL? and QUIT / GIVE UP ask REALLY QUIT?, NO
  highlighted. QUIT goes to the game over screen and GIVE UP to the
  statistics screen, as the original does.
- Escape does nothing on the list (there is nothing to resume) and cancels in
  the prompts.

Keys: arrows (held: repeat after 350 ms, every 80 ms) and the mouse wheel move;
Enter (or X) confirms; Escape, Backspace (or Z) go back. Fixed keys,
independent of bindings and of mouse capture: the lists free the mouse when
they open (the death screen opens from captured play), and gameplay
recaptures as after any pause. Vanilla keeps the original screens.

## How it works

`recomp/src/ttk/game_menu.cpp`, in the same tick hook D19 uses at the pause
dispatch `0x80025fb0` (entry, before the screen's handler runs):

| Original | Screen value | Ours |
| --- | --- | --- |
| Pause menu, handler `0x800857e4` | `0x800be56a` = 12 | 8 (the slot D19's pause OPTIONS uses) |
| Death screen, handler `0x8008647c` | 15 | 9 |

Slots 8 and 9 have no handler (table `0x80013558`) and nothing reads them,
unlike 1 and 13, which the main loop takes as main menu and restart. The guest
draws nothing on them and the paused world stays behind. Our pages are drawn
from the composition hook like D19's (since D19I: from the pause-dispatch tick,
after the darkening quad, [note 148](148-d19i-menu-brightness.md)), with the
game's text, sprite and sound calls. Titles and prompts come from the game's text table: the level name
(`0x800c3d2c`), the briefing ids (`0x800c3e10`, ten per level 0-12), YES /
NO, RESTART LEVEL?, REALLY QUIT?, CONFIRM CONTINUE / REPLAY and
CONTINUE(S) REMAINING.

### Each choice is the original's own sequence

The original handlers are never run for a final choice. A first build set
their prompt state and fed them one Cross press. The result was right, but
the handler draws its screen in that tick, so the old pause screen (or "3
CONTINUES REMAINING / CONFIRM CONTINUE") showed for one image after resuming.
Instead the tick does what the handler and the main loop do:

| Choice | Handler part | Then |
| --- | --- | --- |
| RESUME | sound 0, `0x800c3f28` = 1 | main-loop resume |
| RESTART LEVEL (pause) | sound 1, `0x800c3f28` = 1 | screen value 13 (the main loop restarts) |
| MAIN MENU | sound 1, `0x800c3f28` = 1 | screen value 1 |
| CONTINUE | sound 1, byte `0x800d71cd` = 2 (challenge: continues byte `0x800d7411` - 1, event `0x80094a38(0xfd)`), `0x800c3f34` = 1 | main-loop resume |
| RESTART LEVEL (death) | sound 1, event `0xfe`, `0x800c3f34` = 1 | main-loop resume |
| QUIT / GIVE UP | sound 1, `0x800c3f34` = 1, event `0xff` (challenge `0xfe`) | main-loop resume |

The main-loop resume (`0x80026950`-`0x80026994`): `0x800be568` = 0, player
flags `+0x224` |= `0x4000`, `0x8006ac24`, `0x8003c3dc`. The screen value
stays 8 / 9 through that dispatch (0 would run handler 0, which draws
PAUSED) and is set back to 0 at the next composition.

LOAD uses the original LOAD screens: pause screen 5 with `0x800c3f28` = 1, and
death prompt `0x800dd1c6` = 3 on screen 15. When they return (12, or prompt
0) the list comes back on LOAD GAME.

Savestates taken on our pages load to the list (pause on 8, death on 9).

### Taps

`pc_input.cpp` counts menu key-down events (arrows, Enter/X,
Escape/Backspace/Z, V) and hands the counts to the menu on every pad read,
dropping them while no menu owns the keys. Our menu acts on these counts only
(held state drives repeat). The death screen reads the pad seldom, and
sampled 30 ms taps were lost there. Taps that arrive while the menu is drawing or
waiting for an action's tick are kept for the next read. With test input, the held key showed
before its event and a single Enter counted twice.

### Identity

Guards (SHA-256 on first use) add the pause and death handlers, the
main-loop dispatch and resume block, the event, resume sound and resume
music entries, the level name and briefing tables and the text table up to id
`0x19a`. Fixed at the same time: D19's "font glyph tables" guard began at
`0x800c3f30`, which covers the menu variables `0x800c3f32` (pause OPTIONS
cursor) and `0x800c3f34` (death reset flag). It now starts at `0x800c3f38`.
Before the fix, a state where those variables differed from the executable
(the private death savestate) failed the check and kept the original menus,
OPTIONS included.

## Evidence

Offscreen on the real GPU, private profile, cards and savestates
(`recomp/analysis/d19h-pause`, copied from D19; slot 5 gameplay level 0,
slot 8 a private death screen made there with `DNTTK_GAME_MENU=off`; script
`p.py`, local). Executable
`681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2`.

| Check | Result |
| --- | --- |
| Escape in gameplay | list, RESUME highlighted, level name, SELECT / CONFIRM / RESUME |
| MISSION BRIEFING, Escape | original briefing text and RETURN; back on MISSION BRIEFING |
| OPTIONS, Escape | D19 OPTIONS; back to the list on OPTIONS |
| RESTART LEVEL / MAIN MENU | YES / NO prompts (NO first); Escape back on the item |
| Escape / Enter on RESUME | gameplay, screen value 0; 24 presents at 120: our list, then gameplay (no old screen) |
| RESTART LEVEL YES | loading spinner, level restarts |
| MAIN MENU YES | mode 3, as the original (`DNTTK_GAME_MENU=off`, Z C X) |
| LOAD GAME, Escape | original LOAD screen (no save on the test card); back on LOAD GAME |
| Death list | "3 CONTINUES REMAINING", CONTINUE / LOAD GAME / RESTART LEVEL / QUIT; Escape nothing |
| CONTINUE, YES | Duke respawns; presents: our prompt, then gameplay (no old screen) |
| Death LOAD, Escape | original LOAD (prompt 3); back on LOAD GAME |
| Death RESTART LEVEL YES | spinner, level restarts |
| Death QUIT YES | game over (mode 3), as the original; title after Enter, screen value 0 |
| Challenge stage 1 (level 21) | pause list without RESTART LEVEL or MISSION BRIEFING; death REPLAY LEVEL / GIVE UP |
| REPLAY YES / GIVE UP YES | continues 3 -> 2 and gameplay / statistics (mode 2); the original gives the same |
| Held Down 700 ms, wheel | repeats (six steps wrap to RESUME); two notches up land on RESTART LEVEL |
| Title OPTIONS (slot 1) | D19 OPTIONS, MODERNIZED, back to the main menu |
| Vanilla | original pause screen (12), Cross resumes |
| Tests | Python 146 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`, `check_repo.py` |

Not verified: the real display and feel; the captured-to-death path with a real
mouse (the test input forces the capture state, so the capture release on
the death list is untested); LOAD with a saved game on the card (the test
card has none); two-player.

## Limits

- The original pause menu's button-sequence cheats (entered on the pad in
  the pause menu, `0x80083680`) are not read while our list is up. The typed
  PC cheats (D08G) are unaffected.
- After QUIT or GIVE UP the screen value reads 9 until the next composition
  (title or next level). Nothing reads it in between.
- Prompts still show PlayStation buttons (D19F, D19F1).
- The text was drawn at about half the original's brightness in gameplay;
  D19I draws it at the pause dispatch instead ([note 148](148-d19i-menu-brightness.md)).
- The continues system is kept as the original's for now. The user expects
  to scrap it later; then CONTINUE and the count change here.
