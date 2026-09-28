# D08M / D08O — Swim controls research & case study

Standing research note for modernized underwater / wade controls. Linked from
D08M (accepted foundation), **D08O (Done, accepted 2026-09-28)**, and
[54-swim-redesign.md](54-swim-redesign.md). Binary evidence and playtest logs
live in the board work log; this document owns the **contract**, **original
deconstruction**, **iteration autopsy**, **FPS/TPS case study**, **locked
architecture**, and **next-job design**.

D08O was implemented 2026-09-28 from §8; playtest 1 accepted A/D strafe, Space
ascend and mantle exit, and reported Ctrl dead plus an unreliable shallow →
subway-platform jump. Round 2 (§9) drives the original's **native swim state
machine** for the dive (playtest 2 **accepted** — "swimming works perfectly")
and adds a **wade ledge assist**; round 3 (§9.3) rebuilt the assist on the
plain directed 98 after the user found the 97 → 98 hop far too high; playtest 3
accepted everything (§9.5). **D08O is Done.**

---

## 1. Player contract

### 1.1 Accepted (D08M) — do not regress

| Action | Feel | Status |
| --- | --- | --- |
| Shallow Space | Land-like jump via `8003e2d0(97\|98)` | **Done** |
| Shallow ledge | Facing Space hop-on; anti-bounce holdoff | **Done** (call complete; revisit only if regress) |
| Mount / E out | Mantle / grab exit from water | **Works** (keep) |
| Deep Space | Ascend to free surface | **Works** (keep) |
| Deep W / S | Forward / back on surface and in water | **Works** (keep) |
| Oxygen / water flag | Original `+0x20c` ownership only | **Hard rule** |

### 1.2 D08O scope (implemented; all accepted 2026-09-28)

| Action | Expected Modernized feel | Iter 9 playtest |
| --- | --- | --- |
| A / D | Camera-relative **strafe** | **Fail** → fixed, playtest 1 **Pass** |
| Ctrl (deep) | **Descend** into water | **Fail** → floor-margin gate still dead in playtest 1 → §9 native dive → **accepted** playtest 2 |
| W/A/S/D + Space/Ctrl (underwater) | Swim where the camera looks; Space up, Ctrl down; auto-surface | §9 — **accepted** playtest 2 |
| Shallow Space at a subway platform | Land on the platform from the wall or a run-up, no distance puzzle, **ordinary-looking jump** | Regression reported → §9.3 ledge assist; round-2 97 hop rejected as too high → round 3 plain 98 — **accepted** playtest 3 |
| Mouse | Free look; body yaw follows view; upright | Keep |
| Look + W (deep) | Optional vertical steer while submerged | Keep if already soft |
| **Exit water** | **Mantle / E only** — strip free-swim Space exit hops | Space-at-ledge still exits; **simplify to mantle-required** |

Vanilla water unchanged. D08N (scuba item) stays out of scope.

### 1.3 Exit policy (locked for D08O)

User direction after deep-water playtest:

> Strip down some of those changes we made to the mechanic and just make it so
> that Duke has to **mantle** out of the water.

| Was (iter 9) | Next (D08O) |
| --- | --- |
| Fresh Space + climb-ahead → directed exit hop while water-flagged | **Remove** free-swim Space→ledge exit path |
| E / mantle works | **Primary** (and only intentional) water exit |
| Shallow land-like Space jump | **Keep** (not an “exit hop”; wade jump) |

Rationale: Space in deep water stays **ascend**; exit is a deliberate grab, not a
hop puzzle or accidental surface vault.

---

## 2. Original TTK water deconstruction

Sources: `recomp/analysis/modern-controls-pass/8005201c.txt`,
`recomp/analysis/d07-continuation/80041060.txt`, `recomp/src/ttk/swim.inc`.

### 2.1 Actor fields

| Offset | Role |
| --- | --- |
| `+0x834` | Water surface Y; `0x40000000` = no water |
| `+0x20c` | Water **ownership** flag (nonzero ⇒ water path / oxygen) |
| `+0x1c8` | Support / floor Y used for depth |
| `+8` | Body Y (**Y-down**); submerged when `(+8) > (+0x834)` |
| `+0xfc / +0xfe / +0x100` | Local root deltas before yaw rotate + collide |
| `+0x1f4 / +0x1f8 / +0x1fc` | Ballistic velocity (land jumps); swim does **not** use VY like land |
| `+0x1c / +0x24` | Heading; `+0x1e / +0x26` pitch (176 can pitch the body) |
| `+0x233` | Pad/stick bank index into `800d****` tables |

**Depth** = `(+0x1c8) − (+0x834)` at `80052128–80052148`:

| Band | Meaning |
| --- | --- |
| `< 0x200` | Shallow / wade-side |
| `[0x200, 0x281)` | Mid (thrust often early-outs) |
| `≥ 0x281` | Deep → Square thrust forces anim **176** |

### 2.2 `8005201c` control flow

1. Early-out on `+0x224` bits `0x20000000` / `0x80`.
2. Stick axes via `800d1600` / `800d1b48` (indexed by `+0x233`) → magnitude → run bit (`s2`, threshold `0x51`).
3. `jal 0x80041060` (pad table `800d1b58`):
   - **1** → if `+0x20c` set, force anim **176** (`0xb0`) when depth ≥ `0x281`
   - **2** → anim **175** (`0xaf`)
4. D-pad bit tables (bit0): `800d1ce8` forward → `8007926c(...,4)`; `800d14f8` mode 5; `800d1a68` / `800d1b80` strafe/back family.
5. Root: read `+0xfc/+0xfe/+0x100` → `80066e34` yaw rotate → `8003f040` collide.
6. **`sh $zero, 0xfe($s0)`** at `8005244c` / `8005287c` — **vertical root is cleared** before collide. Swim vertical is **not** dry-land `+0x1f8` ballistics.

### 2.3 Anims

| Anim | Role |
| --- | --- |
| 70 | Water idle |
| 80 / 81 | Forward / swim move |
| 175 / 177 | Water variants |
| **176** | Square thrust / dip (also crouch-enter on land) |
| 72–79 / 63 | Land / wade gaits in shallow |

---

## 3. Modern binding mismatch

| Modern | Pad bit | Vanilla water meaning |
| --- | --- | --- |
| Space (`jump`) | **Square `0x8000`** | Swim **thrust** → anim 176 in deep water |
| LMB fire | Cross | Fire / interact family |
| Ctrl crouch | *(no pad)* | Nothing unless host adds dive |

**Implication:** Space≠ascend in the original integrator. Square starts a pitched thrust clip. Zeroing body pitch (`+0x1e/+0x26`) prevents upside-down posing but **does not** create upward travel. Host must own vertical (soft Y) while **suppressing Square** in free swim, or Space will never feel like modern ascend.

---

## 4. Iteration autopsy

| Iter | Symptom | Root cause |
| --- | --- | --- |
| Early | A/D/S all go forward | Ground Forward-inject while water camera lease made `locomotion_input_ready` |
| Mid | Upside-down on Space | Space→Square→176 + camera-relative yaw |
| Mid | Surface hop loop | Soft “ledge” = headroom; held Space+W fired airborne 98 |
| Mid | Stunted wade jump | Invalid `8003e2d0(96)` (`anim−0x61`) |
| 7 | Mid-air freeze; oxygen above water | Host wrote world XZ/Y, **forced `+0x20c`**, forced swim anims, **cleared `+0x1f8`** on entry |
| 8 | W weak; Space no ascend; Ctrl digs to floor | Square≠ascend; only Ctrl host-Y; D-pad/stick incomplete |
| 9 ship | Root-bridge: Square suppress + soft Y + D-pad/stick/root | See §7 |
| **9 playtest** | A/D still dead; Ctrl still no descend; Space up / W/S / E mantle / ledge OK | Strafe bridge incomplete; Ctrl soft-Y ineffective at surface/margin; exit still Space-hop capable |

**Accepted freeze (D08M):** shallow jump height; shallow Space-facing ledge hop (complete — revisit only if needed).

---

## 5. Modern FPS / TPS swim case study

Patterns from production TPS/FPS and common PC templates (Tomb Raider “modern” swim, modular third-person swim kits, Crest/Invector-style docs):

| Pattern | Typical binding | Notes |
| --- | --- | --- |
| Camera-relative XZ | WASD | W toward camera forward; A/D strafe; body faces view or move dir |
| Look-relative 3D | Mouse + W | Underwater, pitch aims the swim vector |
| Explicit vertical | **Space up / Ctrl down** | Dominant PC keyboard convention in shooter-like templates |
| Surface lock | Auto or toggle | Horizontal at surface; look-down or Ctrl to dive |
| Exit | Grab / mantle | Prefer deliberate climb over jump-exit puzzles |
| Breath | Separate meter | TTK already has automatic air |

**Mapping onto TTK Modernized:**

| Target | TTK-safe means |
| --- | --- |
| WASD strafe | Prove D-pad bits + stick words actually reach `8005201c` strafe paths; late `+0xfc/+0x100` rewrite if tables stay dead |
| Space ascend / Ctrl descend | Suppress Square; soft body-Y; **force Ctrl dive** through surface lock / floor margin |
| Exit | **Mantle / E only** in free swim — no Space exit hop |
| Oxygen truth | Never invent `+0x20c`; never clear fall VY on entry |

---

## 6. Hard rules (any future host layer)

1. **Never invent** `player+0x20c` — original owns water / oxygen.
2. **Never zero `+0x1f8` on water entry** — kills fall and freezes mid-air.
3. **Never primary-locomote by writing world `+4/+8/+12`** — desyncs collision and water volume.
4. Original owns surface Y, collide response, and Vanilla.
5. Wade land jump stays `8003e2d0(97|98)` — never `96` as init id.
6. Mantle/ledge probes must be **standable geometry**, not headroom alone.
7. **Do not re-open shallow ledge hop** unless a regression is reported — reopened 2026-09-28 on a user report (§9.3); the assist must look like the ordinary 98 jump (user rejected the vertical-97 hop); closed again 2026-09-28 (playtest 3 accepted the round-3 assist).
8. **Native swim states own Y** (`800455bc`, `+0x22c` 4/5): never write body Y there; dive through `80045564`, move through the original thrust (§9.1).
9. A probe may write world XZ **only** inside a query that restores it before returning (`swim_probe_from`); that is not locomotion.

---

## 7. Shipped architecture (iteration 9) — root / input bridge

Binary: `47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`.

```
WASD/Space/Ctrl
    │
    ├─ water_flagged? ─ no ─► leave original
    │
    ├─ wade ─► suppress Square → 8003e2d0(97|98) + anti-bounce holdoff
    │         [ACCEPTED — D08M closed for ledge]
    │
    └─ free swim
           ├─ face_view + upright pitch
           ├─ WASD → D-pad (+ stick tables) → 8005201c   [A/D still fail in playtest]
           ├─ suppress Square; soft Y (Space↑ Ctrl↓)     [Space↑ OK; Ctrl↓ fail]
           └─ fresh Space + climb-ahead → exit hop         [REMOVE in D08O]
```

**Not chosen:** host world-position swimming (failed iter 7).

### D08M acceptance (closed)

- [x] Fall into deep water: no mid-air freeze; oxygen only while flagged
- [x] Space ascends to surface
- [x] Shallow Space-to-ledge without bounce-back sweet spot (user: complete)
- [x] E / mantle exit works
- [x] Research artifact stood up
- [x] Vanilla water unchanged (no Vanilla edits in this pass)
- [ ] ~~WASD including A/D~~ → **D08O**
- [ ] ~~Ctrl descends~~ → **D08O**
- [ ] ~~Mantle-only exit~~ → **D08O**

---

## 8. D08O design and implementation

**Job:** Deep free-swim polish after D08M foundation.

### 8.1 Goals (ordered)

1. **A/D strafe** — camera-relative left/right while water-flagged; W/S must stay good.
2. **Ctrl descend** — leave the surface and dive; must work from surface lock and mid-depth; must not dig through pool floor or clear oxygen falsely.
3. **Mantle-only exit** — strip free-swim Space→ledge exit hop / directed water exit; keep shallow wade jump; keep E/mantle.

### 8.2 Root causes and fixes (implemented 2026-09-28)

| Goal | Root cause (8005201c, idle/anim 63) | Fix |
| --- | --- | --- |
| A/D | Free swim injected **D-pad Left/Right (0x80/0x20)** — TTK treats those as turn buttons (anims 71/70, `800d1ce8`/`800d14f8` words) — and `swim_feed_stick` wrote strafe into the **stick-X word** (`800d1600`), also turn. The strafe branches read the layout's L2/R2 words: `800d1a68[pad]` → left (88/91 land, 92 mid-water), `800d1b80[pad]` → right (89/90, 93). Deep water (≥0x281) takes the same 88–91 branch. | `swim_strafe_pads()` resolves the pad bits from those pointers (`bit = (ptr−800d1440)/4 − 21·pad − 1`; live dump: 8 and 9; fallback L2/R2) and `input_pad()` injects them for A/D in free swim. Stick X is no longer written. Pad-injection proof on the D08M binary: `0xFEFF` → anim 91 moving to Duke's left, `0xFDFF` → anim 90 to his right, D-pad Left → 71 turning. |
| Ctrl | `swim_apply_vertical` only allowed `dy>0` when `body − surface > 0x100`, so a dive never started from the surface (body ≈ surface). | Gate on floor margin: next Y ≤ `min(+0x1c8 − 0x1e0, surface + 0x1800)` (standing body sits ≈ +0x1f4 above the feet). Space ascend unchanged. |
| Mantle exit | `swim_try_mantle` fired `8003e2d0(98)` on fresh Space + climb-ahead near the lip. | Removed (with its latch and `near_water_lip`). Free-swim Space = ascend only. `swim_try_wade_jump` (shallow 97/98 + ledge hop) untouched. |

Pointer tables dumped live from the saved layout (pad 0), bit = PSX pad bit:
`800d1ce8`→4 (Up), `800d14f8`→6 (Down), `800d14e8`→7 (Left), `800d1a98`→5
(Right), `800d1a68`→8 (**L2 = strafe left**), `800d1b80`→9 (**R2 = strafe
right**), `800d1b88`→15 (Square), `800d1b50`→14 (Cross), `800d1b58`→12 (L1);
`800d1b48`/`800d1600` point past the button words (stick magnitude / X).

### 8.3 Out of scope

- Re-tuning shallow ledge hop (Done)
- D08N scuba
- Vanilla water
- World-XYZ / invented `+0x20c` / entry `+0x1f8` clear (banned)

### 8.4 D08O playtest 1 result (binary `d72fa5f8…c121e`)

- [x] A/D strafe while submerged and on surface — **Pass** ("works really well")
- [ ] Ctrl descends — **Fail** ("still feels dead") → §9.1
- [x] Free swim: Space only ascends — **Pass**
- [x] Exit requires mantle / E — **Pass**
- [x] No regression: shallow W/S/A/D wade, Space wade jump 98, Ctrl crouch 105
- [ ] Shallow → subway platform jump — **unreliable** → §9.3
- [x] Vanilla unchanged

Deep water is reachable in the lab by teleport when Y is placed below the
surface and `+0x1c8` is set (`tp 35000,-1000,90000,35,0`); earlier snaps came
from teleporting above the surface.

---

## 9. D08O round 2 — native dive and wade ledge assist (2026-09-28)

### 9.1 The original's native swim state machine

`8004b594` dispatches `800455bc` when `player+0x22c` ∈ {4, 5}; jump table
`0x80013ed8` indexed by anim − 122.

| State / anim | Meaning | Notes |
| --- | --- | --- |
| 4 / 122 | surface idle | `80044c18` pins body Y to `+0x834` **every frame** — no host Y write can dive |
| 4 / 125, 126 | surface forward / back | D-pad Up / Down words |
| 4 / 123, 124 | surface strafe | L2 / R2 words (§8.2) |
| 4 / 134–142 | climb-out (mantle) | Up + `8007be9c(4)` ledge probe |
| 5 / 133 | dive | entered by `80045564(player, player+0x60)` (sound 0x3003) |
| 5 / 127 | underwater idle | |
| 5 / 128–130 | underwater thrust | Square / Cross; vector = body yaw (`+0x1c` + `+0x2b4`) rotated by body pitch `+0x1e/+0x26` (**+900 = up**, −900 = down, original clamp ±900); D-pad Up = pitch down, Down = pitch up |
| 5 → 4 | automatic | when thrusting up through the surface |

`80054cc8`: falling into water thicker than 0x280 enters 5 / 127 directly.
Land-side water anims (70/80/81/175–177) remain the only place the host
soft-Y (`swim_apply_vertical`) applies.

Host layer (`swim.inc`): `swim_native_surface()` / `swim_native_underwater()`
recognise the states; at the surface `swim_try_dive` calls `80045564` on Ctrl
(refused while `+0x224 & 0x241` or timer `+0x244`); underwater
`swim_steer_underwater` writes body yaw from camera-relative WASD (or the view
when idle) and body pitch = Space → +900 (+640 while moving), Ctrl → −900
(−640), else the view pitch (host camera pitch is +down, so body =
−pitch·4096/τ) scaled by the forward share; `swim_thrust_input_ready()` makes
`input_pad()` inject Square while any of W/A/S/D/Space/Ctrl is held. The
original integrator moves Duke and surfaces him. Verified in zone 35: headings
906/3978/1930/2954 for W/A/D/S, look-up + W rises, Ctrl descends, Space
surfaces to 122.

### 9.2 Why the shallow → platform jump failed (measured)

West subway platform (hazard stripe): water floor −5504, surface −5632, wall
at z ≈ 43008, platform floor −6144 (room 21 → 57), ledge **635** above the
feet. Directed jump 98 rises only ~560 (VY −4387) while vertical 97 rises
~1240 (VY −8210) but ignores horizontal motion.

| Start distance to wall | Result before |
| --- | --- |
| touching (≤ 330) | 98 accepted then cancelled on frame one by the original (Duke drops to the surface, "splash"); nothing happens |
| 300–600 | reaches the wall below the lip → 107 bump → 105 back in the water |
| 700–1250 | lands (4/5) — the only window |
| > 1300 | falls short |

`swim_climb_ahead` never fired: the standing probe `800797f4` reaches only
~350 units and, at the wall, reports **result 1** (refused step) with
`floor ≈ −630`, which the climb test (result 0/5) ignored.

### 9.3 Ledge assist (round 3: plain directed 98, taller)

Round 2 launched vertical 97 and switched to 98 at the lip (24/24 landings);
playtest 2 rejected it — "very high jump", "I preferred the more realistic
simple jump". Round 3 keeps the probe and the flight-time XZ control but the
jump is 98 itself.

1. `swim_ledge_ahead(dir)` runs the standing probe from the current position
   and from 320 / 640 / 960 / 1280 units along the view (`swim_probe_from`
   writes world XZ for the query and restores it; `terrain_probe` already
   restores `+0x174..+0x1d4`). A **result 1** with floor 256–1792 above the
   feet is a ledge (rise, distance); a flat result 0 continues the scan;
   anything else stops it. A refused step at 0 with no ledge sets `blocked`
   (plain wall).
2. Space (alone or with a forward-ish intent, not S) launches directed **98**
   via `swim_start_airborne_jump` (`8003e2d0(98)`), arms `swim_ledge_pending`
   with `ledge_top = +0x10 + rise`, `hold_xz = distance ≤ 320`, speed, water
   holdoff 40, and sets `delayed_vertical = −9000` / `delayed_takeoff` /
   `terrain_epoch` exactly as the apartment-bed fix does.
3. At the `8003ebf4` ballistic hook the host applies the takeoff VY (98's own
   is ≈ −6900 after its first gravity step; 98 integrates ≈ VY/96 per tick with
   g ≈ 506/tick, so −9000 rises ≈ 840 instead of ≈ 560 — the same arc, about a
   head taller, apex ≈ 200 above the 635 ledge) and then calls
   `swim_ledge_ballistic` every ballistic tick: while `hold_xz` and the foot is
   not yet 32 above the ledge top, `+0x1f4/+0x1fc` = 0 (98's first stride into
   the wall face was exactly the 107 bump / frame-one cancel); otherwise the
   exact XZ speed is written each tick via `swim_boost_want` /
   `swim_boost_horizontal` — 3400 (offset 0) / 4000 (offset 320) after the
   hold, else `1900 + 3.5·distance` (max 6400 ≈ 98's own 6007; flight ≈ 40
   ticks, travel ≈ speed/4) so Duke lands just past the lip. Once the foot is
   clear the assist releases (`ledge_assists`++) and the ordinary
   `swim_boost_frames` reassert carries the speed to landing.
4. `blocked` with W → plain 97 hop instead of the cancelled 98 (unchanged).
5. Stale flag: `swim_update` drops `swim_ledge_pending` whenever anim ≠ 98 or
   state ≠ 9 (round 2 let it survive a landing and could mis-steer the next
   plain 98).

Tuning trail (lab `ledge12`/`ledge17` trial sets, west platform): VY −5600
apex 90 short → 0/9; −9000 + 2.7/unit → 8/9 (miss: 40° oblique from 1430,
out of probe reach); scan to 1280 → 8/9 (1280 bucket under-speed at 4600 cap);
3.5/unit, cap 6400 → 13/15 (misses: near-oblique starts 340–400 from the
wall bumped on 98's first stride); hold for offset 320 too → **15/15**
(`ledge18`: wall, 350–1350 back, standing/running, Space alone or with W,
±40° off perpendicular). Beyond ≈1650 (1280 + probe reach) the plain 98 falls
short. Full walls (floor −65536) never trigger the assist. Counter
`ledge_assists`; trace `DNTTK_SWIM_TRACE=1` (`ttk-ledge probe/launch/clear/
abort/ballistic`, per-tick `ttk-swim`).

### 9.4 Regression evidence (binary `78b68e42…75bb`)

- ttk-controls-test, ttk-input-test, ttk-inventory-test PASS.
- Shallow: W/S/A/D wade, Space → 98 in open water, Ctrl → 105 (`d08o4_walk`).
- West platform ledge 15/15 (`ledge18`, repeated 15/15 as `ledge19` on the final binary); deep run `deep4` identical to round 2.
- Deep zone 35: Ctrl dive 133 → 5; W/A/S/D thrust 128 camera-relative; look-up
  + W rises; Ctrl descends; Space surfaces (122 / 4); surface W 125, A 123.

### 9.5 Acceptance checklist (complete — accepted 2026-09-28)

- [x] Ctrl at the surface dives; underwater W/A/S/D swim where the camera looks; Space up, Ctrl down; auto-surface (playtest 2)
- [x] Subway shallow → west platform: lands every time from the wall or a run-up; no bump-off, no splash-cancel; looks like Duke's ordinary jump, not a tall hop (round 3, playtest 3)
- [x] Plain wall + W+Space: vertical hop (playtest 3)
- [x] A/D surface strafe, Space ascend only, E / mantle exit (playtest 1)
- [x] Shallow and deep regressions above
- [x] Vanilla unchanged

Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`
