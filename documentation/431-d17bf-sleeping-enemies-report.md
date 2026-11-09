# D17BF - Sleeping enemies: report

2026-11-09. Status: Done, accepted by the user 2026-11-09 ("it's absolutely perfect!! i can see everything i want in the security cams"); hits waking copies continue as D17BF1. Sections 1-7 are the report (no game code changed then); 8-9 the implementation. Evidence: static
disassembly of the owned executable (`recomp/analysis/d17az/mips_dis.py`)
and offscreen runs on a private copy of UI slot 9 (file 8, level 0) with
`recomp/analysis/d17bf/lab.py` (`survey`, `sp`); the player's saves and
profile were hash-checked before and after every run. Labels: **[V]**
verified from disassembly or a run, **[I]** inferred.

## 1. Which spawn points get a sleeping copy

Spawner records `*0x800de720` (count `0x800c56a0`, 48 bytes) **[V]**:

| Offset | Meaning |
| --- | --- |
| +0 | type (type table `*0x800d2660`, 28-byte entries; byte 6 class) |
| +4 | live actor, or 0 |
| +8 | position |
| +0x14 | flags: bit 0 spawned; the tick `0x80099530` skips any row with `0x16` set |
| +0x16 | half: a row a level script holds (D22I); its actor is created asleep (actor +0 bit 0x10000) |
| +0x18..+0x27 | facing as a quaternion, turned into actor yaw +0x1c by `0x8002abec` |
| +0x29 | room; setup `0x8009934c` disables (0x10) a row with no room |
| +0x2a | slot in the position table `0x800d2548` (32-byte entries; +0x1c owner) |

- **0x10, disabled:** set by setup (no room), by the removal `0x80098d70`
  when `0x80096924` fails, and by creation `0x80099018` when it fails **[V]**.
- **0x4, dead:** not written as a constant anywhere; creation copies the
  actor's flags into the row, and the blast and list tests skip actor bits 1
  and 2, so 0x4 (with the 0x4000 high half seen here) is the actor's dead
  state carried back to its row **[I]**.
- The spawn check also needs `0x80098eb0`: the row's position slot +0x2a is
  set and owned by nobody else **[V]**.

So a row gets a sleeping copy when type is nonzero, the type's class is
nonzero, `flags & 0x17 == 0` and `0x80098eb0` would allow it. Script-held
rows (+0x16) need a decision: they are asleep by design in the original, so
drawing them is consistent; they must wake only through the script.

Slot 9, level 0 (`survey`): 71 rows, 12 live (cap 12), **25 eligible**
(21 type 26, 2 type 123, 2 type 204), 15 dead (0x4), 18 disabled (0x10),
6 empty type 0.

## 2. Drawing the copy (pose source)

The actor renderer `0x80032e78(camera, actor, view)` is safe on a stand-in
**[V]**:

- It reads no actor list, room list or row back pointer; the room +0x2e is
  used for lighting only.
- It builds the joint matrices itself on every call (`0x800987cc` static,
  `0x80098204` with look/aim) from the skeleton instance at +0x40. The pose
  is whatever the class animation last wrote into that instance.
- Side effects to avoid: particles and RNG on flags 0x08000000/0x04000000,
  and two once-a-frame routines guarded by +0x5c against the frame counter
  `*0x800bcba4`. With those flags clear and +0x5c preset it can be called
  more than once a frame.
- It writes render state `0x800d67a8..0x800d6830`, the shared bone buffer
  `0x800d6a30` and scratchpad; saving those and calling on a CPU copy (the
  D17AZ `host_vis_reach_draw` method) keeps the frame unchanged.

Stand-in (mod memory, about 0x2cc + skeleton + 0x460 bytes per copy):
creation `0x80098f3c` -> `0x80096614` -> `0x800952ec`/`0x80095504` fills
+0x32 health, +0x14 kind 2, +0x15 class, +0x1a/+0x1b light, +0x22 0x1000,
and copies the type's skeleton template `*0x800ddb14[...]` into +0x40
(`0x80095244`). The stand-in repeats only those copies, with yaw from the
quaternion and no list registration.

Pose: the template is the model's rest pose. If it is not a natural stance
(a bind pose), use a captured pose instead: copy the skeleton instance of a
live actor of the same type while it idles. Only an in-game look decides.
The bounding radius +0x5a is set by the per-type init (not traced); copy it
from a live actor or a captured table, or the renderer's frustum test culls
wrongly.

## 3. Where and how many

- **Visibility:** draw a copy when its room is in the list the object pass
  draws this frame (`0x800374cc`), the same list that draws real actors, so
  monitors and game-owned views (D52) are covered and walls hide copies as
  they hide enemies. Host-visibility frames add D17AZ-style reach for copies
  whose room was culled while their sphere still shows.
- **Budget:** nearest first, at most 16 per frame (slot 9 has 25 rows in
  the whole level; a room rarely holds more than 3).
- **Redraws:** the transform recorder keys on object and call index, so
  copies interpolate like any object; they do not move, so they are steady
  at 120/180 Hz.
- **Lighting:** the actor renderer lights from +0x1a/+0x1b and the room, as
  for real enemies; D41 level lamps apply to whatever the renderer draws.

## 4. Handover when the game spawns the enemy

Creation fills the row's +4 and sets bit 0 in the same tick, before the
frame is drawn, so a copy disappears the frame its real enemy appears, at
the row's position and facing **[V]**. The pose differs (the real enemy
starts its own animation); with a captured idle pose the jump is small.

## 5. Hits that wake a copy

TTK has no hitscan: every round is a projectile that dies on its first
sweep **[V]**.

| Attack | Path | Interception |
| --- | --- | --- |
| Rounds (pistol, shotgun, Gatling, crossbow, freezer) | sweep `0x8006dd2c` -> segment query `0x8006d980` over room lists +0x48/+0x4c; damage through `0x800c5ecc[actor+0x16]` | existing hook `0x8006dd2c`: test the segment against copies; on a hit create the real enemy, then let the original query find and damage it |
| Rockets | `0x80070568` sweep, then blast | same sweep hook; the blast does the damage |
| Blasts (rockets, grenades, pipe bombs, dynamite, barrels) | radius damage `0x800a9bf8`, one portal hop, line of sight | new entry hook `0x800a9bf8` (not hooked yet): wake copies inside R + r with a clear line, before the damage runs |
| Flames | `0x800a9ee0` each tick | new entry hook |
| Energy beam | target fixed at fire time (D22D lock) | add copies to the host lock query; wake on lock |
| Auto-aim (EDUKE32/HITSCAN) | host `weapon_aim.cpp` over +0x4c lists | add copies as candidates |
| Kick | damage sphere `0x800a979c` (hooked) | test copies in the same sphere |

Creating then damaging: the damage handler needs only the actor pointer and
a valid source (Duke). Unverified: whether a just-created actor already has
its model box +0x40 and joint matrices filled for the sweep's shape test;
fallback is to set the sweep's result and hit point ourselves.

At the cap the original replacement `0x80099048` removes the farthest live
enemy (D52 tie rule kept). If every live enemy is awake near Duke, nothing
gives way and the shot passes through the copy; level 0 has 12 slots, so
that needs 12 awake enemies at once.

Earlier D17AQ entry hooks were regenerated with the codegen hash unchanged;
the new hooks must be checked the same way so the player's savestates still
load.

## 6. Open items found

- `spawn enemy 26` answers "Spawn could not complete" in slot 9 (level 0),
  also with `DNTTK_ENEMY_REACH=0` (2 live). The console moves the row to Duke
  before creating, so the cause may be the moved row (room or position slot
  +0x2a). Handover creates at the row's own position like the original tick,
  but the prototype must check that host-called creation works here.
- Third area-damage routine `0x800aa83c`/`0x800aa6ec` has no direct caller
  found.
- D17AQ far dormant actors: once copies cover distant views they may no
  longer be needed (they spend real slots); decide after the prototype.

## 7. Proposed plan

1. Prototype, render only: stand-ins for eligible rows drawn through the
   object pass, template pose first, captured idle pose if the template is a
   bind pose. Check slot 9 monitors 2 and 3, a distant view, handover, 60
   and 180 Hz, cost.
2. Wake on rounds and rockets (existing sweep hook), then kick, beam lock and
   auto-aim (host code).
3. Blasts and flames (new entry hooks, savestate compatibility checked).
4. Revisit D17AQ dormant actors.

Modernized only; Vanilla and the "original" draw distance unchanged.

## 8. Prototype, step 1: drawing (2026-11-09, Needs playtest)

User approved the plan ("yes proceed"). `recomp/src/ttk/sleeping_enemies.inc`
(Modernized, Extended draw distance; included after `enemy_reach.inc`). No
new game hook, no regenerated code, no framework change; savestates load.

- **Stand-ins.** In the spawner tick `0x80099530` (live frames only) the 48
  nearest eligible rows get a stand-in in mod memory (0x2d0 actor + 0x5c0
  skeleton each), built by the creation's own type setup `0x80095504` with
  flag 0x10 held during the call (no global list), room `0x8005d790`, facing
  `0x8002abec` from the row quaternion, pitch and roll cleared as
  `0x80096614` does, static joint builder (+0x60 = 0xdc; developer
  `DNTTK_SLEEPING_ENEMIES=dynamic` keeps the look/aim builder). Types with
  flag bit 0 use the position slot (`0x800d2548`, a removed enemy's place),
  shown only while that slot is free.
- **Height.** Real enemies settle after creation: type 57 always at floor -
  326, type 26 at floor - 536..585 (animated), Necros (type 50, class 7)
  about 1100 above their row. The copy learns each type's resting offset
  from live enemies of that type (median of the last 31 samples; floor from
  actor +0x204, flyers from the row height); with no sample, floor - (+0x56
  - 24).
- **Table.** A mod-memory table (row, stand-in, room, distance) is written
  each tick; the redraw workers get it with guest memory, so 120/180 Hz
  frames draw the same copies. A row spawned meanwhile is skipped at draw.
- **Draw.** At the object pass entry, for rooms on the pass's list (the
  entry as rectangle) and, with host visibility, rooms the host shows beyond
  it (whole view `0x800d2210`, host guard-banded box test); nearest first,
  16 a frame; CPU copy with stack, scratchpad and render state
  `0x800d67a8..0x800d682f` restored; +0x5c set to the frame counter
  `0x800bcba4` (the renderer's once-a-frame routines skip).
- Counters: `sleeping` in the enemy reach debug JSON (draw distance).

Evidence (`recomp/analysis/d17bf/lab.py`, private copies of UI slots 9, 2
and 10, player files hash-checked every run):

- Slot 9 monitors: 23 rows eligible, 26 built (rebuilt after load), 0 build
  failures; 4 copies drawn on the monitor views, which are empty with
  `DNTTK_SLEEPING_ENEMIES=0` (`research/before-after/D17BF/monitors-before-after.png`,
  local). At 180 Hz the copy's pixels change smoothly as the monitor camera
  pans, with no frame where it drops out (24-shot burst).
- Handover (slot 2, 20 spawns logged): the real enemy appears at the copy's
  x/z and facing in every case; height matches to 0 for type 57 and to 17
  for Necros once a sample exists (24 before the fallback fix).
- Cost: about 0.45 ms of emulation thread per visible copy, about 60 us for
  one the renderer culls; slot 9 monitors (4 copies) about 50 ms/s; fields
  60/s with and without. Update about 13 us per tick.
- Ordinary view at the slot 9 start: pixel-identical with copies on and off.
  Slot 2 still view: one copy drawn but hidden behind walls (only the HUD
  timer differs).
- Vanilla (`--vanilla`): nothing built or drawn.
- `ttk-near-test`, `ttk-aim-test`, `ttk-input-test` pass;
  `ttk-controls-test` does not link (`gpu_vram_peek` from
  `level_lights.inc`, D41D1, before this change).

Limits: hits do not wake copies yet (step 2). The rest pose is the type's
skeleton template, accepted from shots, not compared with the live idle
pose. A distant view where a copy is visibly on screen was not found in the
fixtures (slot 10 now holds a ground save); the host-room path drew copies
(counter) but they were not identified on screen. Whether the two monitor
views reached here are the user's "monitors 2 and 3" is not established.

## 9. Playtest and slot churn fix (2026-11-09)

User: monitors "absolutely perfect". Slot 2 (watched read-only): the
distant static Necro was a live D17AQ dormant enemy; two rockets woke and
killed it. With one slot then free, rows 28 (Necro) and 38 (type 57) traded
the last slot every 19 frames: the D52 tie rule compared the live enemy's
position with the new row's, and a Necro hovers about 1100 above its row,
while a type 57 settles below its row. Now the enemy that would give way
counts at the nearer of its position and its row's place: if V gives way to
S, min(dV) > dS; the reverse needs min(dS) > dV(row) >= min(dV), which
cannot hold. Offscreen slots 2 and 9: no spawner change in 6 s, cap kept.
User repeat on the fixed build: after the kill the slot went to rows 18,
23, 29 within 10 frames (each nearer), then held for 975 frames until the
game was closed.
