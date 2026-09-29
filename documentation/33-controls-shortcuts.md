# D08A, held crouch and bounded D10 distance

2026-09-27. Authorized scope: semantic weapon/item shortcuts, responsive held
crouch, and Alt-wheel distance only. Human acceptance remains Needs playtest.
D08's accepted movement/traversal is retained; broader D10 is not complete.

## Executable evidence

Read-only SLUS-00583 decoding: the 44-byte weapon records at `800c4590` contain
string IDs at +2, resolved through `800c60f4`. These establish the following IDs,
independently of the manual's order. Original inventory builds entries 0–14 at
`800886d8`; `8003df40` resolves upgrades; `80088074` admits zero-ammo slot 12 when
player+254 (live pipe-bomb count) is positive. No selector fires or grants
inventory. Original draw/model/equipment handling remains responsible for success.
The usable detonator is equipment 1 / drawn ID 0 / selected ID 12
(`8004e688`); selectors and history treat that verified state as pipe bombs,
including switching away from the remote. Other equipment-1 gadgets are excluded.

| Key | Executable base slots | Weapons |
| --- | --- | --- |
| 1 | 0, 1, 2 | Mighty Boot, Throwing Knife, Throwing Axe |
| 2 | 4 | Desert Eagle |
| 3 | 5 | Combat Shotgun |
| 4 | 7 | Gatling Gun |
| 5 | 8 | RPG |
| 6 | 12, 14, 13 | Pipe Bomb, Dynamite, Holy Hand Grenade |
| 7 | 10 | Energy Weapon |
| 8 | 9 | Flamethrower |
| 9 | 6, 3 | Buffalo Rifle, Crossbow |
| 0 | 11 | Freezer |

Inventory flag 8 resolves 7→28 (Laser Gatling), 8→29 (Incendiary RPG),
9→27 (HiTemp Flamethrower), with ammo from the resolved record. Other original
upgrade behavior stays with its base weapon. Upgrades have no extra wheel entry.
Era-specific ownership is authoritative; no substitute is granted. 7–9 are the
user-authorized surrogate groups, not claims of Shrinker/Devastator/Tripbomb effects.

A group pressed from outside starts at its first usable member; another press
inside advances. Wheel up/semicolon go previous, down/apostrophe next in table
order. X tracks observed successful equipment, not attempted selections. Equipment
transitions, attacks, precision aim, interactions and unsupported ownership reject
requests rather than replaying later. Lit dynamite is an original exception: `8004dea4` / `8004e018` run its fuse
until thrown or detonated; a requested next weapon waits for that original
completion. Shortcuts neither stow it artificially nor throw it automatically.
Original settled upper-body idle loops
5/20/29/35/39 authorize switching; draw and charged-throw poses do not. A pending burst is bounded to 32 steps,
resolved to one final equipment request; fractional wheel amounts accumulate until
a full notch. Commands survive host polls until the next guest update explicitly
accepts or rejects them, with an eight-tick expiry when no consumer runs. This
avoids losing taps between host vblanks and slower game updates. Capture/focus/menu
loss clears the queue and fractional remainder.
The original mode dispatcher (`8001bb84`, table `800bcbb4`) distinguishes
frontend/attract mode 0 from game mode 1. Only game mode with original pause/menu
flags clear can offer capture or authorize control writes; a demo actor behind
the difficulty menu must not capture X. This fixes a regression exposed by SDL replay.

## Gadgets and deliberate exceptions

Original inventory items are player+354+4×ID (flags) and +356+4×ID (charge).
`80087d4c` resolves item names, `80088134` checks inventory eligibility, and
`80089428` confirms original requests. `800402d0` owns activation and medkit healing. The item capacity table at
`800c2712` changes after boot: confirmation follows its live value, as the original
menu does. Immutable guards cover code; live item-type fields are checked separately.

| Input | Item ID | Behavior |
| --- | --- | --- |
| M | 5 | Original portable-medkit equipment/use request; no health writes |
| J | 1 | Original jetpack activation/deactivation; retain jump-held thrust |
| B | 2 | Original Bio Mask toggle/equipment path |
| N | 3 | Original goggles toggle/equipment path |
| [ / ] | 5, 1, 2, 3 | Cycle owned eligible gadgets without activation |
| U | selected | Use selected eligible gadget; initial selection is medkit |
| R | 4 | No stored-dose shortcut: original steroids activate on pickup |
| Q | 0 | Original boot kick only when already selected, standing still; inactive while armed |

Steroids are a concrete exception to the manual-based proposal: `800827e8` sets
item 4 active and its timer immediately on pickup. `80087da8` supplies the empty
inventory label, and `800414a0` drains the active timer. R does not invent a dose
or disable/restart this effect. Regular health/armor/ammo pickups retain their rules.
Keys/cards/crystals remain contextual E actions. No scuba or HoloDuke
equivalents; adding a scuba gadget is out of scope (D08N cancelled).
Gadget shortcuts currently share the supported ground/crouch state lease; airborne
jetpack toggling, swimming, attached traversal and other map overlays are not
authorized by it. Original jump-held jetpack thrust is retained. Goggles/Bio Mask
keep their original equip/remove and weapon-redraw animations; another command
during those transitions is rejected, rather than saved for later.

The original boot attack entry `800517a4` checks player+3b8 with mask `00ff00ff`:
equipment and selected ID must be zero; the stale drawn-weapon byte at +3b9 is
irrelevant. This also matters to history: selecting Boot holsters, retains the old
drawn ID and sets selected ID zero. Q calls this original routine only in a supported
standing idle with Boot already selected, no movement/jump/fire/aim/interaction,
and a safe original-call context. It chooses full-body melee animations 112–115;
`80049044` runs their hit path, including steroids. No input/fire macro or temporary
equipment writes are used. Q is inactive while armed, crouching, moving or otherwise
unsupported. A safe independent armed kick was not established: it would require
bypassing original full-body/equipment ownership. Select Boot on 1 first.

## Held crouch contract

LCtrl is an action, with no Triangle bit. The old shared Triangle path at
`80041060` waits for a 150-unit hold and interprets short release as quick turn.
Modernized requests original crouch animation 176 immediately at the verified
player animation update (`8005a210`, caller `80041b34`, original callback table
`800c2754`). Original initialization, event processing and animation completion
remain in place. Only that transition's body-track time budget at `80059db0` is
multiplied by four once per animation update; upper-body, simulation, gravity and
ordinary movement time are unchanged. Reverse 176 stands; 178 is crouched idle.

Release checks the original static-room floor/ceiling query `8007765c` at the
current position, with the original `8007926c` standing threshold of 0x370.
Object-relative support fails closed. No forced actor height, final position,
collision result or animation-event bits. Focus/menu release clears held intent;
standing occurs when gameplay resumes and clearance permits. Interrupted/dead or
unsupported state ownership cancels the stance lease. Crouched movement keeps
original collision and root-motion magnitude, redirected through the same view
axes and probe endpoint contract. Static low-ceiling geometry still needs human
acceptance; native clearance fixtures are not terrain proof.

## Distance and settings

Alt-wheel exclusively changes desired boom distance, up closer/down farther:
192 game units/notch, clamped 768–6144. The smoothed boom approaches preference
once per host-vblank sequence with a 0.25 blend at one tick. Original camera
constraints still solve walls/rooms. Their compressed result never overwrites
preference. Preference lasts for the process, including focus/menu recovery;
restart begins with the original boom. FOV and right-click precision aim are unchanged.
Shoulder/recenter work, other maps, tight rooms and vertical traversal remain D10.

Profile schema 7 backs up old bytes and explicitly migrates default crouch V→LCtrl.
Custom bindings win; newly added actions take unused keys if their defaults are
already customized. Existing speed, aiming, capture, E/C and Start behavior remain.
LCtrl, X, brackets, semicolon/apostrophe and Z are admitted consistently by launcher
and runtime. Captured Z no longer supplies fallback crouch; an explicitly customized
Z action is respected. Menu Z still supplies original Triangle/back. Numbers are
now named rebindable group actions. `--show-bindings` reports actual migrated keys.

## Verification

Final evidence and remaining gameplay acceptance are recorded in
`documentation/reports/d08a-controls.json` and the current status/handoff.
Private SDL replay uses its own preferences/cards and logs every synthetic grant.
No generated C is edited by hand; added hook entries are regenerated through
`build.py`, which validates/imports original media and checks the reviewed patch stack.

Final build: `fdc8d4ece50a4a98248933684e5efa7f51b182b965b3cf7f347c4971e5065c28`.
The established build/import/patch workflow passed. All **57 Python tests** passed
including owned-disc integration, and the input, controls, aim and scene native
checks passed. The complete private SDL replay exited 0 (105 samples, 91 logged
fixture writes); it observed M healing 8000→9000 while consuming a 1000-unit dose.
The same build completed the **14-checkpoint Vanilla route** with exit 0.
Crouched/standing and Vanilla spawn/inventory/turn captures were reviewed. These
are bounded automated/capture results, not human or full-campaign acceptance.

Raw final runs: `recomp/analysis/pc-input/d08a-controls-16/report.json` and
`recomp/analysis/vanilla-regression/d08a-vanilla-01/report.json`. Earlier failed
replays remain available and are explained in the verification report. They
exposed both implementation defects and invalid timing/survival assumptions in
the private driver; they are not counted as successful gameplay checks.

## Human acceptance checklist

- Use real pickups: groups/repeats, upgraded and empty weapons, wheel, X, and an
  empty-ammo live pipe-bomb remote. Check M, N/B, grounded J and brackets/U after
  original gadget/weapon redraw settles; verify unavailable/depleted items.
- Hold/release Ctrl while idle and moving, under a low ceiling, leaving it and
  near doors/objects. Release during focus loss, pause and inventory; check stance
  recovery without clipping or unintended quick turns.
- Test Alt-wheel bounds, corners/wall compression and recovery, aiming comfort
  and traversal return. Check that it never switches weapons or changes FOV.
- Recheck E interaction/ladder transfers, C holster, right-click precision aim,
  Escape/Enter Start, F10, and customized bindings in both profiles.
- Q should use original melee only with Boot already selected and standing still;
  armed/unsupported Q must not fire. R leaves original pickup-activated steroids
  alone. Confirm lit-dynamite pending-switch behavior without expecting a safe stow.

## 2026-09-27 user feedback

The user praises this controls iteration and explicitly confirms wheel weapon
selection and Alt-wheel camera distance both work. This is human evidence for
those controls, not full inventory/terrain/campaign acceptance. The newly requested
standstill directional-jump follow-up is [D08C](34-standing-directional-jumps.md).
