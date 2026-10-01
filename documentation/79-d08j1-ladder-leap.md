# D08J1 - Hold-E run-up grab for overhead ladders

Status: **Done** (user-accepted 2026-10-01: "genuinely working solidly"). Modernized only; Vanilla unchanged.

User: the ladder at save slot 6 is awkward to get to; "I want to just be able
to run up to it while holding E and i grab it, but duke seems to bounce off of
it."

## What the original game does

Private copy of the user's UI slot 6 (runtime slot 5) in
`recomp/analysis/d08j1-ladder-run`, real keys on Xvfb, isolated profile and
cards. Duke starts at (24302, -6647, 84469), heading 2021, shotgun drawn,
first person.

- The ladder is object `0x801da0a4`, type 46, flags `0x108212` (a plain ladder,
  0x200), at (24577, -8203, 80918), yaw 0. Its box (index 0) is a 378 x 2046
  panel (`-189,-1023,0 .. 189,1023,0`). Its bottom is at Y -7180 and the floor
  under Duke (`+0x1c8`) at -6143: the bottom rung is about **1040 above the
  floor**. The top meets a platform at about Y -9711.
- The ground mount (`0x80051cf0`, Up + Cross, 185) needs the ladder at the
  feet. Here, W or Shift + W with E held run into the wall below it and stop
  (94, then idle 63, about 245 from the panel). Holding E there does nothing.
- An E jump catches it in the air: the reach (armed at takeoff since D08X)
  attaches it (147 or 148, mode 3), then 210/207 and the original climb
  (156, 186..189) up to the exit 190 onto the platform.
- Space with E held, by takeoff distance from the panel:

| Approach | Caught | Missed |
| --- | --- | --- |
| Running jump | 500, 700, 900, 1200, 1600, 2000 | (from the wall stop: no jump) |
| Walking jump | 600, 800, 1000 | 400, 1300: met the wall low, **bounce 107** |
| Standing at the wall (245) | vertical jump 97 -> 147 | - |

  So the "bounce" is a jump from the wrong distance: Duke meets the wall below
  the rungs and the original impact 107 throws him back.
- The ladder only joins Duke's cell object list (`player+0x220`) within about
  1800 units, which is enough for the windows above.
- The catch also needs Duke inside the panel's width: from 251..269 units
  beside its centre line (half width 189) a standing jump at the wall missed.

## Change (`ladder_top.inc`, `pc_input.cpp`, `modern_controls.cpp`)

- **Leap.** `ladder_leap_ready()` walks the cell list for a plain ladder whose
  bottom is 300..1400 above the floor `+0x1c8`, within the panel's half width
  + 180 sideways, with Duke heading at the panel (within about 50 degrees) and
  inside the catching window for his gait: running (76..79) 750, walking
  (72..75) 800, standing (63) 450 units from the panel. Only on settled ground
  under the modern lease (the D08U ground gates) and holstered (`+0x3b8` 0).
  With E held or just pressed, `input_frame()` then presses the original jump
  once per E hold, exactly as a Space press would (the original chooses the
  running, walking or vertical jump).
- **Reach held through the leap.** While the leap is under way (takeoff,
  flight, until the catch, landing or 45 updates) E's Cross stays held and
  D08X arms the reach, so a tap of E at the wall catches like a held E.
- **Side assist.** In the air (mode 9), Duke eases along the panel, at most 70
  units per player update and only parallel to it, until he is 60 inside its
  width. Nothing moves him toward or away from the wall; the original
  collision, reach and catch are unchanged.
- E stows a drawn weapon first (D08J rules); the leap waits for the stow.
- Telemetry: `ttk_input` -> `controls.ladder_top.leaps` and `leap_nudges`;
  `DNTTK_TRAVERSAL_TRACE=1` logs `ttk-ladder-leap anim=.. xyz=..`.

## Evidence (binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`)

Private slot-6 copy, real keys, no Space pressed (`leap2.py`):

| Case | First person, manual jump | Third person | First person, assisted jump |
| --- | --- | --- | --- |
| Run with E held (stowed first) | caught (103, 148, 210, climb) | caught | caught |
| Walk with E held | caught (96, 109, 148, 190 onto the platform) | caught | caught |
| Run with E held from the start, armed | caught | - | - |
| Walk with E held from the start, armed | caught | - | - |
| Run, E pressed 1600 out (armed) | caught: stow ends at the wall, then the vertical leap 97 | caught | caught |
| Stopped at the wall, then hold E | caught (97, 147, climb, 190) | - | - |
| Stopped at the wall, single E tap | caught | caught | caught |
| Standing at the wall, W released, E | caught | caught | - |
| Run / walk without E | stop at the wall as before, no leap | run: same | run: same |

Several of these started 263..269 units beside the ladder's centre line (the
saved camera heading carried over from an earlier case); before the side
assist those missed, with it they catch.

Native: `ttk-controls-test` (new D08J1 fixture: gait windows, wall stop,
facing, side limit, armed, rise limits, climbing-wall family, airborne and
Vanilla refusal, in-flight side assist and its end on the catch) and
`ttk-input-test` (one jump per E hold, none without E, reach held through the
leap) PASS.

Regressions on the same binary, manual jump style (`regress.sh manual`, outputs
in `recomp/analysis/d08j1-ladder-run/regress-manual`, compared with the D08Z
baseline `d08z-manual-jump/regress-manual`): slot-12 wall E grab and pull-up
and the no-E bounce identical; D08X cases (late E, E at takeoff, pit, tall
wall), crate mantles, crate-to-crate, angle forgiveness, D08U slot-12 regress
and third-person mount (156, 186, descent, 185 step-off, pistol redrawn),
D08W side jumps and ledge sweep, D08Y furniture: same outcomes, differences
only frame timing (one baseline pit-jump flake now catches, as its second
run did). Alley ladder: the W + E ground climb still uses the original 185
mount (its bottom is at the floor, no leap); the alley ladder-top mount route
stopped about 540 short of the edge with its 6 approach steps, and with 20
steps (`alley20.py`) mounted (156 -> 186) and descended to the 185 step-off.
No `ttk-ladder-leap` fired in any regression route.

## Limits

- Verified on the slot-6 ladder only. Other ladders with a high bottom get the
  same help when they meet the gates; the original still decides each catch.
- E pressed late while running armed: the stow finishes about at the wall, so
  Duke stops (94) and then leaps vertically. Holding E a moment earlier gives
  the running leap.
- One leap per E hold: after a miss, release and press E again.
- The windows are measured on this ladder (bottom about 1040 up). A ladder much
  higher (up to the 1400 limit) may need a different distance.
