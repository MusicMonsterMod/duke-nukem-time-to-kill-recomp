# D08O5 - Left click draws a holstered weapon while swimming

2026-11-01. Done, user-accepted ("very good work!! i can confirm it works.").
Follow-up D14G: holstered swimming arms look disembodied in first person.

## Report

User (2026-10-30, again 2026-10-31): "if you are swimming with the weapon
holstered, left click doesnt draw your weapon like it does on land. It needs
to draw the weapon." Land (D08E) and jetpack flight (D08Q7) already draw with
fire; the native swim states (`player+0x22c` 4 surface, 5 underwater) did not.

## Cause

`fire_draw_ready()` admits the ground movement lease and settled jetpack
flight only. In the swim states the ground draw never ran, and the original
swim handler ignores Cross while holstered (D08O1 limits: "holstered fire does
not draw the weapon in water (original)").

Baseline (private slot-2 copy, moat in level 6, Desert Eagle): holstered and
underwater, held left click 1.5 s left equipment 0 and 0 shots; the same in a
stroke (anim 128) and on the surface while swimming (125).

## Change (Modernized only)

- The draw is the original weapon request (`+0x3ba`, `+0x224 |= 4`), as D08O3's
  E redraw and the number keys use in water, not the land Circle tap: D08O3
  found a Circle draw can end a thrust stroke and leave Duke stopped near
  geometry. `select_weapon` (swim handler entry `0x800455bc`) sends it while
  fire is held, holster and E are not, and `swim_fire_draw_ready()` holds:
  D08O3's settled swim test (swim states, equipment 0, no pending request,
  upper animation equal to body animation), no `+0x224 & 0x2cd` transition
  flags, and a drawable weapon.
- Weapon: the holstered weapon in `+0x3ba`. One the swim states refuse
  (table flag `0x04` clear) takes the original swim remap (1/2 -> 3, 8-11 -> 7,
  13 -> 12) if that weapon is owned and loaded; Boot (0) or no usable weapon:
  no draw, fire stays original.
- `pc_input.cpp`: while fire is held with equipment 0 and a drawable weapon in
  the swim states (`swim_fire_cross_masked()`), unarmed Cross is released so
  fire can never reach the surface climb-out (E remains the climb). The draw
  sets equipment 0 -> 2 directly (about 5 frames); equipment 1 (a thrown pipe
  bomb's detonator) keeps its Cross.
- Held fire reaches the drawn weapon (with D08O1's swim-fire thrust masking);
  a short click only draws; nothing is buffered after release.
- Counter `swim_fire_draws` in the `ttk_input` JSON. No new hook, guard,
  generated-code or framework edit; Vanilla unchanged (all behind the swim
  lease, which requires `input_modernized()`).

## Verification

Private lab `recomp/analysis/d08o5/` (ignored): offscreen OpenGL, dummy audio,
port 9591, copies of the D17AQ fixture cards (player cards and profile
hash-checked unchanged after every run), holster rebound to Z in the private
profile only. Slot 2, heading 0 into the moat.

- Third person 60 and first person 120: held fire underwater draws then fires
  (5 shots in 1.5 s); short click draws, 0 shots; stroke (W) and strafe (A) +
  fire keep anim 128 and fire; surface idle and surface swimming (125) draw
  and fire; mouse aim while firing; Ctrl dive + fire draws and fires.
- Weapons underwater: Desert Eagle, Combat Shotgun, Gatling Gun, Pipe Bomb.
  Boot selected on land, holstered, then water: the game itself substituted
  the Crossbow (3) on entry; fire drew and fired it.
- Regression: land holstered fire and jetpack (slot 10) holstered fire still
  draw and fire; Vanilla underwater holstered Cross stays holstered (0 shots).
- Native: `ttk-controls-test` with the `d08-camera-final` LEVEL00 fixture
  (new D08O5 group: underwater and surface request, holster/E refusal, 2 -> 3
  remap, Boot and unowned remap refused, unsettled, transition flags,
  equipment 1/2, Vanilla), `ttk-input-test` (Xvfb) and `ttk-aim-test` pass.

Build SHA-256:
`b6c4b40ad3a47d97e9de41c45c714068e79643017e7265f419849f7e18766ee3`.

## Limits

- No E climb-out ledge exists along the slot-2 moat, so the masked Cross at a
  ledge (holstered fire must not climb out) is covered by the native test and
  by reasoning, not a live ledge run.
- The remap from a refused weapon held in `+0x3ba` (for example after a save
  holstered with one) is native-tested only; live, the game substituted the
  Crossbow on water entry by itself.
- Only level 6 was driven; functional state/shot-counter checks, not feel,
  visual or 180 Hz measurements.

## Player check

Swim with a water-capable weapon (Desert Eagle, Shotgun, Gatling, Buffalo
Rifle, Crossbow, Pipe Bomb). Holster with Scroll Lock and let it finish.

1. Underwater, hold left click: draw, then fire. Release, holster, quick
   click: draw only.
2. Holster, swim (W/A/S/D, Space, Ctrl) and hold left click: Duke keeps
   swimming while the weapon comes out and fires.
3. Repeat on the surface, idle and swimming, in first and third person.
4. At a ledge, holstered left click draws and does not climb out; E still
   climbs out.
