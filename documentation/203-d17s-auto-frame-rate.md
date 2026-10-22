# D17S - Automatic frame rate

Status: Done (user-accepted 2026-10-22). The user fully accepted the result
and explicitly authorized documentation, commit and push closeout.
Accepted executable: `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.

## Behavior

AUTO is the new Modernized default for fresh profiles and the DISPLAY screen's
DEFAULT action. Existing saved rates and historical migrations retain their
choices. CLI: `--frame-rate auto`; menu: OPTIONS > DISPLAY > FRAME RATE > AUTO.
Both save the preference. Manual rates, Display and Unlimited remain available.
Vanilla keeps its original presentation and launch override.

The compatibility-first candidate uses `refresh / ceil(refresh / 120)`.
Each target period spans a whole number of monitor refreshes, at most 120 FPS,
without assuming VRR is enabled. Examples: 60 -> 60, 120 -> 120, 144 -> 72,
165 -> 82.5, 180 -> 90, 240 -> 120, 360 -> 120. Below 120 Hz it follows the
monitor (75 -> 75, 50 -> 50). Fractional rates remain fractional, including
59.94 and 239.76 -> 119.88. Unknown/invalid refresh falls back to 60.
This selects a target cadence; it does not promise tear-free scanout or detect
VRR. The existing presentation scheduler and overload safeguards remain.

## Implementation

- `player_profiles.py`, `run.py`, `game_menu.cpp`: new allowed value, default,
  description and menu choice. Vanilla defaults stay 60. No saved user profile
  is rewritten by the implementation work.
- `frame_replay.cpp`: AUTO is request -2, distinct from Unlimited (-1). It uses
  the actual rate supplied by the renderer to plan interpolated images. AUTO
  keeps replay available at 60 so a later display change can increase the rate;
  explicit 60 still stops replay and returns to the original path.
- Framework `replay_rate.h`: pure target resolver. `gpu_gl_renderer.c` uses it
  for deadlines, provider information and diagnostics. Host-mode changes reset
  the AUTO/Display deadline and interpolation schedule; explicit rates stay
  fixed. The existing SDL3 display/window events drive re-evaluation.
- Framework `main.cpp` and `mod_plugins.h`: accept and describe -2 at startup,
  live replay start and live retarget. The complete patch is exported.

## Evidence

- Local-dev incremental build succeeds.
- Compiled production resolver/setters test passes: 24, 50, 59.94, 60, 75,
  100, 119.88, 120, 144, 165, 180, 239.76, 240, 360 and 500 Hz; invalid
  inputs; target cap/divisibility; display-change deadline reset; fixed-rate
  preservation and transitions through Display, Unlimited and AUTO.
- All 88 player-profile tests pass, including persistence of every frame-rate
  choice, AUTO's CLI environment, Vanilla isolation and historical migration.
- Real GPU, SDL offscreen, private profile/cards and copied savestate 11 under
  `recomp/analysis/d17s-auto`, port 9431. No player saves or settings used for
  writes. Unknown display refresh selects 60. Menu screenshot inspected.
- In-game AUTO -> 120 -> 60 -> AUTO -> Display -> AUTO applies correctly.
  Measured 118.8 presents/s at fixed 120; original 60 disables replay; returning
  to AUTO gives 59.7 presents/s. Saved profile retains AUTO after exit.
- AUTO camera-turn smoke: 60.1 presents/s, 59.8 distinct images/s and 59.8 guest
  fields/s. Vanilla smoke: 59.7 fields/s with zero replay presents.
- Exported patch applies against the pinned clean framework and reproduces all
  51 changed/new files byte-for-byte. Repository checker and diff whitespace
  check pass.

## Acceptance and measurement limits

The rate matrix above is a compiled production-code test, not physical display
measurement. Offscreen cannot establish real-monitor frame pacing, VRR behavior,
or multi-monitor/fullscreen mode-change behavior. The user fully accepted the
result on 2026-10-22; this does not expand the agent-measured hardware coverage.
Windows and SDL2 have not been exercised. The new AUTO rate uses the same
presentation path as the accepted higher-rate modes, not a simulation change.

Launch from the workspace with:

```sh
python3 recomp/tools/local/run.py --frame-rate auto
```

This saves AUTO. Test smooth camera turning in a familiar scene and, if available,
move between displays or switch fullscreen. F10 captures the mouse if needed.
