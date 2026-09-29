# D11 - First-person playable prototype

**Done - user accepted 2026-09-29:** "i have been playing it for a while, and im insanely happy with it. mark all as accepted. this is phenomenal, and it's like a dream come true." An optional eye-level camera for Modernized
with the independent camera. **P** toggles first/third person in game. The
view is saved in the profile (schema 13, `--view first|third`, default third).
Vanilla, and Modernized with the `original` camera option, never use it.

## What it does

The prototype reuses the D06 orbit and D07 view aiming. It does not move the
camera after the game has placed it. It changes the anchor that the original
normal-camera update (`0x8003ade4`) solves, as the orbit does, and the
original obstruction rays, room update and height queries all still run.

| Piece | Original detail relied on | Host change |
| --- | --- | --- |
| Eye position | Duke's joint matrices at `[player+0x3c]`, 0x20 each, world translation at `+0x14`. The model record at `[player+0x40]` has 19 joints. Joint 9 is the neck/head (level 3, child of the spine, 186 up). | Eye = root + smoothed (neck - root) + 96 up. Only the neck's sway about the root is smoothed (0.4 per update), never the root's own travel. The joints are updated (`0x800987cc` / `0x80097c04`) before the camera in every frame. |
| Anchor | `0x80039c7c` puts the camera at anchor - forward x `camera[+0x42]`/2. | The anchor is requested half a projection distance (193) ahead of the eye. |
| Minimum distance | `0x8003aa48` pushes the camera back out to the word at `0x800c3c48` (768). Only this routine reads it. | Written to 768 x (1 - blend) at that call. Restored to the executable's constant at the update's next hooks (`0x8002a038`, `0x8002a7fc`) and on the next camera entry. A relaxed value that was captured (save state, debug read) is repaired, never adopted. |
| Follow lag | The update eases per view axis with `0x8002a038(delta, rate)` = delta/rate, rates from the state table `0x800c0d4c` (2/5/4 on the ground). Unchanged, the eye trailed the neck by 250-550 units while running. | Rates scaled toward 1 at the three calls from that update (return `0x8003af40/60/80`, same frame). |
| Head | `0x800348d8` draws an actor joint by joint and skips a joint whose record flag byte has bit 0. | Bit 0 set on Duke's joint 9 record only from Duke's draw entry (`ra 0x8003769c`) to the next object-list step (`0x8001ca4c`, `ra 0x800376b4`). |
| Body | The same draw loop skips joints nearer than GTE H (`0x800b4d2c` reads control register 26). | None. The body is culled when looking down (joints are within H). The weapon is not visible either; that is D12. |
| Projection | Camera `+0x40` holds the view angle (512 = 45 degrees). `0x800286b0` derives `+0x42` = 160/tan(22.5) = 386. The render loads GTE H from `+0x42` every frame via `0x800b4d9c` (from `0x8002e4c8`). The world renderer flags vertices nearer than H/2 (`0x8002f1e0`). | The eye view passes 256 there (about 64 degrees wide; near 128 instead of 193). `camera[+0x42]` is never written. Developer override `DNTTK_FP_PROJECTION=160..386` (not a saved preference). |
| Plain jumps | A Space-only standing jump has no D08C flight lease, so the orbit dropped to the original camera. | While the eye view is live, a jump keeps a camera-only lease through animations 96-98, 103-105 and 109. No velocity redirect, no `flight_valid`. Third person is unchanged. |

The view blends in and out over about 10 frames (0.7 per update). Unsupported
states blend back to the third-person orbit: swimming (swim modes 4/5 and
deeper water; waist-deep wading stays first person) and jetpack flight. States
without the orbit lease (ladders, ledges, scripted and turret cameras, death,
menus) switch to the original camera at once, as before.

New `game.local.toml` hooks: `0x800348D8`, `0x8002A038`, `0x800B4D9C`
(`0x8001CA4C` was already hooked). Debug JSON `ttk_input` -> `controls` -> `fp`:
`requested`, `supported`, `blend`, `reason` (off/active/unsupported/lease/orbit),
`updates`, `fallbacks`, `follows`, `head_hides`, `head_flag`, `min_boom`,
`projection`, `projections`.

## Evidence

Final binary `dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`.

- Python: 74 tests pass. New: v12 -> v13 migration adds `view` third and
  `camera_view` P without taking a custom P binding; side-file view is
  optional and validated; `--view` launch environment; Vanilla third.
- Native: `ttk-input-test` (P toggle from the saved view, auto-repeat one press,
  no guest command, uncaptured and `original`-camera presses ignored),
  `ttk-controls-test` (eye anchor, minimum distance relaxed and restored within
  the update, captured value repaired, projection argument only from the
  render call, head flag only between Duke's draw and the next list step,
  lease loss restores everything, third person untouched), `ttk-aim-test`.
- Isolated route (`recomp/analysis/d11-first-person/runs/route-final`, fresh
  private cards, Xvfb, software renderer, god mode and hidden AI by typed cheat
  after the fight). All 14 checks pass:
  first-level street spawn in first person; 7 view-aimed shots at the club
  door, 0 rejected, actor hits (`hit_kind 2`); four waypoints to the club
  door; 360 degree sweep at the doorway threshold (street and club both
  render); entry hall and main room sweeps (tables, stage, bar, the street
  through the entrance); look up/down; Space-only jump stays `active`;
  crouch lowers the eye with the pelvis; P to third person and back
  (minimum distance 768 and projection 386 in third person, 40 reads);
  `camera[+0x42]` never written; no fallbacks.
- Earlier runs (turret-room corridor): walking and running keep the eye 97
  above and within about 25 of the neck; shots follow the crosshair in both
  views; death falls back to the original camera.
- Frame budget, same club state, 15 s of walking and mouse sweeps: third
  person 59.87 fps, first person 59.94 fps, 0 audio underruns each.
- Vanilla regression route `analysis/vanilla-regression/d11-vanilla` exit 0;
  captures show the original camera, fire, jump, inventory, forward and turn.
- The intro movie shard is current (`ttk-fmv: native movie decoder active`).

## Limits

- **Near walls.** Facing a wall renders it now (it was black at the original
  projection distance). Standing against a wall and looking along it at a
  steep angle can still drop or tear the wall polygons that reach beside or
  behind the eye (club side wall, about 60 degrees off its normal). The world
  renderer rejects such polygons instead of clipping them; neither a shorter
  projection distance (192-256) nor moving the eye back 64-128 fixed it.
  Rooms are not missing; stepping back restores the wall. Follow-up: D11B.
- The original obstruction rays did not report the wall Duke was touching, so
  they do not keep the eye off walls.
- No held weapon or hands are drawn; firing shows the crosshair and impacts.
  Body and weapon presentation are D12.
- Switching to an unleased state is a cut to the original third-person camera.
- The field of view is fixed at about 64 degrees; FOV options belong to D14.
- Tested on the first map's street, club entrance, entry hall and main room,
  and the turret-room corridor. Not a campaign pass.
