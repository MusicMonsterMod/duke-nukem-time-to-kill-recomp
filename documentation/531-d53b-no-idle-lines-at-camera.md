# D53B - Duke's idle behaviour: none at a camera, back in walk mode, instant snap-out, IDLE ANIMATIONS and DUKE TALK

Status: Done (user-accepted 2026-11-21: "thats excellent. lock it in and close it out. it works exactly as intended"). Board: `MODERNIZATION_JOBS.md` D53B. Dependencies: D52 (Done). Follow-up: D53G.

## Request

User, 2026-11-14 playthrough: while watching a security-camera feed Duke
sometimes says his impatient waiting lines, which does not fit: the player is
busy. Acceptance: no idle lines during a long camera view; normal idle lines
return after leaving the camera; Vanilla keeps the original.

Widened by the user on 2026-11-21, after the first part ("i'm not sure if any
of duke's idle animations are playing? ... even when not at a sec cam ... so
not just animations, i meant idle behavior"): fix idle behaviour in walk mode
in D53B; "mouse look counts as activity"; "make sure he actually snaps out of
the idle state the second the user interacts with the mouse and keyboard ...
in vanilla, when duke goes idle and he animates, it actually takes a sec for
the player controls to fully kick back in". Choices: a line already playing
when Duke snaps out finishes; the original idle holster stays but the weapon
is redrawn at once; the original ~10 s idle delay stays.

## The original idle

The native idle routine `0x80053038` (from the player update) resets
`player+0x240` to 3000 when `0x8001e070` mode 2 finds any pad button held
(it scans the 20 per-button state words for the held bit) and otherwise
counts it down by the frame step `0x800d21fc`. At zero, armed, it holsters
(`0x8004d4a4`: upper 5 -> 6 -> 0, equipment `+0x3b8` 2 -> 0, the weapon byte
`+0x3ba` kept); unarmed it starts a waiting fidget, lower/upper animation
0x40-0x45 (64-69), cycling through `player+0x232`. About 10 s at 60 Hz.

Audio (output tap with `PSX_TRACE_HISTORY=1`, standing still): fidget 66 is
followed about 1 s in by a loud, voice-length sound (~2 s, RMS from about 700
to 3600-7200 over the CD music); fidgets 65 and 68 trigger one short SPU
sample (0x1620) about 0.6 s in. Which other fidgets speak was not mapped.

## Causes (reproduced)

Offscreen, private copies of the player's UI slot 9 savestate (file 8, LEVEL00
security room) and cards, first person, default walk mode:

1. **No idle in walk mode.** Modernized walk mode holds the original Walk bit
   in captured locomotion, so `0x8001e070` always reported activity: with
   capture on, 25 s standing still kept the timer at 3000 and Duke never
   fidgeted, spoke or holstered. With Shift held (run, no Walk bit) fidgets 65
   and 68 came at 8 and 19 s. (The lab's usual uncaptured runs never apply the
   Walk bit, which first hid this.)
2. **Lines at a camera.** During a security-camera view (Duke `+0x224` bit
   `0x20000000`, the alternate camera, D52) the Walk bit is not applied either
   and the pad is neutral, so the countdown ran: fidgets 65, 68 and 66 at
   10.1, 20.7 and 31.8 s of a 30 s view. That is why the user heard the lines
   at cameras and nowhere else.
3. **Slow return.** In run mode (where the original idle ran), W pressed
   0.3 s into fidget 65 waited 61 frames (about 1 s) for the fidget to end;
   fire pressed 0.5 s into fidget 68 waited 61 frames, then the weapon draw,
   first shot about 1.4 s after the click.

## Change (Modernized only)

- `pc_input.cpp`: a player activity clock for captured Modernized play,
  `input_activity_serial()` / `input_idle_ms()`. Activity is any held action
  but the Walk modifier, movement, the arrows, queued commands (weapon keys
  and so on), or mouse/stick look of 3 or more counts in one input frame (a
  smaller nudge is not); resuming capture counts too. The Walk bit is let go
  when `idle_walk_release()` says so.
- `console_idle.inc` (the D26K owner, called from the authenticated player
  update `0x8005a210` from `0x80041b34`), now the idle owner:
  - `idle_walk_release()`: the player has been idle 0.5 s with Duke standing
    (lower 63 or 64-69, mode `+0x22c` 0), so the pad lets go of the Walk bit
    and the original countdown runs. Stops and gaits keep the Walk bit as
    before.
  - Activity resets the countdown to 3000, in walk and run mode alike, so
    mouse look counts.
  - Activity during a fidget 64-69 ends it in that update: standing idle 63
    restarts at frame 0 (lower `+0x60/+0x68/+0x6a`, and upper `+0x74/+0x7c/
    +0x7e` when it plays the same fidget), the way the original starts any
    animation; the update then dispatches the input as usual. The fidget's
    lower-body path never reaches `0x800493a4` (where kick_convert works), so
    a first version there never fired. A line already playing finishes.
  - The original idle holster stays. When the countdown runs out with a
    weapon out, the holster is recorded once equipment reaches 0 (with the
    weapon in `+0x3ba`); the next activity requests that weapon through the
    original weapon request (`+0x224` bit 4, as the D08J7B mantle draw does)
    unless the player pressed holster or a weapon key, and the draw runs at the
    2x rate (`idle_draw_boost`, beside `mantle_draw_boost` in the D08J6A
    transition boost). A holster the player asked for is never redrawn.
  - Camera view and console holds (part one, below) take precedence.
- Debug: `ttk_input` -> `controls.idle` (timer, idle_ms, walk_release, resets,
  snaps, holsters, draws, draw_drops, the view and console keeps).

### Part one: the camera hold

While Duke `+0x224` has bit `0x20000000` the countdown keeps the value it had
when the view began (the use press reset it to 3000); a reset upward is kept;
leaving resumes the original countdown. The view moves the game camera off
Duke, so this uses Modernized, game mode and the code identity instead of
`player_identity_ready()` (camera `+0xa4` == Duke); the input epoch advances
every update during the view, so the hold is latched for the continuous view,
not per epoch as the console hold is.

No hook, codegen or framework change. Vanilla: every path requires
`input_modernized()` (the pad change is in the Modernized pad path), so Vanilla
keeps the original idle, its delay and its camera lines.

## Evidence

Probes `recomp/analysis/d53b/` (local: `probe.py`, `idle.py`, `armed.py`,
`interrupt.py`, `stand2.py`, `camcap.py`; private card and savestate copies,
offscreen, capture on, debug port; player saves, cards and profile hashes
unchanged after every run). The probe polls every 1-2 frames.

- Walk mode, standing: the idle holster about 8 s in, then fidget 65 (16.4 s
  after start); the next fidget about 10 s after the previous.
- W 0.3 s into a fidget: fidget ended by frame 5 (before: 61), walking (72) at
  frame 7, Duke moving at frame 7, the redrawn weapon ready (upper 5) at
  frame 13.
- Mouse look (30 counts) in a fidget: ended by frame 4 (before: no effect),
  weapon ready by frame 12.
- Mouse nudge of 1 count per frame for 3 s: not activity (the countdown ran);
  a slow look (4 counts every 0.25 s): the timer stayed at 3000.
- Fire after an idle holster: first shot (upper 16) at frame 18 (26 without the
  2x draw; about 84 frames from inside a fidget before).
- Run mode (Shift held): idle as before (holster, fidget 65 at 16.4 s).
- Security camera, capture on, 35 s view: timer 2980-3000, no fidget; uncaptured
  40 and 45 s views: the same; idle resumes after the view.
- Native: `ttk-controls-test` D53B groups (camera hold; idle owner: activity
  reset, fidget end at once with the upper following, idle holster redrawn on
  the next activity, a chosen holster not redrawn, Vanilla untouched); the
  camera group fails without the hold. 158 groups pass, D26K included.
  `ttk-input-test`: the Walk bit follows `idle_walk_release`; the activity
  serial advances for held W and a 3-count look, not for a 1-count nudge or
  Shift alone. Local unit tests 268 OK (2 skipped).
- Final executable `a722318a219cf5b41fe8fd9e07c597ce08f791b1af03e701e569cedcb655b55a`
  (the in-game runs above used the build before a comment-only edit).

## Part three: IDLE ANIMATIONS and DUKE TALK (2026-11-21)

User: "i dont mind when duke fidgets but i think he should always reholster
the weapon if he lowers it for any reason. you cant actually see him crack his
knuckles or whatever in first person, so just randomly lowering the weapon just
feels off ... I'm thinking we have a switch in options for idle animations on,
voice only, or off"; "i think we need a duketalk toggle in sound like
eduke32"; "the 3 tier system for idle can still work, but it needs to
carefully grey something out"; "yeah the speech slider can grey out too";
"make something that genuinely fits and has good UX for this predicament";
then "work autonoomously on it". The agent chose the details below.

### Findings

- The idle's speech: the lower-body event handler `0x80048410` (jump table
  `0x800140a8`, anim - 63) gives fidget 66 the case `0x80048478`, which on its
  event frame says one of lines 143, 329, 330, 331 (random & 3) through the say
  wrapper `0x8006cad8` (skipped while `0x800d73c4` is 5). Fidgets 64, 65 and 68
  play sound effect 2 (`0x8006b270`, the knuckle crack) and 69 effect 3. The
  idle counter `+0x232` picks 66 on every third beat (0), a random 64/65/67/68/69
  otherwise; observed: line 331 about 49 frames into fidget 66.
- Speech is one XA system: `0x8003c000(line)` streams line < 397 from the table
  at `0x800c0dac` (16 bytes a line) and sets the CD input volume to the SPEECH
  word `0x800c39e8` (`0x8006ad48`); music goes back to the MUSIC word
  (`0x8006ad98`). All about 40 EXE call sites of the say wrapper are player,
  pickup and weapon code. No second speaker was found, but the 397 lines were
  not all listened to; if another character speaks, DUKE TALK OFF mutes it too.
- EDuke32 (`research/eduke32`, local): Duke talk is `ud.config.VoiceToggle`
  bit 1; `S_PlaySound`/`A_PlaySound` drop `SF_TALK` sounds when it is off and
  do not start a talk line while one plays. The menu greys DUKE TALK and the
  Speech slider only when sound itself is off: there Speech also scales
  `SF_SPEECH` voices that are not Duke's.

### Design

- **IDLE ANIMATIONS** (OPTIONS > GAMEPLAY, `idle_animations` on/voice/off,
  `DNTTK_IDLE_ANIMATIONS`, `--idle-animations`, schema 81):
  - ON: third person, the original idle with two changes: after the idle
    holster the fidget follows within about a second (the countdown is cut to
    300), and once the fidget ends the weapon comes back by itself through the
    original weapon request at the normal draw rate (activity before that
    still redraws at once at 2x). First person (the global VIEW): played as
    VOICE ONLY, since a fidget cannot be seen there and a lowered weapon only
    looks wrong. D53G (backlog) is for first-person idle poses.
  - VOICE ONLY: the native countdown is kept full; the owner runs the same
    cadence (3000 units by the frame step, only while Duke stands in 63 on the
    ground) and the counter `+0x232`; on the beat where the original plays
    fidget 66, Duke says one of the same four lines through the original say
    (a host call on a saved stack, as `damage_flash.inc` makes). No holster, no
    animation. A fidget already under way when the mode applies ends.
  - OFF: the countdown is kept full and nothing plays.
  - Activity, snap-out, camera and console holds as in parts one and two.
- **DUKE TALK** (OPTIONS > SOUND, `duke_talk` on/off, `DNTTK_DUKE_TALK`,
  `--duke-talk`): OFF plays every speech line at zero CD gain while it plays
  (speech states 5-7, `concurrent_music.cpp`, also without the second music
  bus), so timing and anything waiting on a line stay the original's. The
  guest SPEECH value is never changed (so a memory card or the original screen
  never stores a muted value). It is a controls row on the SOUND screen, not
  one of the five sound slots.
- **Greying** (the menu's D19N `needs()` rule: the row stays changeable, is
  drawn grey and its description says why): SPEECH with DUKE TALK OFF shows
  its label grey and MUTED in place of the bar ("MUTED: DUKE TALK IS OFF"),
  and keeps its value; IDLE ANIMATIONS is grey only while it is VOICE ONLY and
  DUKE TALK is OFF ("DUKE TALK IS OFF, IN SOUND: NO IDLE"); that pair behaves
  as OFF. ON with DUKE TALK OFF stays live (fidgets without words).
- Vanilla: `run.py --vanilla` passes on/on and the owner and mute need
  Modernized.

### Evidence (part three)

Offscreen, capture on, private card and savestate copies (UI slot 9),
`recomp/analysis/d53b/modes.py`, `idle.py`, `live.py` (local):

- Third person ON, armed: holster, fidget 66 1.1 s later, line 331, the weapon
  back 1.6 s after the fidget began while the line plays on; repeats about
  every 11 s; fidget 65 and 69 cycles the same.
- First person ON and third person VOICE ONLY: weapon up throughout (upper 5,
  equipment 2), lines 331 and 330 about 30 s apart, no animation.
- OFF: 35 s standing armed, nothing. Live switch by the console's `option`
  (the menu's apply path): OFF applied at once.
- DUKE TALK OFF, third person ON: the fidget and line 331 run as before; the
  output peak is 1503 (RMS, 0.1 s windows) against 7251 with DUKE TALK ON on the
  same route.
- Snap-out (third person): W in a fidget, fidget over by frame 4, weapon back
  at frame 14; mouse, frame 4; fire in the fidget after the holster, first shot
  at frame 22.
- Native: `ttk-controls-test` D53B idle-owner group extended (OFF, VOICE ONLY
  beat, DUKE TALK OFF, redraw after the fidget, the fidget within a second);
  158 groups pass. `ttk-music-test`: zero gain while muted, SPEECH unchanged,
  music states untouched, also without the music bus. All native test targets
  pass. Profile tests: new IdleAndDukeTalkTest (migration from 80, CLI, env,
  Vanilla, validation); 152 pass. Local unit tests 269 OK (2 skipped).
- Not checked by the agent: the OPTIONS screens themselves (the scripted
  input does not reach the paused menu offscreen); the grey rows need the
  user's look.

## Remaining

- The OPTIONS rows and their greying were not seen by the agent; the user's
  playtest accepted the job as a whole.
- Other ways Duke lowers his weapon (for example a climb's sidearm with SIDEARM
  RETURN NEVER) are not changed; only the idle holster now always comes back.

- The snap-out is an instant pose change from the fidget to standing (as
  kick_convert's restart); judge it in third person.
- A fidget ended before its line starts never says it (the line only plays
  when Duke gets that far).
- Crouched idle (the routine's 0xb2/0xb3 branch) is not covered by the
  snap-out; not observed in these tests.
- The camera hold covers every view with the script bit `0x20000000`, not
  only security cameras.
- Vanilla is covered by construction and the native groups; the scripted input
  driver does not run in Vanilla.
