# D07 — Guarded view aiming and crosshair

> Superseded behavior note (2026-09-27): D07C/D08H implementation and current
> coverage are in [feedback implementation](40-feedback-implementation.md).
> Earlier held-aim exclusions, weapon-family limits, and apartment non-reproduction
> below are historical evidence, not the current implementation.


Current close-range/temporary-holster follow-up: [D07B / D08 close combat](28-d08-close-combat.md). Earlier behavior below is historical.

Current follow-up: [D07 controls, assistance and timing](24-d07-controls-and-aim-options.md). The checkpoints below are historical; the follow-up replaces repeated SHA hashing with authenticated full live-word comparisons and adds independent aiming/display preferences.

D07 is Done on the user's explicit acceptance of this bounded first-map iteration,
including the known lack of body/gun alignment. D01–D06 remain Done. D07A tracks
view-aligned facing and weapon presentation as the next focused follow-up. This work does not certify all weapons,
states, acquisition/switching, maps or platforms. See the final evidence report
at [reports/d07-aiming.json](reports/d07-aiming.json).

## Player contract

Modernized schema 4 adds `controls.weapon_aim`: `view` (default) or `original`.
Settings choice 8 and `--weapon-aim` expose it. Migration backs up the exact old
bytes and preserves renderer, bindings, camera, sensitivity and inversion.
Tests use isolated preferences; the player's saved file is not migrated by tests.
Vanilla forces original aiming and never activates the weapon mutation hook.

Captured, drawn, supported normal first-map weapons show a white crosshair with
black edges in Software and OpenGL presentation. It is a view target, not a hit
confirmation. It disappears on release/holster/unsupported state or weapon.
The reticle is a host overlay: raw `screenshot_file` excludes it; use
`present_shot` to verify the composed window image. Vulkan is not advertised by
the launcher and has no new reticle path.

`view` replaces the *shot argument*, after original target selection, with a
muzzle-to-view-target direction. It does not write the saved game auto-aim
preference, actor heading or persistent auto-aim vector. `original` leaves all
weapon handling and the in-game auto-aim option in place. Original arm/facing
animation is not retargeted; this visual limitation is separate from shot travel.
Unknown states/maps and special weapons retain original behavior.

## Verified shot boundary and collision contract

Owned executable identity remains SLUS-00583 SHA-256
`b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a`.
The existing movement/camera lease rechecks code and the exact 9668-byte LEVEL00
payload at each shot. Additional weapon/query and table byte guards are in
`src/ttk/aim_guards.inc`; no generated C is hand-edited.

The starting pistol's live ammo 200→198 chain is:

- `0x8003C690`, RA `0x8004F784`, constructs the original origin.
- `0x8003C500(player, sp+0x40, player+0x144, weapon)`, RA `0x8004F7B4`,
  is the mutation boundary. The observed caller SP is `0x801FFE50`.
- `0x80071B88`, RA `0x8003C670`, dispatches weapon slot 4 to projectile type 0.
- `0x800718AC`, RA `0x80071E60`, constructs the projectile with a signed-word
  Q12 direction. Original speed is separate from this direction.
- `0x8006DD2C`, RA `0x8006EFC0`, sweeps its previous→proposed positions through
  `0x8006D980`, RA `0x8006DDA8`.
- `0x8006F1B8`, RA `0x8006FBEC`, receives the resolved wall-impact position.

Pistol bullets are **fast swept projectiles**, not proven instantaneous hitscan.
Observed segments span thousands of raw game units per simulation update.
The original integrator scales Q12 direction by its original speed/time step.
Player movement units must not be substituted for projectile speed.

The query takes `(ignored_actor, two padded s32 XYZ endpoints, room_seed,
room_output)` plus stack pointers to hit position and normal, and two query
flags. Return 0 is a miss, 1 a world hit, otherwise an actor pointer. A boundary
hit may return room -1. Initial experiments incorrectly rejected actor pointers
and room -1; those runs are retained as failures, not acceptance evidence.

At a supported shot:

1. Read the actual camera position/matrix, not the requested orbit. Query up to
   16384 raw units forward, selecting the first view obstruction/actor.
2. Use the physical muzzle `player+0xBC`, including its offset. The original
   precision-aim helper can place an origin along the camera ray; that relocated
   origin is not reused in this adapter.
3. Check body X/Z at muzzle height→muzzle. If obstructed, retract to 8 units
   inside the body side of the hit. Never move the origin forward through cover.
4. Normalize muzzle→resolved view target to Q12 and pass a dedicated enhancement
   vector to the original weapon dispatch. Keep the original muzzle argument,
   spread construction, swept collision, gravity/bounce, damage and ammo handling.

The query runs synchronously on a CPU/GTE copy and restores its 4 KiB temporary
stack region plus scratchpad. It uses the original world query, not a host
approximation of level geometry. Precise/lockstep or an active dispatch bailout
refuse the adapter. Forced-interpreter execution and scheduler variants remain
unverified. Query failure retains original shot behavior; it is not counted as a
modern shot. This fallback and reticle behavior under query failure still need
broader gameplay scrutiny.

## Weapon coverage boundary

Slot numbers below are executable weapon-table IDs, not keyboard shortcuts.
Do not infer a weapon's identity or acquisition from changing this byte.

| Slot / projectile type | Adapter | Evidence scope |
| --- | --- | --- |
| 4 / 0, starting pistol | View target, physical muzzle, original sweep | Real starting weapon; ammo, launch, flight and wall impact recorded |
| 5 / 1 | Same pre-spread aiming; original ten-pellet construction | Synthetic inventory/selection fixture; natural switching not accepted |
| 6 / 6; 7 / 7 | View launch direction; original projectile handling | Synthetic path coverage; natural weapon use remains required |
| 8 / 8 | View launch direction; original projectile handling | Synthetic path coverage; trajectory/damage playtest required |
| 11 / 13 | View launch direction; original projectile handling | Synthetic path coverage; bounce/owner collision behavior not certified |
| 12 / 4; 13 / 11 | Original, no modern crosshair in the final candidate | Experimental launch-only adaptation withdrawn: flight/impact and special-state chain not verified |
| 9 / 14; 10 / 15 | Original, no modern crosshair | Separate effect/beam paths; not certified as modern hitscan |
| 0–3, 14, upgraded/time-era variants, enemies | Original | No modern aiming coverage claim |

The beam branch `0x80073220` invokes target acquisition `0x800587C8` and stores a
selected target in projectile `+0x14`. Its updater `0x800733C8` can replace the
endpoint from that actor. Merely replacing its launch vector would falsely claim
to disable auto-aim. It is therefore explicitly excluded. The type-9 transient
rays seen in target acquisition are not evidence of a damaging hitscan weapon.
**No damaging instantaneous hitscan/beam path is accepted as modernized.** This
is an implementation/coverage gap, not a playtest pass.

Inventory fixtures modify only the isolated process's inventory/selection fields,
log each write, never save, and retain code/overlay guards. Direct selection does
not load the correct weapon model or establish normal firing animations. Two
attempts to cycle to the granted next slot through H+Right failed their selection
assertion; both are retained. Do not describe these as successful weapon-switch
or acquisition tests. No test writes player memory-card files.

## Verification and remaining acceptance

The native adapter harness checks Vanilla/original no writes, unsupported weapon,
actor and code rejection, unsupported precise mode, camera/muzzle parallax,
CPU/GTE isolation and a thin-cover fixture. The fixture verifies that an offset
muzzle at X=100 behind a plane at X=50 retracts to X=42. It is not retail-map
geometry evidence. Existing native movement/camera and SDL input checks still
run separately.

A reviewed early pistol wall hit was `(6140,-9563,1023)` against a resolved
view target `(6140,-9561,1023)`: 2 raw units error after original swept travel.
This is bounded evidence of Q12/step rounding, not a universal error tolerance.
Moving actors can leave the selected point before a projectile arrives; pellets,
gravity and bounce retain their original deviations. There is no target leading.

Remaining coverage still includes natural use of every enabled family, damaging
beam/hitscan integration, close-wall and camera-around-corner cases, high/low
pitch, moving/firing, original precision aim, holster/draw and progression
transitions, original-auto-aim on/off comparisons, desktop rendering and full
Vanilla route playtesting. Automated replays and image review are labelled
separately in the report. D07 must not be marked Done from a linked executable,
fixture success, or the single pistol impact.

## Reproduce

```sh
python3 -m unittest discover -s recomp/tests/local -v
cmake --build recomp/build-local --target psx-runtime ttk-aim-test ttk-controls-test ttk-input-test --parallel 4
recomp/build-local/ttk-aim-test recomp/disc/SLUS_005.83
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls weapons
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls weapons --renderer opengl --near-cover
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls weapons --synthetic-weapons
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls weapons --weapon-aim original
```

Each driver refuses an active game or occupied debug port and owns its process,
private X display, cards and preferences. `--synthetic-weapons` is explicitly
mutating test setup; `--weapon-fixtures` attempts original selection after a
logged inventory grant. Both are excluded from normal acquisition acceptance.
Raw owned-data RAM, images and logs remain under ignored `recomp/analysis`.

## Final-candidate review notes

The final OpenGL approach stops Duke at approximately `(5726,-9721,1418)`.
Its two close-wall impacts are 1 and 1.4 raw units from their view targets;
reviewed presentation shows the crosshair and wall effects. The earlier
long-distance static-wall impact on this candidate is 3.6 units from target.
The first camera query selected an actor, but that projectile subsequently hit a
farther wall (about 1255 units from the selected point). Its path remained within
0.8 units of the launch line. The exact reason it did not hit that actor is not
established; moving-target/actor-hit acceptance is still open.

On the preceding candidate (before narrowing the supported weapon set), explicit
`--weapon-aim original` consumed pistol ammo 200→198 with zero modern
queries/shots and no modern reticle. The final Vanilla regression completed;
reviewed firing, jumping, inventory, movement and turning match the original
route. This preserves D01 confidence without claiming a new full-campaign test.

A Software pellet-fire presented frame lacks the crosshair even though its
preceding sampled state reported reticle eligibility. Samples and presentation
span different frames. Visibility through firing/animation transitions is an
additional required check, not a claimed pass. Other reviewed Software and
OpenGL captures show the crosshair, and release captures show it removed.

Prefer short synthetic runs so death cannot contaminate later samples:

```sh
python3 recomp/tools/local/pc_input_probe.py --name unique_a --controls weapons --synthetic-weapons --weapon-slots 5,6,7,8
python3 recomp/tools/local/pc_input_probe.py --name unique_b --controls weapons --synthetic-weapons --weapon-slots 9,10,11,12
python3 recomp/tools/local/pc_input_probe.py --name unique_c --controls weapons --synthetic-weapons --weapon-slots 13,14
```

Use fresh names for each run. The long
`d07-final-families` run's terminal death remains a failed route even though its
preceding live samples are retained. Short-run results and their individual
binary identities are in the report; earlier family sweeps predate coverage tightening. Type 13's original collision routine briefly
clears its owner pointer; the owner-filtered flight observer therefore does not
establish its entire flight. Types 4/11 need broader trajectory/impact
coverage and were removed from the enabled set after review. Do not infer flight validation from a constructor log alone.

The final coverage tightening keeps slots 12/13 original. Earlier experimental
runs that changed their launch vector are retained as research only, not shipped
acceptance. Both the long sweep and short 13/14 sweep ended in death after live
samples; no exact cause is asserted. The remaining enabled set is 4–8 and 11.

Final executable SHA-256: `0ac41da3915c2ecd49dcd54caf15ec9f1424665699e9eab28f5b339797acd1ca`.
The final fallback replay confirms slots 9, 10 and 12 consume ammo without modern
queries/shots or a modern reticle, while slot 11 receives the view launch argument.
This is synthetic dispatch evidence, not natural weapon acquisition acceptance.

Final verification: 52 Python tests discovered, 50 passed and 2 skipped; native
aim, input and movement/camera harnesses passed. Final close-wall OpenGL,
synthetic fallback and Vanilla replays exited 0 on the recorded final binary.
Vanilla firing/jump/inventory/movement/turn screenshots were reviewed. Its debug
quit response reported busy/frozen before the owned process exited 0; this is
not a graceful-quit or complete campaign/save/audio acceptance claim.

## D07A follow-up — 2026-09-26

[View-facing and presentation](22-view-facing.md) adds body yaw and the original
upper-body direction-argument adapter. It also corrects a D07 research error:
player+0x224 bit 2 is run/walk state, not weapon-drawn state. New reticle eligibility
uses the equipment byte at +0x3B8, with Mouse2 precision-aim fallback. This is a
specific ownership correction; the previously recorded pellet reticle discrepancy
has no newly proven cause, and broad firing/animation continuity remains open.
All weapon, actor-hit, cover and campaign coverage limits above remain in force.
