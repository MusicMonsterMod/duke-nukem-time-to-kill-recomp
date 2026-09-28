# Next-session handoff

## 2026-09-28 - game/ drop folder (tested)

First run of `run.py` / `build.py` creates `game/` with a README. Importer accepts
Redump USA `.cue`/`.bin` or CloneCD `.img` (hashes `708c0404...` and `230a34c2...`)
when the EXE pin matches. Europe/PAL `SLES-01515` prints a console error. User
tested the drop-folder flow. Original media is never overwritten.

## 2026-09-28 — D08O closed (accepted); nothing pending from the swim work

Playtest 3 on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb` accepted
everything: A/D strafe, Ctrl dive, underwater W/A/S/D/Space/Ctrl, mantle-only
exit, the round-3 shallow → platform ledge jump, and the `biomask-small.png`
item-switcher icon. D08O is **Done**; the entries below are history. Do not
reopen the swim/ledge code without a reported regression (55 §6 rules 7–9).
Next job: pick from the Todo list in `MODERNIZATION_JOBS.md` (D08N scuba item
is the natural swim follow-on; D08L, D09 are also open). No commits.

## 2026-09-28 — D08O.4 ledge jump: plain 98 arc, taller (Needs playtest)

Playtest on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.
Swimming (Ctrl dive, underwater WASD/Space/Ctrl, A/D strafe, mantle) and the
Bio Mask icon are **accepted** — do not touch. Only the ledge jump changed:

1. **Subway shallow water → platform** (west platform, room 21 → 57). From
   touching the wall, from a few steps back, running or standing, Space alone
   or with W: Duke does the ordinary directed jump (98) — not the tall 97 hop of
   round 2 — about a head higher than his normal shallow-water jump, and lands
   on the platform. At the wall / one step back he rises first and moves onto
   the platform only once his feet are above the lip (XZ held, then 3400/4000);
   from further back he travels from takeoff (1900 + 3.5·distance, max 6400).
   Landing is 98's normal landing (sometimes the hard 105). From ≳1650 units the
   ledge is out of probe reach and the plain 98 falls short — intended.
   `ledge_assists` in the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1`
   prints `ttk-ledge probe/launch/clear/abort/ballistic` and per-tick `ttk-swim`
   lines to stderr (`runs/<name>/runtime.log` in the lab).
2. If it still looks too high: `k_ledge_vy` (−9000; 98's own ≈−6900, rise scales
   ≈ VY/96·ticks — −8000 was not tried, apex margin at −9000 is ≈200 above the
   635 ledge). If it bumps the wall: `k_ledge_hold_distance` (320) / the hold
   speeds. If it overshoots into the back wall: `k_ledge_speed_per_unit` (3.5).
3. Regressions to keep: everything in the D08O.2/.3 entry below.

Mechanics (`swim.inc`): `swim_ledge_ahead` (probe 0/320/640/960/1280),
`swim_try_wade_jump` → `swim_start_airborne_jump(…,98)` + `delayed_vertical =
k_ledge_vy` (applied at the `8003ebf4` ballistic hook, `terrain_epoch` gated),
`swim_ledge_ballistic` (called from the same hook each ballistic tick: hold
XZ = 0 below the lip when `swim_ledge_hold_xz`, else exact XZ via
`swim_boost_want` / `swim_boost_horizontal`; clears `swim_ledge_pending` once the
foot is 32 above the top). `swim_update` drops a stale pending flag whenever
anim ≠ 98 or state ≠ 9. Lab: `/tmp/swimlab/ledge17.json` (15 trials),
`deep3.json`, `walk`. No commits.

## 2026-09-28 — D08O.2/.3 native dive + wade ledge assist (Needs playtest)

Playtest on binary
`71d2a1394ca4dad6c6a1e6fe9c1347cdb7eec8ecdacb3fbd54f9009a604edec5`:

1. **Ctrl at the surface** → Duke dives (original anim 133, `+0x22c` = 5).
   Underwater: **W/A/S/D** swim where the camera looks (thrust 128–130),
   **Space** ascends, **Ctrl** descends, looking up/down + W follows the view.
   Duke surfaces automatically (122, state 4); Ctrl again re-dives. If Ctrl is
   still inert: `swim_dives` in the debug JSON must increment; `swim_try_dive`
   refuses while `+0x224 & 0x241` or `+0x244` (timer) is non-zero.
2. **Subway shallow water → platform** (west platform with the hazard stripe,
   room 21 → 57). From touching the wall, from a few steps back, running or
   standing, Space alone or with W: Duke should hop high and land on the
   platform every time, with no bump-off (107) and no splash-cancel. Landing
   is the original hard-landing 105 (the 97 arc is higher than 98's). From
   >≈1300 units the plain 98 still falls short — intended. `ledge_assists` in
   the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1` prints
   `ttk-ledge launch/switch/abort` and probe lines to stderr.
3. Facing a plain wall in shallow water with W+Space → vertical hop (was a
   cancelled 98 splash).
4. Regressions to keep: A/D surface strafe (123/124), Space ascend only, E /
   mantle exit, shallow W/S/A/D wade, Space 98 in open water, Ctrl 105.

Mechanics (`swim.inc`): `swim_ledge_ahead` (standing probe `800797f4` from
0/320/640/960 along the view, world XZ written and restored inside the query),
`swim_try_wade_jump` launches 97 and arms `swim_ledge_pending`,
`swim_ledge_assist_update` (early hook, runs without the swim lease) switches to
98 when `+0x10` ≤ ledge top − 96, keeps VY via `delayed_vertical`/`delayed_takeoff`
(applied at the `8003ebf4` ballistic hook), XZ = 2200 + 3.2·distance (max 5400)
via `swim_boost_want`; `swim_reassert_boost` now also runs in flight.
Native swim: `swim_try_dive` (`80045564`), `swim_steer_underwater`,
`swim_thrust_input_ready` (Square injection in `pc_input.cpp`). Lab scripts live
outside the repo in `/tmp/swimlab` (`swim_lab.py`, `ledge*.json`, `deep3.json`).
No commits.

## 2026-09-28 — D08O implemented (Needs playtest)

Playtest the deep-water trio on binary
`d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e`:

1. **A/D** while swimming → camera-relative strafe (anims 88–93), no turning.
   If A/D still turn, dump `800d1a68/800d1b80[player+0x233]` — the strafe pad
   bits are resolved from those layout words (default L2=0x100 / R2=0x200).
2. **Ctrl** at the surface and mid-water → descend; **Space** → ascend to surface.
   Dive stops at `+0x1c8 − 0x1e0` (floor margin) or `surface + 0x1800`.
3. **Space** at a ledge no longer hops out; leave via **E** / original mantle.
   Shallow wade jump / ledge hop (`8003e2d0(97|98)`) is unchanged.

Regression already confirmed in the shallow subway savestate: W/S/A/D wade with
stable heading, Space wade jump 98, Ctrl crouch 105. ttk-controls-test and
ttk-input-test PASS. No deep-water savestate exists; deep zones (rooms 31, 35,
42, 64, 65, 68, 72) sit behind a keypad door (−7400,51700) and a fence
(40636,49390) from the subway state, and teleporting snaps Duke to geometry.

Code: `recomp/src/ttk/swim.inc` (`swim_strafe_pads`, `swim_apply_vertical`,
`swim_try_mantle` removed), `pc_input.cpp` free-swim block,
`modern_controls.h`, `tests/local/pc_input_native.cpp` stub. No commits.

## 2026-09-28 — D08O marked next (docs only)

**Next job to pick up: D08O** — deep free-swim polish.

Locked design in `documentation/55-swim-controls-research.md` (§1.2, §8) and
`documentation/54-swim-redesign.md`:

1. Fix A/D camera-relative strafe (W/S already OK)
2. Fix Ctrl descend from surface / mid-depth
3. Strip free-swim Space→ledge exit → **mantle / E only** (keep shallow wade jump)

D08M closed: shallow jump + ledge hop Done (do not reopen unless regression).
Space↑, W/S, E mantle accepted on binary
`47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`.

Hard rules: no invented `+0x20c`, no entry `+0x1f8` clear, no world-XYZ swim.

This session: documentation/design only — no `swim.inc` changes.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

No commits.
