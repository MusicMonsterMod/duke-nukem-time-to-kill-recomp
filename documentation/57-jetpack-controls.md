# D08Q — Modern jetpack flight controls

**Done - 2026-09-29 (user-accepted):** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."

## Report

"When using the jetpack I cannot control it at all aside from spacebar to
ascend. WASD are blocked, modern controls fail here completely."

## Baseline (binary `79a3fd5b…`, isolated probe)

Isolated copy of the level-1 states in `recomp/analysis/d08q-jetpack/`
(gitignored; `cards/openbios/state_800AB6FC_slot01.pst` turret room,
`slot02.pst` ledge). Jetpack via the typed cheat `dninventory`, J to switch
on, Space to lift.

| Observation | Cause |
| --- | --- |
| Mouse dead in the air | Original mode 10 / anims 163–170 matched no lease clause in `state()`, so no camera-only lease; `lease_refusal_reason` = state. |
| "WASD blocked" | Without a lease `input_pad()` used the tank fallback (W/S → Up/Down, A/D → L2/R2). The original thrust is **body**-relative and the body heading could not be turned; the original's idle gravity (bit 31 of `+0x224` clear without input) sank Duke to a landing (anim 76) within 1–2 s of the Space release. |
| Ctrl inert | No original meaning in flight (pad 0). |
| Space works | Bound to Square, which the original handler reads directly. |

## Original flight (SLUS-00583, read-only)

| Address | Role |
| --- | --- |
| `8004aaf8` | Called from the per-update pre-state step (`800c2754[3]`, `80055e80`). Square word `800d1b88` bit 0, `!(+0x224 & 0x241)`, `(+0x358 & 3) == 3` (jetpack owned + on), fuel `+0x35a ≥ dt (800d21fc)`, not submerged, mode ≠ 10, anim not 105/106 → anim **163**, mode **10**. |
| `8004ade0` | Mode-10 handler (`8004b594` dispatch, case 10). Ends flight (anim **108**, `8003ff6c(player,1)`) when `(+0x358 & 3) != 3` or fuel < dt. `80066e84` rotates thrust `+0x1e4/+0x1e8/+0x1ec` into body space by `+0x1c`. Hover button word `800d1cf0` `(w & 3) == 1` toggles `+0x224 ^= 0x08000000`, base `+0x860 = Y`, phase `+0x864 = 0`. Square held clears the hover lock and lifts (`+0x1e8 -=`); Up/Down `+0x1ec ±=`, L2/R2 `+0x1e4 ±=` (half gain); any direction adds a small lift (`+0x1e8 -= 29` per update in the probe). Any input or the hover lock sets bit 31 (no gravity this update), else gravity. `80066e34` rotates back; lean smoothing to `+0x87c/+0x87e/+0x880`. Hover lock: `+0x1f8 = +0x1e8 = 0`. `8003ea58`: `bv += v·dt`, root `+0xfc/+0xfe/+0x100 = (bv·dt + v·dt/2) >> 10` (dt ≈ 17: `+0x1f8 = 7000` moves ~105 units). Hover pin: `+0xfe = +0x860 + bob(+0x864) − Y` (16-bit). Thrust or hover → `s2 = 1` → `+0x224 |= 0x02000000` and fuel `−= dt` (unless `800c3cc4`). `8004ac08` then `80049638`. |
| `8004ac08` | Root apply. Horizontal probe `8007a98c` with `+0xfe` zeroed; result 1 with `+0x1c0` inside ±0x201 / ±0x400 → `8003ef78` floor approach (pulls toward ~0x200 above a nearby floor). Second probe with the full root: 0/1 → `Y += +0xfe`; 2 → water (`8007a048`/`80054c04`); 3 → landing `8003efd0`. |
| `800402d0` / `8003ff6c` | Inventory item apply: `+0x358` bit 1 set → validate (fuel, not submerged, mode not 4/5) and open (`+0x224 |= 0x1000`); clear → `8003ff6c` closing (`0x10000000`). `use_item` toggles bit 1 and calls `800402d0`; the handler then cuts out to the 108 fall. |

Pad word pointer tables (`+0x233` index): Up `800d1ce8`, Down `800d14f8`,
Left `800d14e8`, Right `800d1a98`, L2 `800d1a68`, R2 `800d1b80`, Square
`800d1b88`, Cross `800d1b50`, L1 `800d1b58`, hover toggle `800d1cf0`.

## Modernized layer (`recomp/src/ttk/jetpack.inc`)

Runs from the existing `8005a210` player-update hook (after
`swim_update`), before the pre-state step and the handler read pads,
heading and flags. No new generated hook; no change to Vanilla (every write
is behind Modernized + capture + lease + identity).

| Input | Host | Original |
| --- | --- | --- |
| Mouse | camera-only lease (`state(camera_only)` `jet` clause, like swimming); `face_view()` sets the body heading to the view (not while Original Aim is held) | thrust rotates by `+0x1c`, so it becomes camera-relative |
| W / S | `input_pad()`: Up / Down (16 / 64) | `+0x1ec ±=` |
| A / D | layout strafe pads via `swim_strafe_pads` (L2 / R2) | `+0x1e4 ∓=` (half gain) |
| WASD held | hover lock released; `+0x1f8 = 0`, `+0x1e8 = 18` (trim against the directional lift) | bit 31 set → no gravity → level flight, ballistic build-up to the original speed clamp |
| Nothing | `+0x224 |= 0x08000000`, `+0x860 = Y`, `+0x864 = 0` once | hover pin; fuel still drains (`s2 = 1`) |
| Space | passes as Square | lift, clears the hover lock |
| Ctrl | hover lock released; `+0x1f8 = 2800`, `+0x1e8 = 0` every update | ~48 units/update down; floor probe result 3 lands (anim 105) |
| J | `select_weapon` allows only the jetpack action while `jetpack_input_ready()` → `use_item(1)` | bit 1 cleared → handler cuts out → anim 108 fall |
| 108 fall | `jet_fall_grace` keeps the camera-only lease while anim 108 in mode 9/10 (set on the cut-out update, cleared on landing / other state / new capture epoch) | mode 9 fall, then 105 landing, then the land lease |

`locomotion_input_ready()` and `airborne_input_ready()` are false during
flight and the grace, so the ground forward/walk injections, step-drop and
short-fall paths stay out. `jetpack_input_ready()` is exported for
`input_pad()` and the test stub. Debug JSON: `jet`, `jet_updates`,
`jet_hovers`, `jet_descents`. `DNTTK_JET_TRACE=1` prints a per-update line.

Guards added (`control_guards.inc`): `8004aaf8` (272), `8004ac08` (472),
`8004ade0` (1972), digests from `disc/SLUS_005.83`.

### Rejected on evidence

* **Driving the hover base `+0x860` for descent.** The pin only holds small
  offsets; with the base 69+ below Duke the root was not applied at all
  (Y drifted 2/update), and re-anchoring `base = Y + k` produced the same
  ~18 %-of-floor-distance sink for k = 1, 3, 6, 20 — that is `8003ef78`'s
  floor approach, not the pin.
* **Hovering while flying WASD.** Under the lock the handler moves Duke by
  the raw thrust vector (~10 units/update, 3 units/frame) instead of
  integrating it into the ballistic velocity (25–30 units/frame).
* **Trim 29** (the full directional lift): flight sank ~5 units/update.
  18 is level within ±25 units over 55 frames.

## Evidence (binary `481dd2ec…`; behaviour re-probed unchanged on `81a4a909…` after the D23A frame-budget fix)

`/tmp` harness: Xvfb + xdotool real keys/mouse, isolated cards, debug
port, `savestate load` slot 1 (turret room) / 2 (ledge).

| Check | Result |
| --- | --- |
| Hover after Space release, 60 frames | dy −19 / −32 (slot 1), −183 (slot 2, near a floor above) |
| W / S / A / D heading vs camera −30° | −33 / 154 / −120 / 61 (expected −30 / 150 / −120 / 60) |
| Level flight over 55 frames | dy 24 / −23 / 11 / 18 |
| Speed, units per frame | 25.1 / 28.7 / 23.7 / 13.9 (R2 side slower in the original too) |
| Mouse 300 px | body 3758 → 66, camera −0.518 → 0.111 rad, `looks` 0 → 1 |
| W after the turn | heading 14° vs camera 6° |
| Ctrl 30 frames | dy 489, still mode 10; from 500 above the floor: 234, 240 then anim 105 |
| Space 30 frames | dy −540 … −852 (stopped by the room ceiling in one run) |
| J in the air | anim 108 mode 9, mouse `looks` 0 → 1 during the fall, 105, then 63 with `ready` and ground run 78 at 45.7 |
| Tank fallback messages | none |

ttk-input-test (new jetpack pad case), ttk-aim-test and ttk-controls-test
PASS. The controls test needs an **original** overlay fixture
(`analysis/pc-input/d08-camera-final/level00-guard-fixture.bin`); the
`d08p-turret` fixture was captured Modernized-patched and fails its Vanilla
assertion.

## Remaining

Accepted by the user; possible later polish: stop-on-release is immediate; after Ctrl release Duke settles
another 130–300 units (floor approach) before holding; strafe is the
original half gain; fuel drains while hovering. Not probed: low ceilings,
water below, damage/death in flight, fuel-out mid-flight (same handler path
as J), controller.

# D08R - Selectable jetpack scheme: Modern / Classic

**Done (2026-09-29, user: "it's absolutely rock solid"; revision 2).** A persisted Modernized control
`jetpack` = `modern` (default, D08Q unchanged) | `classic`. CLI only:
`run.py --jetpack classic|modern` (append `--show-settings` to save without
launching). Profile schema 11 (v10 and older migrate with a
`.recovered-*` backup, adding `modern`). `run.py` exports `DNTTK_JETPACK`
(`modern` whenever Vanilla is active); the runtime reads it once, so a
switch needs only a relaunch. In-game menu exposure belongs to D19.

## Scheme definition (user direction)

Classic keeps **all modern controls** - mouse look with Duke facing the view,
camera-relative WASD, J, the cut-out fall with a live camera - and changes only
the **flight physics** to the original burst style: Space lifts (original
Square), the original gravity pulls Duke down whenever he is not thrusting,
no host hover on release, no Ctrl descent, no level trim. The original hover
toggle (physical L1 = Modernized walk, Shift) still reaches the game.

Revision 1 (same day) ran Classic with the original camera and D-pad turning
(A/D -> Left/Right) and no camera lease. User: "having trouble turning with
the mouse ... the controls are slipping in and out of our non-modern
controls". Rejected; revision 2 shares the Modern lease.

## Runtime

| Piece | Behaviour |
| --- | --- |
| `jetpack_classic()` (`modern_controls.cpp`) | `DNTTK_JETPACK == "classic"`, cached. |
| `state()`, `jetpack_input_ready()`, `input_pad()`, `shortcuts.inc` | Identical in both schemes: camera-only `jet` lease, W/S -> Up/Down, A/D -> layout strafe pads, J in flight. |
| `jetpack_update()` | Both: fall grace and `face_view()`. Classic then returns before the host vertical layer (hover engage, Ctrl descent `+0x1f8 = 2800`, WASD level trim) and counts `jet_classic_updates`. |
| Debug JSON | `jet_scheme`, `jet_classic`, `jet_classic_updates`. |

Vanilla: every path is behind `input_modernized()`; `input_pad()` returns
early outside Modernized.

## Research kept from revision 1 (read-only)

Original flight turns with D-pad Left/Right (live probe; yaw 3762 -> 3423
over 30 frames) although `8004ade0` never reads them; turning is not thrust
for the no-gravity bit 31. Pad tables (layout 0) index the button-word array
at `800d1450` in PS bit order from Start: Up 1, Right 2, Down 3, Left 4,
L2 5, R2 6, hover toggle `800d1cf0` -> 7 = **L1**, `800d1b58` -> 9 =
Triangle, Cross 11, Square 12.

## Evidence (binary `1a8907148aad...`)

Isolated Xvfb instance, scratch copy of the D08Q level-1 cards (slot 1),
`dninventory`, real keys and mouse; no player cards; the user's game closed.

| Check | Classic |
| --- | --- |
| Lease in flight | `jet` true on every mode-10 sample (0 losses), 197 updates; no tank-fallback / ORIGINAL MOVEMENT messages |
| Mouse in the air | body 3759 -> 25, camera -0.518 -> 0.111 rad, mode 10 |
| W / D | dir -32 / 64 vs camera -30 (expected -30 / 60) |
| W + mouse | body 313 deg, camera 312 deg |
| Release 30 frames | sinks 306, hover lock clear, `jet_hovers` 0 |
| Ctrl 20 frames | gravity only, `jet_descents` 0 |
| J in the air | 108 fall mode 9 with mouse live (looks +1), landing, ground lease ready |

S and A headings in the same run were skewed by carried momentum between
bursts and one gravity landing (original inertia, no host zeroing). Modern
was re-probed on revision 1 (same Modern code path): hover -19, W/S/A/D
-33/154/-120/61 vs camera -30, Ctrl 459 - matching D08Q. Tests: 68 Python
(`JetpackSchemeTest`), `ttk-input-test` (D08Q jetpack pad case covers both
schemes), `ttk-controls-test`, `ttk-aim-test` PASS.

## Remaining

Accepted. Not probed: fuel-out mid-flight, low ceilings,
water below, damage in flight, controller.

# D08Q1 - Faster Modern Ctrl descent

**Done (2026-09-29, user: "verified working!!!").** User: "for jetpack modern, i'd like ctrl to
descend be faster. basically the speed of ctrl in water to descend".
`k_jet_descend_velocity` 2800 -> 5500 (Modern only).

Target: the underwater Ctrl dive (anim 133, state 5) moved y -1438 -> -888 in
19 frames (~29 units/frame) in the D08O deep-water lab (`/tmp/swimlab/deep4.log`,
zone 35 teleport). The descent velocity scales linearly.

| Value | Steady Ctrl descent (units/frame) | Landing |
| --- | --- | --- |
| 2800 (D08Q) | ~16 (459 / 30 frames) | soft |
| 5100 | 27.0 | anim 63, health unchanged |
| **5500** | **29.5** (Ctrl+W 26-33) | anim 63, health 10000 -> 10000 |

Binary `9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.
Regression: Modern hover -32 / 60 frames, W/S/A/D -33/154/-120/61 vs camera
-30, mouse 3758 -> 66, Space climb; Classic unchanged (no host descent, 0
lease losses). Native suites PASS. The last samples before touchdown speed up
(floor approach `8003ef78`), as before.
