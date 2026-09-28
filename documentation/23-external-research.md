# External research register — 2026-09-26

## EDuke32 control reference (current controls pass)

User selected [EDuke32](https://www.eduke32.com/) as the control reference.
Official [player.cpp](https://voidpoint.io/terminx/eduke32/-/blob/master/source/duke3d/src/player.cpp)
retrieved today uses XOR between autorun and Run in its default run-key mode;
its alternate mode uses OR; [official config.cpp](https://voidpoint.io/terminx/eduke32/-/blob/master/source/duke3d/src/config.cpp) sets runkey_mode to 0. The [console documentation](https://wiki.eduke32.com/wiki/Console_commands)
lists independent autorun, run mode and autoaim controls. Our selected contract:
walk on launch, Shift temporarily reverses speed, Caps Lock toggles autorun.
Walk by default is the user's explicit preference, not a claim about EDuke32's
autorun default. Preserve saved action bindings, camera-relative strafe/backpedal
and independent pitch. TTK's red marker is original autoaim, not a laser sight.
A later assisted-view-aim setting needs weapon-specific research; the existing
Original weapon aiming option remains available. EDuke32 source was read as a reference; no EDuke32 code was incorporated into the player build.

## HRP (D16A, later)

[HRP](https://hrp.duke4.net/) is built for Duke Nukem 3D/EDuke32. Its site describes
replacement models and high-resolution textures. The 5.5 release notes list
widescreen status/menu assets and revised weapon alignment. This makes it a
candidate for first-person weapons first, then interface, overlays and optional
model replacements; compatibility with TTK is unproven.

The [about/credits page](https://hrp.duke4.net/about.php) distinguishes art licensing
from engine/port licensing. Later research must inspect actual asset files,
credits, art terms, formats, animations and mapping to TTK before deciding reuse.
No pack installed or asset imported. D12/D16/D16A must preserve optional original
presentation. D13 resolution and D14 widescreen are independent graphics jobs.

Additional research requests can be appended here with source, scope, job link,
findings and unresolved questions; this register does not authorize implementation.

## Modern in-game menus (D19, later)

User requests an EDuke32-inspired in-game menu system, including investigation of
open-source code reuse. The [official repository](https://voidpoint.io/terminx/eduke32)
is the primary source. Research navigation, settings grouping, input prompts,
scaling and pause/resume, then inspect candidate code and its license/dependencies.
Record whether adaptation is practical for this recomp or whether a local menu
implementation is preferable. No source adoption or implementation yet.

## Aiming configuration clarification

The user requests eventual EDuke32-style configurability with independent aim
assistance, crosshair visibility and original red-dot visibility, including no
red dot. D07B owns behavior and persistence; D19 exposes the settings in the
modern in-game menus. Hiding either marker must leave the selected aiming
behavior unchanged. This is a requested configuration contract, not evidence
that the current player implements these switches.


## Local EDuke32 configuration inspected during D07 continuation

The desktop launcher resolves to
`/home/spartacus/Games/build-eduke32-vanilla/eduke32.exe` through the existing
Bottles container. Read-only inspection of its `settings.cfg` confirms E Open,
WASD movement, LShift/RShift Run, CapLck AutoRun, and 1–9/0 Weapon_1..Weapon_10.
Its mouse wheel bindings are previous/next weapon. This is evidence of this local
installation's mappings, not a claim about every EDuke32 release or its defaults.
The workspace `DUKE3D.GRP`, that installation, configs and saved games were not
changed or launched. No EDuke32 implementation code/assets were imported. TTK's
numbered requests use TTK inventory and original equipment transitions.

## D08A local reference checkout and inventory comparison

On 2026-09-26, at the user's request, cloned the official EDuke32 repository to
`research/eduke32/` at `ec5824db81817866f70da326d3811bb0f52b3517`. See
[provenance](../research/README.md) and [the complete proposal](32-eduke32-weapon-item-plan.md).
This supersedes older statements that no source checkout is present. No EDuke32
code/assets were incorporated into the player build; neither game was launched.
The proposal distinguishes actual local/upstream bindings, manual-documented TTK
roles, proposed substitutes, and unverified executable behavior.
