# D08Z2 - Mantle pose regression since D08Z1

Status: **Done** (2026-10-08, user-accepted after revision 1: "accepted!!"). Modernized only; Vanilla never
reaches the changed code.

## User report (2026-10-08, at D08Z1 acceptance)

"this has also introduced a regression where his mantling animation is now
some static pose with his legs and looks weird."

## Reproduction

Private copies of the D08X/D08Y savestates and blank private cards
(`recomp/analysis/d08z2-mantle-pose`, Xvfb, dummy audio), with a private copy
of the player's settings (first person, manual jump, 120 fps, camera distance
1536). Every frame of the animation blocks `+0x60..+0x100` was recorded with
screenshots, for `jump_walls slide` and `original` on the same executable.

| Route | `slide` | `original` |
| --- | --- | --- |
| Crate A E jump mantle, eight takeoff timings (slot 11), holstered | 135/136/137/139+140, full body | the same |
| Slot-12 wall, W+E+Space: grab 148, pull-up 140 | full body | the same |
| Slot-5 gap, E held (lowered catch, 134) | full body | the same |
| Slot-5 gap, no E, **weapon drawn**, pressed 86600 / 86750 | wall slide, then low-lip step-up **134 on the legs only** | 86600: bounce 107, fall (first person); third person: 107, then the same legs-only 134 |

Animation state (`+0x60..+0x6a`) matched frame for frame between the modes in
every route where both mantled, so the D08Z1 safety net and the slide do not
leave a stale lower-body pose. What differs is the upper track (`+0x74`): in the
armed step-up it stayed on **20**, the weapon-ready rest pose, for the whole
mantle. The legs climbed while the torso and arms held the gun pose still.

## Cause

The original only ever starts a mantle from the ground obstacle action
`0x80051cf0`, which refuses (returns 0) while the upper animation carries
upper-table bit 8 (`0x800c2824 + 4 * upper`, the weapon-owned poses such as 20
= `0x200c`). So in the original a mantle only sets the lower animation
(`+0x60`, `+0x68/+0x6a` = 0; `0x80051fe0`). The upper track then follows it,
because a free upper body takes the lower animation. The animation start
`0x800593b4` never touches the other track.

The D08Y low-lip step-up (`ledge_step_up`, no E, armed or not) has no such
gate, so an armed scramble played 134 on the legs while the weapon pose kept
the upper track. Before D08Z1 it only started from the first updates of a 107
bounce off the lip. D08Z1 starts it from every wall slide, so a first-person
player with a weapon drawn now meets it on most jumps that touch a low lip.
The E mantles (crate, lowered catch, lifted catch) already require a stowed
weapon and a free upper body, so they always played full body.

## Change

`recomp/src/ttk/ledge_reach.inc`:

- `jump_mantle_start(climb, feet, rel)` is now the one place that starts an
  original height mantle from flight (D08X crate jump mantle, D08Y lowered
  catch, D08Y step-up). It does what the three copies did (floor `+0x1c8`,
  ledge height `+0x1c4`, velocity zeroed, mode 0, lower animation restarted).
- If the upper track holds a weapon-owned pose (bit 8), it is restarted on the
  same mantle (`+0x74` = climb, `+0x7c/+0x7e` = 0), as a holstered mantle plays
  it. Its whole 0x14-byte block is kept.
- `mantle_upper_restore()`, once per player update after the mantle helpers:
  when the lower animation leaves 134..140 (139 chains into 140), the kept
  weapon block is written back, unless a savestate was loaded, the equipment
  byte `+0x3b8` changed, or the upper track already holds a weapon-owned pose
  again. Without this, the upper body followed the legs after the mantle with
  the weapon lowered, and the fire button did nothing (measured: 63/72/74 on
  the upper track, Mouse1 ignored).

Diagnostics: `ttk_controls` `mantle_upper_handoffs`, `mantle_upper_restores`.
No new hook, no generated code or hashed header change (savestates load).

## Evidence

Executable `9de8da617862f53e545ac3c8be0f23623979fea2265944e2f3cdfaa3ec3f96b0`.

- Slot-5 gap, no E, armed, slide, pressed 86600 and 86750: upper = 134 for the
  whole mantle (screenshots: the arms reach for the lip and push up). After it
  the upper is 20 again one update after the lower returns to 63; walking keeps
  20; Mouse1 fires (24, 25, 26, then 23, 24, 20), as on a fresh load.
- D08Z1 regression routes with `slide` (`regress.sh`, the same 12 routes as
  D08Z1), compared with the D08Z1 run: the same outcomes. Slot-12 wall grab
  148 and pull-up 140; holstered E jumps and the pit jump catch (6/6), the
  tall wall never grabs; crate-to-crate 8/8 on top; crate A E mantles; D08U
  ladder top and furniture run-offs unchanged; every slot-5 gap case lands
  across or mantles in both jump styles, with the same misses (manual no-E
  pressed 1000 early, assisted fall). Run-to-run variation as before: crate B
  (one 149 hang more or less), the D08X -10 degree angle (caught this time),
  side-jump distances.
- Native: `ttk-controls-test` 45 groups; the D08Y group now covers the armed
  handoff (upper 134, restarted), the mantle in progress, the exact block
  restore and a single restore. `ttk-input-test`, `ttk-aim-test` pass. Python
  136 OK (2 skipped).

## Limits

- After an armed mantle the upper body shows the idle pose for one game update
  (1/30 s) before the weapon pose comes back, because the original switches
  both tracks to idle in the update the mantle ends and the restore runs at the
  start of the next one.
- The fix also applies with `jump_walls original`. There, the armed step-up from
  a 107 bounce had the same legs-only pose; it now plays full body too.
- The weapon stays in Duke's hand during the armed mantle (the original never
  shows this combination).
- Not verified: the user's own view of mantles in play.

## Revision 1: legs frozen after a corner bump

User (2026-10-08, first D08Z2 build): "duke's animation is stuck in that pose
after hitting the side of the statue in slot 8, in such a way that it causes
the "bump" noise ... Then check out slot 6 ... duke's lower half gets
completely locked in that pose no matter what you're doing".

Private copies of the player's new UI slots 6 and 8 (runtime 5 and 7). In UI
slot 6, in third person, the legs stay straight and still while idling,
walking and running, although the lower animation advances normally (63, 72,
78). The cause is the third animation track (`+0x88`, 0x14 bytes like the
lower `+0x60` and upper `+0x74`). Each track keeps a cumulative joint mask at
block `+0xc` that `0x80058c70` only ever ORs into. Normally track 3's mask is
`0x1c00`; in the saved state it was `0x1cef`. So track 3 owned the leg joints,
had no leg keys in idle, walk or run, and the legs kept one stale pose. A
savestate keeps that.

Reproduced at the old slot-6 monument corner (private copy): one D08Z1 safety
net catch (the bump sound) left `0x1cef` for good; the original bounce (`jump_walls
original`, the same headings) never touched the mask. The safety net undid
the bounce's 107 at the next animation runner call. By then the player update
had already started 107 on the tracks (lower state 0 -> start callback and
`0x800593b4`), and track 3 had taken 107's leg joints. This is a D08Z1 defect
(revision 1, the safety net), not the D08Z2 upper-body handoff. The handoff
traces keep every mask as the original's.

Change: the 107 is now undone at the entry of the contact sound
`0x8006bbd8(0x2008)` (ra `0x80055a10`), which the handler calls right after
writing 107 and before anything starts it. That entry is a new opt-in hook
(`game.local.toml` `mod_function_entry_funcs`, regenerated with
`psxrecomp_cli.py generate`: one generated line; codegen hash `0x8bab543c`
unchanged, savestates load). The runner-call undo is gone. Counter
`jump_walls.undos`.

Evidence (executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986`):

- Monument corner, nine headings, running, first person: three safety-net
  catches, three undos, no 107 on any track, track-3 mask `0x1c00` after every
  jump (before: `0x1cef` after the first catch). Walking afterwards in third
  person, the legs stride.
- Native 45 groups (the D08Z1 group now checks that the undo happens at the
  sound call from the bounce only, once, and not at a runner call). Input and
  aim tests pass, Python 136 OK.

Limit: a savestate made while the legs were frozen (the player's new UI slot 6)
keeps the polluted mask; nothing in normal play clears it. Load an earlier
state, or the level start, instead. The rumble and the bump sound of a caught
corner still play.
