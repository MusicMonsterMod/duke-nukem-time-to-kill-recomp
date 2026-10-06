# Typed debugging cheats — D08G

2026-09-27. Implemented for Modernized single-player gameplay; the user later
accepted the delivered functionality. Broader-era coverage remains outstanding,
and requested message/font polish is queued separately (see latest section). The user's furniture follow-up remains
open in [D08B notes](37-furniture-traversal.md); this iteration changes cheat entry,
not furniture physics or the light-switch target.

## Player use

Stand on solid ground in captured Modernized gameplay and type a complete code.
No console or Enter is required. After `dn`, an on-screen message shows the input
and typing consumes the remaining letters instead of activating their shortcuts.
The first D still acts as the ordinary movement key, so start away from an edge.
Escape/F10/focus loss cancels entry; a three-second typing gap expires it. Codes
are case-insensitive physical letter keys, consistent with the keyboard bindings.
Vanilla, menus, dead players, attached traversal, unsupported code and multiplayer
cannot execute these commands. A rejected request is discarded, never deferred
until the player later lands or enters gameplay.

| Code | Time to Kill effect |
| --- | --- |
| `dnmonsters` | Hide/show hostile AI; NPCs, pickups, switches and other entities stay present |
| `dnkroz`, `dncornholio` | Toggle original invulnerability; while on, health is at least 100 and an owned jetpack stays full (D08Q4) |
| `dnstuff` | Original all-weapons/ammo, inventory and keys grants |
| `dnweapons` | Original all-weapons/ammo grant |
| `dninventory` | Original inventory grant and charges |
| `dnitems` | Original inventory plus keys |
| `dnkeys` | Original key inventory grant |
| `dnhyper` | Activate/refill the original steroid timer |
| `dnammo` | Refill ammunition for already-owned weapons |
| `dnhealth` | Restore living Duke to 100 health |
| `dnunlimited` | Toggle original unlimited ammo/charge mode |
| `dnupgrade` | Original weapon upgrades for weapons 4/5/7/8/9/10 and the persistent mask (D08G3) |

These are TTK equivalents: `dnstuff` does not promise Duke 3D items such as HoloDuke,
and `dnitems`
does not add a separate armor grant. The last three
spellings are explicit debugging additions, not claims of original Duke 3D codes.
`dngod` (World Tour alias), `dnclip`, `dnscotty#**`, `dnunlock`, `dnskill#`,
`dnshowmap`, `dnview`, `dnrate`, `dndebug`, `dncoords`, `dncashman`, `dnbeta`,
`dntodd` and `dnallen` are not implemented. Their original-game contracts or equivalents
have not been verified in this build.

Use a separate test save for cheat-assisted play. Inventory grants alter ordinary
game state. Enemy hide/show ownership is session/scene-local: show enemies again
before saving or loading a state. This feature has not established save/load
round-trip support for hidden enemies. Turning enemies back on resumes the original
activation/animation machinery; it is not a frozen, frame-exact enemy snapshot.

## Reference and owned-game proof

The user also supplied the [original Duke 3D cheat list](https://dukenukem.fandom.com/wiki/Cheat_codes#Duke_Nukem_3D)
and its text. That page could not be fetched (HTTP 402); the supplied list is retained
as the requested reference, with the differences and omissions listed above.

The [EDuke32 cheat table](https://wiki.eduke32.com/wiki/Definecheat) establishes the
familiar names/effects; [Cheatkeys](https://wiki.eduke32.com/wiki/Cheatkeys) describes
the default DN prefix. Local primary source:
`research/eduke32/source/duke3d/src/cheats.cpp`, revision
`ec5824db81817866f70da326d3811bb0f52b3517` (official Voidpoint checkout).
EDuke32's monsters code cycles three states; this requested TTK adaptation is a
simple hide/show toggle for the original hostile handler class.

Read-only decoding of owned SLUS-00583 establishes:

- Original cheat dispatcher `800839a8..80083a1c` toggles unlimited ammo at
  `800c3cc4` and invulnerability at `800c3cc6` and invokes the grants below.
- `8003d7bc` grants weapon records0..35, preserving flags and reading capacities
  from the live44-byte records at `800c4590+4`.
- `8003d738` grants items0..5 and charges from the live item capacity table.
  `8003d840` grants key/item flags6..16. All three iterate original player count;
  the adapter requires exactly one player before calling them.
- The original steroid pickup at `800827e8` sets item4 active bit2 and copies
  `800c2722` into player+366. `dnhealth` uses the previously verified 10000 raw
  health value for HUD100, rejecting dead/unsupported players.
- Enemy records are48 bytes, base `800de720`, count `800c56a0`. Object definitions
  are28 bytes at the pointer in `800d2660`; byte6 identifies the handler.
  First-level hostile types26/57 use handler6. NPC types123/167 use handler8;
  other classes (including type204/handler9) are excluded.
- `80098d70` / `80098f3c` deactivate/reactivate an enemy through original list,
  pool and active-count bookkeeping. The update loop `80099534` skips records
  with flag16. Hiding prevalidates candidates, excludes dead and scripted-disabled
  records, stores live health/position, deactivates active hostiles, then holds
  selected records dormant. Showing clears only the owned dormant bit, restores
  previously active actors and health, and leaves originally dormant actors to
  ordinary proximity activation. NPCs and original disabled/dead rows are untouched.
- The existing scene-teardown entry `8002b9f4` clears host enemy ownership at its
  verified original callers before addresses can be recycled. No generated C edit
  or new recompiler hook address is required.

The existing Modernized code/overlay/player/context lease authenticates execution.
Additional SHA guards cover original grants, toggle/steroid paths and enemy manager.
Original calls use a copied CPU and restored private stack/scratchpad. A RAM-bounded
record/type validation pass precedes enemy writes. This is not an all-entity flag sweep.

`cheat_codes.h` holds the bounded parser/table; `pc_input.cpp` owns input/expiry;
`cheats.inc` handles original-game actions. The already implemented runtime host OSD
is enabled for this title through a source-only compile definition and preserved
`time-to-kill-zzz-cheat-osd.patch`. The patch-stack verifier passes. The runtime
submodule's earlier unrelated modifications remain intact.

## Verification and limits

Final binary: `6efa111e9e838a00273aaef454ac3130563a49c1e03f9e84eb314d8e37e519b6`.
57 Python tests (including owned-disc integration), four native suites and the
14-checkpoint Vanilla replay pass. Captures reviewed, private sessions exited,
and player settings/cards are unchanged. Final live hostile counts were 9 → 0 → 9
with three NPCs unchanged; a shot enemy retained 2813 health after reactivation.
Unlimited firing preserved 200 rounds, then ordinary firing reduced ammo to 192.
God mode and health refill held health at 10000 during the subsequent live wait.


See [structured delivery evidence](reports/d08g-cheats.json) for the final binary,
source hashes, tests, observations and rejected harness attempts. Native tests
exercise every spelling, once-only delivery, ordinary D, shortcut suppression,
focus cancellation, grant-call order, toggle flags, unsupported player count,
NPC/dead/disabled separation, health restoration and scene ownership reset.
They model enemy routines; actual list/pool behavior is checked separately in the
private live game. First-level evidence is not a full campaign or boss guarantee.

The user's latest report confirms natural pipe-bomb collection after opening the
bed and no early pickup on that attempt; no pickup fix is claimed. Close-contact
bed entry, the residual running-drop slowdown and switch targeting stay open.

Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized captures automatically; F10 toggles capture.

## User acceptance and requested presentation follow-up

2026-09-27: user says the cheats work fine. D08G is Done for the bounded delivered
functionality; previously documented save/load and campaign limits still apply.
D08G1 requests exact Duke3D confirmation strings; D19A requests the supplied Duke
fonts. Neither presentation change has been implemented at this planning checkpoint.
See [next-session brief](39-next-iteration-brief.md).

## D08G3 `dnupgrade` and D08Q4 `dnkroz` additions (2026-10-06, accepted)

- `dnupgrade` ("Weapons Upgraded") sets the original upgrade bit `0x8` on
  weapons 4, 5, 7, 8, 9 and 10. It also ORs `0x3f` into the persistent mask
  `player+0x85f`, which challenge stages 21-26 set, saves store, and load,
  restart and pickups reapply. `8003df40` resolves 7/8/9 to 28/29/27, which
  read their own ammo. For owned 7/8/9 the upgraded record is marked owned
  and given at least the base ammo, capped at its capacity; otherwise the
  weapon counts as empty. Unowned weapons are not granted; they arrive
  upgraded. The original's unused dispatcher entry `8003f5cc` sets the bits
  without the mask, so it is not called.
- God mode: `god_mode_update()` raises health to the type maximum
  (`types[+0x2c]+0xc`) and never lowers it. Atomic Health (`80096cac` with
  `over` = 1) keeps its original ceiling of twice the maximum. It also holds
  an owned jetpack at the item capacity `800c2716`. It runs at the
  `8005a210` entry and after the mode-10 drain, from the `80058120` poll, so
  the HUD stays at 100%. The flag persists through death, Continue and level
  changes. Unlimited ammo `800c3cc4` is not used. EDuke32 pins health at
  exactly the maximum, which would discard an Atomic surplus; this follows
  the user's description instead.
- Guards added: `8003df40` (112 bytes), `80096cac` (152 bytes). Evidence and
  limits are in the 2026-10-06 work-log entry in `MODERNIZATION_JOBS.md`.

