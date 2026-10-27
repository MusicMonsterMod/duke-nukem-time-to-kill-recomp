# D35H - Separate overhead traversal perspective

2026-10-25. Done, user-accepted. Explicit implementation, documentation,
commit and push closeout authorized: "lock in this work and close it out".

## Finding and behavior

The user reports successful first-person monkey-bar travel from their slot 9.
The D35 audit's statement that overhead traversal always uses orbit is stale:
D35G's mode-7 149-151 mantle classification already supplies a stable body eye
and native interaction hands for ceiling hangs. No new movement or eye model
is needed for this option.

OVERHEAD VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers FIRST PERSON
(default) / THIRD PERSON. CONFIRM applies and saves it. Global Third Person
wins. Modern controls and Independent camera remain prerequisites. Schema 47
adds `overhead_view`; `run.py --overhead-view first|legacy-third` saves it.
MANTLE VIEW now remains independent of ceiling hangs.

## Implementation

Presentation-only Overhead classification uses attached object flags masked
with 0xc0 equal to 0x80, mode-7 poses 149-151, shared mode-6 catches 147/148 and
the immediate mode-9 drop from 7. Crates (0xc0) and ordinary ledges retain
Mantle. The shared eye, native hands, world-depth clipping and replay snapshot
paths are reused unchanged. D08J4's camera-relative movement, native object
transfers, collision and drop controls are untouched.

The user slot-9 private copy loads on the outdoor moat walkway, not hanging.
Its route to monkey bars has not been reproduced in this task. Live overhead
checks use the historical D08J4 medieval chain-top fixture, copied into a new
private directory. The player's cards, saves and preferences are not writable
test inputs.

## Verification

Accepted executable SHA-256:
`3fd09af36c36682d3ca94da1bba1c4bad4229dbd5060c4de3f9410bd78b3a164`.

- Player build and native controls/input/aim/near suites pass. Controls uses
  the owned EXE, clean LEVEL00 fixture, LEVEL01 and all selectable overlays.
- Policy tests cover 8192 fact combinations. New native checks cover overhead
  versus crate/ledge ownership, opposing mantle/overhead choices, global view,
  paused input, save acquisition, attachment and movement-permission isolation.
- All 97 profile tests pass, including schema-46 migration with backup,
  validation, persistence, menu sidecar merge, launcher and Vanilla settings.
- Private first-person route at the 60 FPS setting: 285 hanging samples all
  resolve Overhead, effective First Person with blend 1.0, while MANTLE VIEW
  is Third Person. Forward, backward, left, right, diagonal and turned travel,
  end holds and saved-hang reload span four native grate objects. Screenshots
  show the eye below the ceiling and native arms when looking up.

- Private Third Person comparison at the 120 FPS setting: 239 hanging samples
  resolve Overhead with effective First Person false and blend 0.0. The same
  direction inputs produce the same steering headings and grate route, with
  normal native travel/edge stops. These wall-clock sampled runs are not
  frame-for-frame trajectory equality tests. Held Ctrl and Space release;
  global first person resumes through the subsequent fall/landing.

- Live Advanced menu while hanging: row layout inspected, cancel produces no
  writes, CONFIRM switches to Third Person and back, global P both ways,
  and first-person Ctrl/Space drops all pass. Reloading the private hanging
  save reacquires the selected perspective. All diagnostic sessions stopped.

The first short synthetic Ctrl/Space taps were too brief to reach a game tick;
those samples stayed hanging and are not drop evidence. Held-input reruns
above verify drops. The first historical catch attempt pressed E before Space
and mounted the nearby chain; Space followed by held E reproduced the grate
catch without moving or editing the fixture.

Evidence and scripts: `recomp/analysis/d35h/` (ignored). Dummy audio and frame
rate settings are not audible or performance acceptance. Full campaign,
all costumes, every catch/transfer intermediate frame and the current slot-9
route remain outside the recorded coverage. User accepted the new option; this does not establish additional playtest
coverage beyond the evidence recorded above. No framework source, generated C or submodule pins changed.

```sh
python3 recomp/tools/local/run.py
```

P selects global First Person. Try OVERHEAD VIEW both ways and CONFIRM on the
user's monkey-bar route. Movement stays camera-relative; Ctrl/Space drops.

## Closeout

Authored implementation, regression tests, manual and job records are included
in the authorized MusicMonsterMod commit/push. Runtime patch re-export is
unchanged; application to the clean pinned framework index reproduced all 51
changed/new files. No pins changed. No post-acceptance gameplay edits, rebuilds
or launches; no player data touched. D35F1 records the reported slot-10 chain
visibility problem as Todo only, explicitly deferred by the user.
