# D08O2A - natural swim-fire pose (v2)

2026-10-07. **Accepted** (user: "this is actually rock solid ... I'm happy with it and its 100% playable"). The final-shot-down issue from the user's capture is fixed (see below). User request (on accepting D08O2 v1): "a more
natural pose rather than his entire torso standing up, so it will involve
moving the arms in the position as if firing up and his head looking up."
During the work the user added: "while duke is swimming, his head is always
facing in the right direction anyway i think, looking ahead, so probably not
much if anything to do there at all." The head is therefore left on the
original stroke animation.

v2 replaces the v1 mechanism ([note 118](118-d08o2-swim-weapon-forward.md));
the v1 chest aim no longer exists in the build.

## Model build facts

Duke's model (`[player+0x40]`) has 19 joint records of 0x28 bytes from
`model+0x44`; byte 2 of a record is its depth, and a record's parent is the
most recent record one level up (scratchpad matrix `0x1f800000 + depth*0x20`,
parent at `depth*0x20 - 0x20`). Measured in the private lab:

| Record | Depth | Part |
| --- | --- | --- |
| 0 | 1 | pelvis (root) |
| 1 | 2 | chest (`[model+0x2d]`) |
| 2, 3, 4 | 3, 4, 5 | left shoulder, forearm, hand |
| 5, 6, 7 | 3, 4, 5 | right shoulder, forearm, gun hand (`[model+0x31]` = 5) |
| 8 | 3 | chest attachment (`[model+0x38]`, the jetpack joint) |
| 9 | 3 | head (`[model+0x34]`) |
| 10-13, 14-17 | 2-5 | legs |
| 18 | 3 | attachment on the second leg chain |

- `0x80097a44(m, dir)` writes a world look-at matrix: columns
  `r = normalize(dir.z, 0, -dir.x)`, `u = normalize(dir x r)`, `dir`.
  `0x800978f8` is the same with the direction as the bone's Y axis (the
  original one-arm aim of record 5 for upper animations 8/11 in modes 3/6/7).
- `0x800b42ec(P, L, out)` composes a record's rotation with its parent's.
  The plain build `0x800987cc` calls it for records 1-18 at ra
  `0x8009899c` (record in s0); the aim build `0x80097c04` at ra
  `0x800980d0` (record in s1). s5 is the actor.
- The update picks `0x80097c04` for `+0x224 & 0x200100` and also for
  `+0x2b4` / `+0x2b8` / `+0x358` state, so a swimming Duke is built by
  either routine (the first lab trace saw only `0x80097c04`).

## Change (Modernized only)

- `swim.inc` `swim_aim_begin` (entry `0x800411b8`, ra `0x80042634`) keeps the
  v1 conditions (underwater under the swim lease, weapon out, view aim, no
  original `0x200100` bit, fire held or the shot's upper animation still
  playing) but now only arms a host flag. No game memory is written, so the
  v1 contact-callback skip is gone.
- `swim_aim_arm` (new entry hook `0x800b42ec`, its own lightweight
  callback): for Duke's records 2 and 5 at either build call site, the
  shoulder's local rotation `L` becomes `P^T * A * L`, where `P` is the
  chest's world matrix and `A` the `0x80097a44` look-at of the camera
  forward (the direction D07A supplies). The shoulder's world orientation is
  `A * L`, which is exactly the arm pose of floating fire (where the original
  sets the chest to `A`), while its position stays on the stroking chest.
  Forearms, hands and the weapon follow through the original chain. The
  chest, head, pelvis and legs are untouched.
- The flag clears after record 5, and at the swim handler `0x800455bc`,
  `0x80055e80`, `0x800493a4`, the camera update `0x8003ade4` and the next
  `0x800411b8`.
- Shape checks before writing: 19 records, the chest at depth 2, the
  shoulder at depth 3, parent and output scratchpad matrices at
  `0x1f800040` / `0x1f800060`.
- Guards added: `0x800987cc` (672 bytes), `0x800b42ec` (352). `0x80097c04`,
  `0x800411b8` and `0x80042634` were already guarded. Codegen hash unchanged
  (existing savestates load).
- **Single shots (fix after the first playtest, 2026-10-07).** User: "doing
  one single shot while swimming forward will result in duke shooting
  downards. holding shoot then shows him shooting forwards as expected." A
  shot plays raise, fire and lower upper animations (Desert Eagle `9`, `10`,
  `9`; shotgun `24`-`26`; Gatling `33`/`34`; crossbow `24`, `25`, `27`)
  before the stroke's own upper animation (`5`, `20`, `29`) returns. The
  release rule kept aiming only while the animation seen at the press was
  playing, so after a tap the change `10` -> `9` dropped the arms (gun hand
  230 -> 47) during the lowering. Now, after release, the arms stay aimed
  until the upper animation is back to the one from before the press, capped
  at 90 updates. Lab tap trace (fire held 3 or 6 frames, swimming W): the
  hand stays forward from the press until the stroke animation returns for
  all four weapons (Desert Eagle 198-237 through the lowering).
- **Last shot pointing down when surfacing (follow-up after acceptance,
  2026-10-07, accepted: "another rock solid milestone").** User capture (0:16-0:17): as a burst ended,
  one final shot fired with the gun pointing down. Duke reached the surface
  (mode 4, anim 125) while the shot's upper animations were still playing;
  the arm aim only ran underwater, and the original aims at the surface
  only while fire is held, so for those frames the arms dropped (hand -19 in
  the lab). The release tail now continues into the surface states. Shot
  adaptation was never involved: every lab shot was adapted (`rejected` 0).
- **Release tail rule (same follow-up).** Remembering the pre-press upper
  animation failed when fire was pressed while floating (ready pose 8) and
  the stroke then rested on 5: the arms stayed aimed until the 90-update cap
  (seen intermittently in the lab). The tail now uses the original's upper
  animation flags (`0x800c2824`, already guarded): it continues while
  `flags & 0x804 == 0x800` (raise / fire / lower / reload, all `0x809`) and
  ends at a rest pose (`0x200c`: 5, 20, 29) or a ready pose (`0x80c`: 8, 14,
  23, 32). Lab: taps keep the arms up through the lowering for four
  weapons; surfacing with the lowering keeps them up until the stroke; no
  stuck tail in repeated runs. Build
  `61334eeba81e450a95aee49c13c686dbcd91f05b1f909b5f8f0ba1283d074271`.
- Debug counters (`ttk_input` controls): `swim_aims` (builds armed),
  `swim_aim_restores`, `swim_arm_aims` (shoulders changed; two per build).
  `swim_aim_contacts` was removed with the v1 contact skip.
- Vanilla: `swim_aim_begin` is reached only in Modernized, and the arm hook
  acts only on that flag.

## Verification

Private lab `recomp/analysis/d08o2a-20261007/` (ignored): Xvfb `:96`, debug
port 9353, copies of the D08O2 private profile, cards and savestates (level 6
FAMILY JEWELS, private slot 11), audio on a null sink. The lab profile runs
at 120 fps with fast CPU timing. No player card, save or preference was used.

Gun hand along the view (max of joints 4/7 relative to the head, average of
8 samples) and the chest's Y axis projected on the view (`-1` is the
swimming body laid along the view, `0` upright as when floating):

| Weapon | Floating fire | W | A | D | Ctrl | W, no fire |
| --- | --- | --- | --- | --- | --- | --- |
| Desert Eagle | 245 | 228 | 230 | 229 | 254 | 92 |
| Combat Shotgun | 201 | 199 | 192 | 193 | 225 | -95 |
| Gatling Gun | 100 | 110 | 86 | 91 | 153 | -112 |
| Pipe Bomb (fresh load each) | 157 | 152 | 145 | 152 | 174 | 26 |
| Buffalo Rifle | 178 | 166 | 167 | 168 | 195 | -102 |
| Crossbow | 164 | 148 | 153 | 159 | 177 | -97 |

- Chest axis while swimming and firing matches swimming without firing
  (about -0.95 to -1.0 for W/A/D; the dive keeps its own pitch), so the
  torso stays in the stroke. The pipe bomb's throw animation moves the chest
  itself (original data).
- `swim_arm_aims == 2 * swim_aims` in every run; `swim_aims ==
  swim_aim_restores`. Duke keeps swimming (1500-1730 units per run) and
  shots stay adapted.
- Floating fire: no arming (the original's own `0x100` path). Surface W +
  fire: no arming, hand 242 (original). Vanilla underwater Up + Cross still
  stops Duke, no arming. Modernized ground W + fire moves with adapted shots,
  no arming.
- Suites: ttk-controls-test (LEVEL00 fixture, LEVEL01, all levels),
  ttk-aim-test, ttk-input-test, ttk-near-test, Python unittest (121,
  2 skipped) pass. Runtime patch export unchanged (no framework edits).

Build SHA-256:
`47602e8c000ce449c99485b7706bee6f9f1caba79e67450a4cdb4dbb2e507dae`
(with the single-shot fix; held fire, Vanilla and the suites re-checked).

## Limits

- The chase camera sees Duke from behind, so the lab screenshots show the
  weapon moving from below the body to ahead of the head but cannot judge
  the pose from the side; the look is the user's playtest.
- Arm orientation equals the floating-fire pose in world space. With the
  body laid flat, the arms reach forward past the head; any clipping with
  the head or the shoulders at steep up/down aim is unverified.
- The jetpack takeoff and the holster key did not register under Xvfb (as in
  v1); neither path can arm by construction (not underwater / `+0x3b8 != 2`).
- Only level 6 was driven. The user's UI slot 2 (file 01, first medieval
  level) loads with Duke on land; the swim there is the user's playtest.
