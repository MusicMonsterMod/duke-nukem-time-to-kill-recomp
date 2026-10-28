# D14B - Weapon and arm projection independent of world FOV

2026-10-28. Done, user-accepted; implementation/documentation closeout authorized.
Evidence labels follow the accepted [D36 audit](257-d36-first-person-presentation-audit.md):
S = source, L = private runtime observation, A = prior acceptance, U = user report.

## Diagnosis and bounded correction

U: weapons and arms become undersized and oddly positioned after increasing
world FOV. Reproduction requested current UI slot 4, `dnmonsters`, `dnstuff`,
`fov 90`, then weapon cycling.

L: reproduced in a private copy of current slot 4, the LEVEL00 street facing
Bootylicious. The source save SHA-256 was
`df0a976a2c8503389877880ff8427d925c6109e48ef477560c8a0bbf7ff5eeab`.
Slot numbers are reproduction pointers, not permanent scene identities.

S: `prepare_display_matrix` and `display_pose` place the accepted hand and
connected arm chains independently of world H. Existing slot scales, tilts,
grip offsets, reload lift and recoil also do not depend on H. However,
`near_clip.cpp:object` projected those meshes through the shared world H.
At FOV 90, H=160 instead of 256: screen displacement from the projection centre
and apparent linear dimensions become 160/256 = 62.5% of the accepted baseline.
This explains both shrinking and inward/upward movement. It also exposes arm
segments previously below the screen, changing the silhouette without changing
any mesh or grip. At FOV 60, H=277 enlarged that same presentation by about 8.2%.

L: original and wide-FOV captures show this effect across the weapon families.
The corrected Gatling, pistol, shotgun, rifle and other sampled weapons retain
the original framing as the world widens. No FOV-dependent per-weapon placement
adjustment was justified. The RPG's broad twin-barrel framing is present in the
accepted original-FOV baseline; this work preserves it. Existing individual
weapon poses and firing-hand motion are not redesigned or newly certified as
ideal by this projection correction.

## Implementation and ownership

`limb_presentation.h` identifies fixed projection only for `DisplayHand` and
`DisplayArm`. After reading the current GTE into a draw-local `NearGte`, the
object renderer uses H=256 for those owners. This precedes vertex projection,
clipping, screen coordinates and depth-plane calculations. The original-FOV
case uses exactly the same H as before.

- S: semantic part mapping supplies the owner; costume joint order, texture
  page, mesh address and weapon record are not used as new identification rules.
- S: weapon attachment `0x80033e40` from `0x8003531c`, held item `0x80033f5c`
  from `0x80035250`, and flash `0x800341e4` from `0x80035348` already inherit
  the verified display hand. Their render paths reach object renderer
  `0x80010000`. Flash transform caller `0x8003437c` retains that ownership.
  Owned-executable disassembly confirms `0x80033dd8` prepares vertex scratch;
  it is not an additional projection-dependent weapon culling stage.
- S: the held keycard/crystal/remote uses fixed projection with its display
  hand. On transition to native Use/Push/Ladder/etc., the Interaction owner
  retains world projection, world depth and contact poses. World projectiles,
  smoke and other independent effects retain their original ownership.
- S: native body/legs, holstered locomotion arms, jetpack's unarmed arms,
  interaction limbs and quick kick retain world projection. A shared viewmodel
  depth layer alone is deliberately insufficient to select fixed projection.
- S: weapon depth, near plane, ordering, UVs, lighting, native transparency and
  display matrices are retained. This does not introduce wall retraction or
  change the existing overlay-versus-world clipping policy.
- S: no gameplay matrix, physical muzzle query, centre aiming ray, projectile,
  collision, animation event, source mesh/material or per-weapon offset changes.
  Modernized/identity/view gates and third-person/Vanilla exclusions remain.
- S: the GTE itself is never changed. No new guest allocation, host latch, save
  format or replay state is introduced. Ownership is reconstructed per draw in
  each worker and cleared by the existing joint/draw/load boundaries. Framework
  and UI submodules, their pins and the accepted framework patch are untouched.

This is a separate projection within the existing mesh renderer, not a second
framebuffer or an image resize. It preserves the accepted aspect convention:
widescreen adds columns while weapon scale relative to image height stays stable.

## Verification

Build and all four native suites pass: controls with LEVEL00/LEVEL01 and all
owned level fixtures, SDL input under Xvfb, aiming, and owned-EXE near clipping.
New packet tests exercise DisplayHand, DisplayArm, Interaction, Body and Kick
at H=160/177/209/256/277 with margins 0/85/192. Display output coordinates match
the H=256 baseline exactly; other owners still respond to H. UVs and depth
metadata are preserved, and the complete shared GTE register array is unchanged.
Existing ownership, attachment, replay, load, parallax, muzzle-obstruction,
lighting, clipping, Vanilla and code/actor/caller guards also pass.

L: captures and scripts are local under ignored `recomp/analysis/d14b/`:

- `baseline2` and `candidate`: 15 verified selections, covering Boot and all
  14 base weapon records, including knife/axe, all three thrown explosives and
  crossbow/rifle. Candidate idle comparisons use default/60/90 at 16:9.
- `upgrades43`: the same selections at 4:3 and default/60/90. All six campaign
  upgrade flags are 9 and the upgrade mask is 63. Held base records are normal
  for this campaign path; they must not be mistaken for failed upgrades.
- `actions60`: 105 sparse samples at 90, covering selection, attack/release,
  recovery, crouch and look changes. A label records the requested action, not
  proof that every sample caught that phase. Short fire intervals missed some
  flashes and motivated the separate firing pass.
- `firing60`: longer firing/recovery samples for pistol, shotgun, Gatling, RPG,
  flamethrower and rifle. Captures show pistol/Gatling muzzle flashes, flame
  effects and rifle reload lift; the native flash counter advances. This is
  sampled visual evidence, not every frame of every weapon animation.
- `contacts60`: default/60/90 in D36 private fixtures. Wheel 270, vending 277,
  ladder 186, held crystal/item 11 and keycard/item 6 with upper 47, keycard use
  266 and weapon returns are observed. Dumpster samples reach pull 119 and
  hold 121; the intended forward-push sample remained in ground state and is
  not counted as push evidence. Real P keys verify third-person H=386 and
  return to first-person H=160; Escape pause/resume retains the selected FOV.
- `scenes60`: eight current copied saves at default/90, including both observed
  19-joint orderings. This is cross-save/ownership evidence, not every costume
  with every weapon. Dark and near-wall saved scenes are included.
- `motion60`: corrected geometry/textures at default/90, maximum downward
  pitch (1.047198 radians), native body, active Q kick and return, crouch 178,
  movement toward the opposite wall, and armed jetpack 164. The synthetic
  holster and Gatling-switch requests did not take effect, so their labels do
  not establish unarmed or Gatling flight coverage. No packet-budget failures.
- `replay120`: five verified upgraded weapon selections at 21:9, default/60/90.
  1,606 replays, zero failed replay sessions/jobs. Switching motion remains in
  some captures, so those are not pose-identical comparisons. The earlier
  `upgrades120` timing-based harness missed selections under heavy readback and
  ended with a connection reset; it is excluded from weapon coverage. The
  corrected run disables surface readback checks. No sustained-120-FPS claim.

Each completed harness verifies SHA-256 and mtime preservation for player cards,
saves and profile. Tests use separate writable copies, private settings and
port 9502. Captures are sequential observations, not atomic RAM/image snapshots.
The first baseline harness also missed repeated actions because its synthetic
command token was not reset; `baseline2` fixes that and verifies actual records.

Candidate executable SHA-256:
`bc1329ece5c94fa033149d96527aa9c2e84525add340681b931fb8eb20b86f53`.
Baseline: accepted D14A executable
`45148c23fde7ead1a6084c26a61f7d121ff0d773f552833a6d94e6333de6ee6f`.

## Review limits and playtest

User visual acceptance is recorded below. Full continuous-motion coverage, direct challenge
records 27-29, every costume/weapon/lighting/effect combination, full swimming
coverage, all wall/portal angles, software renderer and adaptive resizing are
not certified by these samples. Existing native ownership/aim tests and prior
accepted work remain complementary evidence. No claim of full campaign fidelity
or a new performance benchmark is made. The correction retains existing native
world contact and viewmodel depth behavior rather than solving unrelated poses.

From the workspace root:

```sh
python3 recomp/tools/local/run.py
```

In first person, compare `fov default`, `fov 60`, `fov 75` and `fov 90`; cycle
weapons, fire, crouch and look up/down. The world should change while weapon
size and framing remain anchored to the accepted default. Check switching,
reloads, contact interactions and nearby walls in motion. FOV remains session
only; D19X is separate. No acceptance, documentation closeout, commit or push
is implied by this candidate.


## Acceptance and closeout

The user confirms "arms is now really solid. great work" and explicitly requests
lock-in and closeout on 2026-10-28. D14B is Done. The evidence limits above remain
separate from user acceptance. D14C is a new Todo investigation of the reported
leg-view alteration; its brief intake does not reopen D14B or select leg work.
No additional gameplay edits, build or launch were made during closeout.
Prior build/native-suite results stand. Repository/privacy checks pass, and
implementation, tests, manual and documentation commit/push are authorized
as MusicMonsterMod.

Closeout verification: exporting the existing framework delta leaves the accepted
patch byte-identical. It applies cleanly to the pinned dependency and matches
all 55 modified/new framework files. No framework edit or pin change is included.
