# Current status — 2026-09-29

**D08R Done (user-accepted: "it's absolutely rock solid").** **D08Q1 Done
(user: "verified working!!!"):** Modern jetpack Ctrl descent 2800 -> 5500, steady 29.5
units/frame to match the underwater Ctrl dive; soft landings, no damage.
Binary `9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.

**D08R selectable jetpack scheme (Done).** `run.py --jetpack
classic|modern` (persisted, Modernized, default modern). Classic (revision 2,
after user feedback) keeps all modern controls - mouse look, camera-relative
WASD, J - with the original burst physics: Space boost, gravity, no host
hover or Ctrl descent. Details: documentation/57-jetpack-controls.md (D08R
section). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`.

**D23B Done - intro FMV stutter was a stranded native movie shard.** The
shard's cache folder is keyed by the `game.local.toml` hook list; the 28 Sep
hook additions moved it, so the intro decoder ran interpreted (~52 fps,
continuous underruns). Rebuilt: 59.92 fps, 0 underruns; the user confirmed
smooth playback. The build preset and `run.py` now re-check the shard on every
build/launch (~0.2 s), and the session log says `ttk-fmv: native movie
decoder active` or warns. Details: documentation/59-fmv-shard-namespace.md.
Binary `3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`.

**D23A Done (user-accepted) - the Modernized stutter/slowness had a measured
cause and is fixed in the current build.** Isolated probe: Modernized ran
at 47.5–49.3 fps with continuous audio underruns (output fill 17–34 ms
against a 180 ms target) while Vanilla ran 60 fps clean. `ttk::identity()`
re-read all 106 guards (81 KB) word-by-word through the guest bus on every
call — 46 calls per frame, 108 µs each, 5 ms of every 16.7 ms frame (24 %
of wall time). It now memcmps the runtime RAM image once per host frame
(re-verifying immediately on any RAM-code generation change) and costs
28 µs per frame. Result: 59.94 fps, 0 underruns, fill 267 ms; jetpack flight
60.1 fps. Details: documentation/58-modernized-frame-budget.md.

**D08Q Done (user-accepted) - modern jetpack flight.** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful." User: in the jetpack only
Space worked, WASD were "blocked", mouse dead. Original mode 10 had no lease;
WASD reached it only through the tank fallback relative to an unturnable body,
and its idle gravity landed Duke within seconds. New host layer
(`recomp/src/ttk/jetpack.inc`) over the unchanged original handler: mouse
orbit with the body facing the view (so the original body-relative thrust is
camera-relative), W/S/A/D → original Up/Down + L2/R2, Space climb (original),
release everything → hover (original hover lock at the current height), Ctrl
→ steady descent to a soft landing, J in the air → original cut-out fall with
the camera still live. Probes: headings within 4° of the camera, level flight
±25 units, 25–29 units/frame forward/back, Ctrl 489 units/30 frames then
landing, ground controls resume. Known: stop-on-release is immediate; after
Ctrl release Duke settles ~130–300 units more near a floor; fuel drains while
hovering (original rule). Details: documentation/57-jetpack-controls.md.

Binary `81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.

**D08P Done (user-accepted) — area-wide control loss was overlay identity;
mid-depth wade now runs at land pace.** The user's session log showed `ident=0` from the
flooded-corridor ledge onward. The LEVEL00 zone script writes a hit position
into the overlay's trailing 16-byte scratch vector (`0x800ccf1c`), which the
9668-byte overlay guard covered, so identity failed permanently the first time
that script fired. The guard is now the 9652-byte code/table body. Separately,
that water is depth 512: the original swaps every gait for wade clips 80/81
(Vanilla 8–14 units/frame, forward only). The wade handler `0x800539f8` is
now hooked (new generated entry) and its root retargeted to camera-relative
WASD at the land run band: 45–58 units/frame, mouse steering, strafe, Space
jump; 0 identity refusals across and past the ledge. Wading straight into a
wall idles as in the original; turn the view to resume. Details:
documentation/56-scripted-camera-controls.md (iteration 6).

Pending playtest jobs D07A, D07B, D08A, D08H and D10 distance were **Done** on
user acceptance of ongoing play. D08N scuba remains cancelled. D10 broader work
stays In progress. D08B is Done on 2026-09-29 after the user playtested all
of its recorded limits (see the job board work log).

Previous binary `79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`
(D08P acceptance).

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
