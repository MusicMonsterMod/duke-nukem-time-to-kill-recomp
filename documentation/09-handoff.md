# Next-session handoff

## 2026-09-29 - D23B intro FMV stranded movie shard (Done)

Intro-only stutter after D23A. Cause: `psxrecomp-game --overlay-config-hash`
covers the `game.local.toml` hook list, so each new host hook moves the
movie shard's cache folder (`cache/SLUS-00583/gcc/linux-x64/cg10_..._gc<hash>_f0`)
and the loader silently interprets MOVIE.OVR. Now: `ttk-movie-shard` target in
the `local-dev` build preset, `run.py` `ensure_movie_shard()`, and
`fmv_poll.c` logs `ttk-fmv: native movie decoder active (19 functions)` or a
WARNING carrying `overlay_loader_last_msg()`. After adding a hook, just build
or launch normally. If the intro stutters: check the `ttk-fmv:` line in the
session log first. Launch tests in `tests/local/test_player_profiles.py`
must pass `--no-session-log` and stub `run.ensure_movie_shard`, or they
start the real game on the player's cards. Binary
`3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`. No commits until asked.

## 2026-09-29 — D23A Modernized frame budget: identity guard cost (Done, user-accepted)

User: "that stuttering audio/slowness issue" before the D08Q playtest.
Measured with the runtime's telemetry in an isolated Xvfb instance
(`/tmp` harness; `audio_stats`, `phase_profile`, `phase_hot`, `frame`,
`ttk_input`): Modernized 47.5–49.3 fps + continuous underruns, Vanilla
60.0 clean, Sep‑27 binary 57.5. Cause: `identity()` walked all 20,305
guarded words via `psx_mod_read_word` on every call, 46 calls/frame ×
108 µs = 5 ms/frame (24 % wall). Fix: `code_identity.h` memcmp fast path
on `g_psx_ram` (exact per-word fallback keeps the apartment substitute),
`IdentityMemo` per `input_host_frame()` × `g_dirty_ram_code_gen` in
`modern_controls.cpp` and `weapon_aim.cpp`. Now 1 check/frame, 28 µs,
59.94 fps, 0 underruns, fill 267 ms. Debug JSON `identity_calls`,
`identity_checks`, `identity_us`. Tests: mocks define `g_psx_ram` /
`g_dirty_ram_code_gen`, `poke()`/`pokeb()` bump the generation; new memo
case. All three native suites PASS. If stutter returns: check
`identity_checks` ≈ 1/frame and `audio_stats.out.fill_ms` vs target first;
`perf` is unavailable (paranoid=4). Do not add per-call guard walks again;
new guards are now nearly free. Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. No
commits until asked.

## 2026-09-29 — D08Q modern jetpack flight controls (Done, user-accepted)

Accepted: "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."


User: jetpack uncontrollable except Space; WASD "blocked"; modern controls
fail. Isolated baseline on `79a3fd5b…` (turret-room / ledge states copied to
`recomp/analysis/d08q-jetpack/`, `dninventory` cheat, J, Space): original
mode 10 (anims 163–170) matched no lease → no mouse; WASD via tank fallback
relative to an unturnable body; idle gravity landed Duke in 1–2 s. New
`recomp/src/ttk/jetpack.inc` from the existing `8005a210` hook, camera-only
lease clause `jet` in `state()`, `jetpack_input_ready()` for `input_pad()`
(W/S → Up/Down, A/D → layout strafe pads) and `select_weapon` (J only in
flight), `locomotion_input_ready`/`airborne_input_ready` false in flight and
the 108 fall grace. Host writes: hover lock `+0x224 |= 0x08000000` + base
`+0x860 = Y` when idle; Ctrl → lock off, `+0x1f8 = 2800`, `+0x1e8 = 0`; WASD
→ lock off, `+0x1f8 = 0`, `+0x1e8 = 18` trim; `face_view()`. Do **not**
drive `+0x860` for descent or hover during WASD (57-jetpack-controls.md,
"Rejected on evidence"). Guards `8004aaf8`/272, `8004ac08`/472,
`8004ade0`/1972. Live probes: headings within 4° of the camera, level ±25,
25–29 units/frame, Ctrl 489/30 frames → 105, J off → 108 with live mouse →
ground run. ttk-input-test, ttk-aim-test, ttk-controls-test PASS (controls
test: use `analysis/pc-input/d08-camera-final/level00-guard-fixture.bin`;
the `d08p-turret` fixture is Modernized-patched and fails the Vanilla
assertion). Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. Playtest:
J, Space, mouse, WASD, release → hover, Ctrl → land, J in the air. If the
feel is off, the tunables are `k_jet_descend_velocity` (2800) and
`k_jet_level_trim` (18) in jetpack.inc; `DNTTK_JET_TRACE=1` logs each
update. No commits until asked.

## 2026-09-29 — D08P overlay scratch identity + mid-depth wade handler (Done)

User: the whole ledge area (F7 UI slot 3) kills run, mouse and jump for good;
depth is not the cause. Their `session-20260929-005059.log` showed
`[TTK lease] inactive (identity)` from the ledge onward. Live guard diff on the
isolated slot 3 copy: only `0x800ccf1c/20/24` changed — the LEVEL00 overlay's
trailing scratch vector, written by the zone script's `0x8007177c` hit test
from overlay code `0x800cb1a4`. The overlay guard now covers 9652 bytes
(code/tables), digest `274d71dd…`; `ttk_state_probe.py` matches; the controls
test proves the vector write keeps the lease and the last table word is still
guarded. Then the same water (depth 512, original mode 1 / clips 80/81,
Vanilla 8–14 units/frame) got its handler `0x800539f8` hooked — added to
`game.local.toml` `mod_function_entry_funcs`, regenerated (one generated line;
`build.py` itself stops at the pre-existing runtime patch-stack drift, so
generation was run directly with the built recompiler) — retargeting the
world root to camera-relative WASD in the land run band (gain 4, 80..120 per
tick). Guards added for `0x800486b0`/260, `0x800539f8`/452, `0x800788e0`/200.
Live: 45–58 units/frame, mouse steering, strafe, wade jump 98, 0 refusals,
turn/run/jump after the zone; slot 2 regression clean. Wading into a wall
idles (original handler); turning resumes. ttk-input-test, ttk-aim-test,
ttk-controls-test PASS. Binary
`79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`.
Isolated cards: `recomp/analysis/d08p-slot2/cards/openbios/` now holds copies
of state slots 01 and 02. User playtest: "it really works. perfectly" →
D08P **Done**; docs committed and pushed. Next session: pick the next job from
the board (no job auto-started).

## 2026-09-29 — D08P held keys across recapture, fresh capture offers (Needs playtest)

User: iteration 4 changed nothing for them (session log shows a fresh launch
and F7 slot 2 load 30 s before the report). Live Xvfb + xdotool probes
(`/tmp/d08p_live_probe*.py`, isolated `recomp/analysis/d08p-slot2/`, software
and OpenGL, debug load and the real `F7 → 2 → l` menu, the user's own profile)
all show the wade at 48–54 units/frame with mouse look; slow headings are the
original wall slide beside the spawn. Reproduced instead: a Shift/W held
through any recapture was wiped by `clear()` → walk gait + Walk pad until
re-pressed. Fixes: capture resyncs bound keys/mouse buttons from SDL device
state; capture offers expire after 8 frames so the original pause menu is not
captured; sustained tank fallback announces `ORIGINAL MOVEMENT (reason)`;
`run.py` mirrors stderr to `recomp/build-local/logs/session-*.log`. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Binary
`f8122f58e828a35b39fd11708195ccb5901b24e7e5207e593446b70b658564fb`.
If the playtest still degrades, read the session log's `[TTK lease]` /
`[TTK input]` lines first. Turret itself not reached in probes (S doorway →
bridge → deep water; N platform → W walkway dead end). No commits until asked.

## 2026-09-28 — D08P waist-deep land locomotion (Needs playtest)

User: after speed recovered, the turret wade still crawled, mouse look was
dead in the water, and after leaving they only walked with no Shift. Isolated
slot 2 already uses land run anim 78 on D-pad forward; Modernized dropped
the lease on tank turns 70/71 and injected Walk. Waist-deep / mid water
now keeps the land camera lease, faces the view, converts 70/71 to run,
and does not inject Walk. ttk-input-test, ttk-aim-test, ttk-controls-test
PASS. Binary
`b642ab9fbfdac845a0e3c23157b3e2550f1fde520d4351b2b2585b4764414ecd`.
Playtest F7 slot 2: run + mouse in the wade and after stepping out. No
commits until asked.

## 2026-09-28 — D08P identity reader stall (Needs playtest)

User: after the slot-2 recapture binary, everything ran slow including
audio. The apartment pair check ran on every word of every identity
guard, many times per frame. It now only inspects the two LEVEL00
apartment words. F7 recapture behavior is unchanged.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`44abdda343cc0014f3bfd6602cb42211d8807e00f2f5d57d30a62eac672b5d4c`.
Restart the player build; the running session will not pick this up.
No commits until asked.

## 2026-09-28 — D08P F7 slot 2 recapture (Needs playtest, iteration 3)

User: iteration 2 did not restore the turret room; F7 UI slot 2 (file
`state_800AB6FC_slot01.pst`, turret around the corner) lost Modernized
entirely. Isolated dump matches the flooded wade (depth `0x100`, anim 63,
normal camera, apartment words already patched). F10 left
`initial_capture` false; F7 released the mouse and did not request
recapture; the F7 menu could eat `capture_offer`. F7 now sets
`initial_capture` even if the cursor is already free; offers stay until
`allow_capture`; identity remaps the complete patched LEVEL00 pair.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS (including F7
recapture and slot-2 patched overlay). Binary
`48e5c25f282d65741a94fd69eb58fc5409f7ac1fcf3282cbdf9bab9604e87336`.
Playtest: F7 load slot 2, mouse/WASD/strafe/crosshair without a manual
F10. No commits until asked.

## 2026-09-28 — D08P turret wade (Needs playtest, iteration 2)

The placed checkpoint is host F7 savestate slot 1, not a memory card. Isolated
dump: flooded turret room, depth `0x100`, anim 63, normal camera, no
`0x20000000`. First tank fallback made A/D turn (D-pad L/R → anims 71/70) and
classified 70 as free swim, which hid the crosshair and blocked Escape
recapture. Fixes: free swim only at original `≥0x281` or states 4/5; A/D
fallback L2/R2 strafe; recapture from `0x8005a210`; reticle on camera-only
locomotion. ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`5cf66b4e2469b0045265e47135a1bdc305d2ae842edc3603caf283e8d78fed56`.
Playtest from that savestate: mouse, A/D strafe, crosshair, Escape resume.
No commits until asked.

## 2026-09-28 — D08P turret WASD freeze (Needs playtest)

Crystal-2 ceiling turret (after first crystal, broken-bridge water corridor)
dropped the modern camera lease. Captured WASD was inert because move actions
use pad 0 and Forward inject requires the lease. F10 could not restore WASD.
First fix: captured play without a lease feeds original D-pad tank bits
(iteration 2 replaced A/D turn with strafe). stderr
`[TTK input] WASD tank fallback` and `[TTK lease] inactive (...)`. Playtest
that room; if the window ignores keys entirely, that is a guest halt, not this
path. D07A/B, D08A, D08H and D10 distance closed on user play acceptance.
Binary `fd9e4c4d015673ec6fa1dcdedb3bcd60d9b74ddb4377786afb5a241d0f4e4c28`.
No commits until asked.

## 2026-09-28 — D08N cancelled (scuba item out of scope)

User: do not add a scuba device to this game. D08N is **Cancelled**. TTK
underwater air stays original and automatic; Bio Mask stays gas-only. No scuba
prototype, inventory slot, HRP scuba art, or toast. Historical notes that once
listed D08N as a swim follow-on are superseded. Next job: pick from the Todo
list (D08L, D09, and other open rows; not D08N). No code change. No commits.

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
Next job: pick from the Todo list in `MODERNIZATION_JOBS.md` (D08L, D09 are
open; D08N scuba is cancelled). No commits.

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
