# D08A23 - Instant night vision

Status: Done, user-accepted 2026-10-25. Explicit lock-in and closeout authorized.

## Acceptance and closeout

User: "lock it in, close it out!!". Implementation, tests, manual and
documentation are authorized for commit and push as MusicMonsterMod.
The tested candidate below is the accepted executable. No post-acceptance
gameplay changes, builds or launches occurred. No next job is started.
The runtime patch was re-exported unchanged and verified against clean pinned
framework `08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files match.
Acceptance retains the evidence limits below.

## Agreed behavior

Modern night vision toggles immediately with N or selected-item activation,
without a holster, equip/removal animation, movement lock, camera switch or
added shot delay. Held fire continues. TTK's visual effect and battery duration
remain; the feature copies Duke3D's interaction, not its battery timing.
The persistent NIGHT VISION MODERN / ORIGINAL setting defaults to Modern.
Vanilla is unchanged. D08A24 automatic Bio Mask remains separate and Todo.

## Native contract and implementation

`shortcuts.inc` handles Modern goggles before the fire/weapon-idle gates.
The ordinary gadget request (`800402d0`) sets hand-request flags 0x0c and
starts holstering; it is not used for this toggle. The later legacy command
loop skips an already-handled Modern goggles command and can still process
weapon commands in that snapshot.

The owned executable's `8004e94c..8004ea20` proves the native endpoints:
player+0x360 bit 1 is active, +0x224 bit 0x80000 is the visual effect,
+0x854 is face equipment (3 for goggles, 2 for Bio Mask). The model at +0x40
uses byte +0x34 to select a 40-byte joint record; its mesh pointer at
+0x68 becomes the goggles mesh from `[800ddb18]+0x10c4`, or the original
head mesh from `[[800ddb14]+4]` on removal. Dynamic addresses are range/alignment
checked before any write. Existing guarded single-player ownership, gameplay,
code/overlay identity and health checks apply. Native pending gadget actions
are allowed to finish rather than being commandeered after a load or setting change.

The adapter changes only those effect/model endpoints and inventory selection,
not the hand equipment, animation or weapon-request bits. Bio Mask active bit
is cleared when goggles replace it, with charge retained. The native charge
handler `800420bc..80042208` remains responsible for drain and depletion,
including restoring the head and clearing the effect. Both native contracts
have executable SHA-256 guards. No hook registration, generated code, runtime
framework changes or dependency pin changes were needed.

`game_menu.cpp` adds OPTIONS > ADVANCED > ITEMS > NIGHT VISION, live on
CONFIRM. Launcher schema 51 migrates existing preferences with a backup;
`--night-vision modern|original`, terminal settings N and the in-game row
share `night_vision` / `DNTTK_NIGHT_VISION`. Vanilla always receives Original.

## Evidence

Candidate executable: `ec32ea578286344faea16de7d073a80dc120ff4c0ca72f62f8d4b7c4afaf9ec6`.
Private scripts, saves and logs: `recomp/analysis/d08a23/` (ignored).
Live runs use Xvfb, real keyboard/mouse events, OpenGL, dummy audio, copied
savestates and separate writable cards/settings.

- Build and native controls/input/aim suites pass. The controls test exercises
  the real shortcut dispatcher with held fire and a busy weapon, both activation
  paths, preserved equipment/pose, empty/absent/dead/invalid-model refusal,
  pending native action refusal, Original/Vanilla exclusion and Bio Mask ownership.
- Launcher tests cover migration, persistence, CLI/Vanilla environment selection,
  terminal editing and invalid values, alongside the existing profile suite.
- `check60c.log`: third person at 60 FPS, instant N on/off, inventory Enter
  during fire, repeated toggles while ammo decreases and weapon remains drawn,
  one toggle for held N, movement, native depletion and empty-item refusal.
- `check120.log`: first person at 120 FPS repeats that route, plus private save
  and load with active goggles and pause/resume. The native saved effect/model
  state restores correctly; no host latch is required.
- `original-route.log`: Original rejects N during held fire; after fire ends,
  N enters the native pending/holster path, equips normally, and removes normally.
- `menu-route.log`: the actual in-game row changes Modern to Original and back
  during one session. Held-fire behavior follows the confirmed choice at once.
  Menu screenshots show the row/help text fitting; first/third-person screenshots
  show the original green effect with the weapon present.
- All 40 player source save/card files retain their hashes and modification
  times. Tests write only private copies. Source profile settings are not edited.

Early harness attempts pressed U, which is unbound in the copied player profile,
and used the wrong debug write payload for shortened charge. Corrected routes
use inventory Enter and byte writes with `val`. Those failures are retained in
local logs; neither was a gameplay implementation failure.

## Evidence limits and player check

The user accepted combat feel and presentation. The diagnostic routes
exercise the current slot-1 scene, not every campaign encounter or costume.
Dummy audio does not verify sound quality. Try N repeatedly while firing and
moving, and Enter with the goggles selected, in either camera. Original behavior
is available through the setting above. Modern input captures automatically.

```sh
python3 recomp/tools/local/run.py
```
