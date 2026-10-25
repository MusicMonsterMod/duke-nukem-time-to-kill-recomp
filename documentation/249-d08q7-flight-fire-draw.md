# D08Q7 - Draw a holstered weapon with fire during jetpack flight

2026-10-25. Done, user-accepted.

## Acceptance

User: "perfect. lock it in, close is out". Gameplay accepted and implementation,
documentation, commit and push closeout authorized. No further gameplay edits,
builds or launches during closeout. The runtime patch re-export is unchanged
and reconstructs all 52 modified/new files against the pinned clean framework.
The automated coverage limits below remain; no next job started.

## Cause and implementation

`fire_draw_ready()` required `interaction_restore_ready()`, whose ground
movement lease intentionally excludes jetpack flight. The private baseline
entered flight with equipment 0 and upper/body animation 164; holding Mouse1
left it holstered with zero adapted shots.

Fire-to-draw now also accepts the authenticated jetpack lease when equipment
is holstered, upper/body animations match, a non-Boot weapon is selected,
and the native action/transition flags used by flight weapon shortcuts are
clear (`+0x224 & 0x2c5`). The input layer still sends its existing single
Circle tap and suppresses unarmed Cross. Native code owns drawing and firing;
no shot is buffered after release. E-owned automatic restoration stays
ground-only. No new hook, generated-code edit, framework edit, inventory
write, flight physics change or Vanilla behavior change.

## Verification

- Incremental player build passed.
- Full `ttk-controls-test` passed with the owned EXE and pristine LEVEL00
  fixture. Added D08Q7 coverage for all eight flight animations, Boot,
  equipment/action transitions, upper-body mismatch, capture, death, camera
  ownership, modified native code and Vanilla refusal. D08Q5 switching and
  existing ground fire-draw checks also pass.
- `ttk-input-test` under Xvfb and `ttk-aim-test` with the owned EXE passed.
- Private live evidence: `recomp/analysis/d08q7/`, port 9437, offscreen
  OpenGL, dummy audio, private profiles and copied test savestates/cards.
  The harness rebinds holster to C only in its private profile.
- Modern at 60 and 120-target: holstered takeoff, held Mouse1 draws/fires;
  the 120-target route also checks tap-to-draw with no shot, mouse aiming,
  switching shotgun to Gatling, moving fire, J off, landing and ground fire.
- EDuke32 at 120-target, first person: the complete draw/tap/aim/switch/
  moving-fire/landing route passes with the same expected results.
- Classic at 60, third person: the same route passes with Space held for
  thrust. A tap during unfinished holstering is refused as intended; after
  the upper-body transition settles, the tap draws without a shot.

Accepted executable SHA-256:
`f3d2f69f4d0e8272d96bd94a52849dcc4dde605905143fd15b605760d6a73de8`.

These are functional state/shot-counter checks, not visual acceptance or
sustained frame-rate measurements. They do not establish every weapon,
upgrade or campaign location. Player cards and saved preferences were not
used as writable test destinations. The pre-existing framework modifications
were left untouched.

## Player check used for acceptance

Use the reported location, with an owned charged jetpack and a firearm with
ammo. Select the firearm, tap Scroll Lock to holster and let that animation
finish. J starts Modern/EDuke32; Classic also needs Space to lift.

1. In flight, hold left click: draw, then fire, without another holster press.
2. Release fire, let the attack finish, holster again and wait until settled.
   Briefly click and release: draw only, with no delayed shot.
3. Fire while looking up/down, turning and moving; switch weapons with a
   number key or wheel after the attack finishes, then fire again.
4. J off, land and check ground draw/fire; take off holstered once more.
5. Repeat the core draw checks in first and third person and all three
   jetpack schemes. F10 captures Modernized controls if needed.

Normal launch preserves the player's saved preferences:
`python3 recomp/tools/local/run.py`.
`--jetpack modern|classic|eduke32` selects and saves a scheme on launch.
