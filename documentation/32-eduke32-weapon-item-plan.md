# D08A research: Duke 3D weapons, items and modern controls

## Historical proposal (superseded by authorized implementation)

See [the verified controls implementation](33-controls-shortcuts.md) for executable
identities, delivered bindings and the steroids/quick-kick limitations. The user
subsequently authorized the proposed groups, B/U and bounded D10 distance.

2026-09-26. Research and proposal only; no player implementation, build, launch,
settings migration or gameplay verification. D08A remains Todo. This records the
user's planning discussion after D08 acceptance, including quick held crouch and
Alt + wheel camera distance. Existing D07A/B playtest gaps remain.

## Evidence and boundaries

An upstream EDuke32 checkout is now in `research/eduke32/`, pinned to
`ec5824db81817866f70da326d3811bb0f52b3517`. Provenance and config hashes are in
[research/README.md](../research/README.md). The local installed configuration
was compared with upstream defaults; it is not assumed to be the same version.

Primary code references in that checkout:

- `source/duke3d/src/inv.h`: weapon and inventory identities.
- `source/duke3d/src/_functio.h:267`: modern default key table; `:401`: mouse defaults.
- `source/duke3d/src/function.h`: matching action order.
- `source/duke3d/src/player.cpp:3360`: numbered/previous/next/last weapon input;
  `:3391`: separate held and toggle crouch; `:3595`: quick-kick attack timing.
- `source/duke3d/src/sector.cpp:2611`: empty-ammo remote-bomb exception;
  `:2683`: independent quick kick; `:2845`: weapon resolution, subweapons and cycling.
- `source/build/src/sdlayer.cpp:2719`: wheel direction to mouse button mapping.

The US TTK manual's Weapons and Gadgets spreads (printed pages 10–15) were
visually reviewed. It documents selectable boot attacks, throwable weapons,
inventory gadgets and contextual key/crystal use. This establishes original
player-facing roles, not executable slot IDs or a safe hook. Existing TTK
`select_weapon()` only admits slots 4–13 and checks positive ammo: it is not yet
a complete weapon selector and cannot be reused unchanged for boot/detonator.

## Proposed complete weapon mapping

Duke 3D reference is the classic/Atomic roster. EDuke32 also has a World Tour
flamethrower, conditionally paired with the freezer key; it is not a classic
Atomic weapon. The TTK assignments below are proposals, not current bindings.
All 15 base TTK weapons have a place. Actual era availability and upgrade IDs
must be checked against the owned game's tables and original selection behavior.

| Key | Duke 3D role | Proposed TTK weapon/group | Match |
| --- | --- | --- | --- |
| 1 | Mighty Foot | Mighty Boot; Throwing Knife; Throwing Axe | Boot is direct; throwables are a proposed low-tech group |
| 2 | Pistol | Desert Eagle | Direct sidearm role |
| 3 | Shotgun | Combat Shotgun | Direct |
| 4 | Chaingun | Gatling Gun | Direct automatic-fire role |
| 5 | RPG | RPG | Direct |
| 6 | Pipe bombs / remote | Pipe Bomb; Dynamite; Holy Hand Grenade | Pipe bomb direct; other throwables grouped, retaining their own fuse/trigger rules |
| 7 | Shrinker / Atomic Expander | Energy Weapon | Proposed alien-energy slot; does not shrink or grow enemies |
| 8 | Devastator | Flamethrower | Spare-key assignment; no TTK dual-rocket equivalent |
| 9 | Laser Tripbomb | Buffalo Rifle; Crossbow | Spare-key assignment for precision weapons; no TTK tripmine equivalent |
| 0 | Freezer | Freezer | Direct |

No equivalent is invented for Shrinker, Expander, Devastator or Tripbomb behavior.
TTK's flamethrower does have a broad role match in World Tour, but using 8 keeps
0 dedicated to the classic freezer while making TTK's distinct weapon accessible.
Weapon upgrades should stay with the corresponding base key, without separate
wheel entries when they replace that weapon. Exact upgrade/name relationships
remain to be verified in TTK; do not infer them from this grouping.

Proposed selection rules:

1. Pressing a group from another group chooses its canonical available weapon:
   boot on 1 and pipe bomb on 6 when usable. If absent, choose an available
   listed alternative. Pressing again within the group advances through its
   usable members. A held number key must not repeatedly cycle.
2. Wheel up selects previous, wheel down next; wrap in displayed group/member
   order. Skip unavailable or unusable entries, but preserve a valid remote
   detonator when live pipe bombs exist even if spare ammo is zero. Investigate
   TTK's equivalent state; EDuke32 source explicitly handles this exception.
3. Selection never fires, throws, detonates or grants anything. A group change
   during a charged throw must respect the original weapon state.
4. X restores the last successfully equipped weapon, not the last requested
   slot. Semicolon/apostrophe provide previous/next keyboard alternatives.
5. Q always requests a boot attack, regardless of which low-tech member is
   selected. TTK quick-kick feasibility is separate from the confirmed boot.

## Items and inventory proposal

The installed EDuke32 config and upstream table agree on M, R, J, N, H,
brackets and Enter. Preserve familiar actions where TTK has a corresponding
item. These are proposed native shortcuts; TTK's own use rules remain authoritative.

| Duke 3D item/action | Reference input | TTK counterpart and proposal |
| --- | --- | --- |
| Portable medkit | M; middle mouse also defaults to MedKit | Portable medkit: M. Middle-click remains optional rather than silently added |
| Steroids | R | Steroids: R, through original inventory activation |
| Jetpack | J | Jet Pack: J for its verified equip/use path; retain TTK's jump-held thrust behavior |
| Night vision | N | Goggles: N toggles original use |
| HoloDuke | H | No listed TTK equivalent; leave H unassigned unless customized |
| Scuba gear | Automatic underwater use | No TTK scuba gadget and none will be added (D08N cancelled). Bio Mask protects against gas, not underwater breathing |
| Protective boots | Automatic hazard protection | No listed matching TTK gadget; do not confuse these with the Mighty Boot weapon |
| Inventory previous/next | [ / ] | Cycle eligible TTK inventory items without using them |
| Use selected inventory | Enter / keypad Enter | Propose U; Enter already belongs to Start/pause. I continues opening original inventory |
| Access cards | Contextual use | Keys/cards: retain E at the relevant panel |
| Health/Atomic Health, armor, ammo | Pickup | Preserve TTK pickup behavior; no consumable shortcut for ordinary pickups |
| No exact Duke 3D counterpart | — | Bio Mask: propose B toggle |
| No exact Duke 3D counterpart | — | Crystals: retain E at the appropriate receptacle |

Manual evidence distinguishes selectable steroids from ordinary health pickups.
The portable medkit's live inventory identity/use path must be distinguished
from the manual's small/large health pickups before implementing M. Full health,
depletion, item ownership and repeated presses need checks; no healing rule changes.
Jetpack equip/activation is still a research boundary, not a confirmed J toggle.

## Agreed controls and conflicts

User-confirmed: hold Ctrl to crouch, release to stand, with quick entry/exit;
wheel cycles weapons; Alt + wheel changes camera distance. This is not toggle
crouch. Keep C holster, E interaction-only, Space jump, Shift speed reversal,
Caps Lock autorun, Escape/Enter Start and F10 capture. Right-click retains original
precision aim. Do not copy EDuke32's C toggle-crouch, Enter inventory, Alt strafe
or right-Ctrl fire over these decisions.

The current TTK action default is V for original crouch/quick-turn, while the
fallback key list includes Z. The user reports holding Z. Inspect saved binding
and fallback routing during implementation; remove reliance on that combined
path for Modernized Ctrl crouch while preserving Vanilla and intentional custom
bindings. Quick crouch requires identifying stance/animation/collision ownership,
not just mapping Ctrl to the old PS1 button. Measure key-to-stance latency before
and after; preserve low-ceiling clearance, pause/focus release and original
fallbacks in unsupported traversal states. Do not accelerate the whole simulation.

The current input-token allowlist does not admit all proposed keys (Ctrl, Alt,
brackets and X). Extend and verify the binding editor, serialization and
runtime consistently rather than assuming these are existing rebindable actions.

Alt + wheel belongs to D10 camera distance: up closer, down farther, clamped and
smoothed, with wall collision and a separate preferred distance. Keep FOV unchanged.
Before D10 exists, reserve/consume Alt + wheel so it does not switch weapons;
do not advertise working zoom. Discard queued wheel events on focus/capture/menu
changes. Handle high-resolution wheels and multiple notches without delayed
weapon changes after capture returns. Alt must not trigger a fallback action.

## Quick kick: what is established

EDuke32's Q sets an independent quick-kick timer, and later calls the knee attack
without selecting the boot as the current weapon. The TTK manual instead documents
selecting Mighty Boot and pressing Attack. No equivalent independent TTK shortcut
has been established by this research.

Trace the original kick state, hit registration and interruptions. Prefer its
original attack path. If TTK requires a temporary selection, verify original
transition timing and restore only the prior valid weapon; a new manual weapon
choice, death or lost ownership must cancel restoration. Do not emulate Q with a
blind select/fire/restore macro that could fire a gun or detonate an explosive.
Do not promise concurrent kicking and shooting until the TTK state machine proves it.

## Implementation order and acceptance

1. D08A: verified complete weapon/item identities, number groups, wheel cycling,
   last weapon, direct item use and Q feasibility. Include the user's requested
   Ctrl crouch follow-up in the next controls pass, with its own animation and
   collision evidence rather than hiding it inside a binding migration.
2. Preserve D08B broader traversal and D09 controller scope and order.
3. D10 implements Alt + wheel distance; its input is reserved in the earlier pass.

The exact surrogate groups (7–9), multi-member key behavior and B/U assignments
are recommendations for review. They are not user sign-off or implemented facts.
Binding migration must preserve custom choices and explicitly migrate old defaults.

Test original equipment/item requests using isolated cards/preferences: natural
ownership, absent/empty items, full health, last live pipe bomb, depleted charge,
upgraded weapons, era alternatives, repeated keys, wheel bursts, kick interruption,
Ctrl release under low ceilings, focus/menu isolation, rebinding conflicts and
Vanilla regression. Record synthetic fixtures separately from natural playtests.
User feel acceptance remains required. No gameplay claims arise from this research.
