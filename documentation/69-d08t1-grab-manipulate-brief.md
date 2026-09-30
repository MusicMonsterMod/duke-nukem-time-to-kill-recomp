# D08T1 brief: separate push/pull manipulation from mantling

User-supplied backlog brief (2026-09-30) for [D08T1](../MODERNIZATION_JOBS.md),
a follow-up to the accepted D08T ([64-d08t-pushable-objects.md](64-d08t-pushable-objects.md)).
Backlog only: do not implement until the user selects D08T1. The inspection of
the current mantle, push/pull and input code comes before any change.

**Default binding decided while filing: Right Mouse / Mouse2 in
Modernized.** Mouse2 is currently held **original precision aim**
(`19-pc-action-input.md`, `22-view-facing.md`, the D08Q flight lease in
`47-playtest-iteration.md`). The user decided that Modernized controls do not
need precision aim: with `view` weapon aiming, holding right mouse only "keeps
the same view target, body facing and weapon pitch" (manual), which is
redundant with modern mouse aiming. So in Modernized, Mouse2 becomes Grab /
Manipulate by default. Vanilla keeps right-mouse precision aim unchanged. The
audit must still decide and record: what Modernized loses (any state where
Mouse2 still does something useful, such as `original` weapon aiming, original
camera, or the aim-held flight lease in D08Q), whether precision aim stays
available as an unbound or rebindable action in Modernized, and that no
Modernized code path still reads Mouse2 as `original_aim` once it is Grab.
**Proposed binding model (user: "alt to grab should probably be legacy",
2026-09-30; confirm when the job starts):**

| Profile / aiming | Right mouse | Alt | Grab / Manipulate |
|---|---|---|---|
| Modernized, `view` weapon aiming (default) | Grab | Grab (secondary) | RMB or Alt |
| Modernized, `original` weapon aiming or original camera ("legacy" aim) | Original precision aim | Grab | Alt |
| Vanilla | Original precision aim | unchanged | Original Action rules, untouched |

Grab / Manipulate is one logical action with two default bindings, RMB and
Alt. Right mouse only reaches Grab when the profile does not use it for
precision aim, so nobody loses precision aim where it still means something,
and Alt always works as the grab key. Both bindings are rebindable.
Audit Alt too: **Alt + wheel** already sets third-person camera distance
(`pc_input.cpp`, manual). Holding Alt, alone or with W/S, must not release
mouse capture ("Ctrl/Alt/GUI shortcuts release capture",
`19-pc-action-input.md`) or trigger an OS or window-manager shortcut. Alt +
wheel while grabbing stays a camera zoom. Left and right Alt should both work
unless the audit says otherwise.

## Goal

Redesign the controls for pushable/pullable objects so that physical object
manipulation no longer overrides or changes Duke's normal mantle controls.

The current push/pull implementation works mechanically, but introduces an
inconsistent control rule for objects that are both:

- mantleable, and
- pushable/pullable.

This task should preserve the existing push/pull functionality while giving
physical object manipulation its own dedicated input.

## Current behaviour

Normal mantleable objects use **E -> mantle/climb**. This is established
behaviour throughout effectively the entire game.

Pushable objects currently behave differently. When Duke approaches a pushable
object such as the dumpster and presses **E**, Duke enters the push/pull
manipulation state. The current manipulation controls are:

- **W/S -> Push/Pull**
- **E -> Let Go**
- **Space -> Climb**

The HUD currently communicates this as:

`W/S PUSH/PULL - E LET GO - SPACE CLIMB`

This works mechanically, but creates a UX inconsistency.

## Problem

The player has learned throughout the game that **E = mantle/climb**. However,
if an object happens to possess the pushable property, the controls
unexpectedly change: **E = manipulate** and **Space = mantle**.

The player cannot necessarily tell visually whether an object is implemented
as pushable. An invisible implementation property changes the controls
required to perform an otherwise identical traversal action. For example:

1. The player encounters a dumpster.
2. They press E expecting to mantle it, as they would with other objects.
3. Instead, Duke grabs the dumpster and discovers that it can be pushed/pulled.
4. Later, the player wants to climb onto the dumpster.
5. Their established expectation is still to press E.
6. Because the dumpster is pushable, E does not perform the normal mantle action.
7. The player is instead expected to know that this particular object's mantle
   control has changed to Space.

Discovering that an object is movable does not naturally communicate that its
mantle control has also changed. We should remove this exception.

## Design principle

**Pushability must not change traversal controls.**

A pushable object is still an ordinary world object that happens to possess an
additional manipulation capability. Therefore:

> A pushable object that is geometrically valid for mantling should mantle
> exactly like an equivalent non-pushable object.

Push/pull should be an **additional verb**, not a replacement for the object's
normal interaction/mantle behaviour.

## New control model

Introduce a dedicated rebindable action: **Grab / Manipulate**, default
keyboard binding **Right Mouse Button / Mouse2** in Modernized (see
the note above; Vanilla keeps right-mouse precision aim).

- **E -> existing interaction / mantle behaviour**
- **Space -> jump**
- **LMB / Mouse1 -> fire**
- **RMB / Mouse2 -> grab/manipulate physical objects** (hold)
- **WASD -> movement**

Do not hardcode manipulation logic directly around Mouse2. Create/use a proper
logical input action so that Grab / Manipulate can be rebound like other
controls. Mouse2 is simply its default binding.

## Pushable object behaviour

- **Press E:** perform the normal mantle behaviour. The object being pushable
  must not alter this. E against the dumpster mantles the dumpster exactly as
  E would mantle any other valid object.
- **Hold RMB / Grab:** Duke grabs/engages with the object. While Grab remains
  held, **W -> Push**, **S -> Pull**. Preserve the existing push/pull mechanics
  wherever possible; the objective is the interaction/control model, not a
  rewrite of already-working object movement.
- **Release RMB / Grab:** immediately disengage. Duke returns to normal
  movement.

## Prefer hold-to-grab, not toggle-to-grab

Grab should behave as a **held physical action**, not a modal toggle: the
player holds RMB because Duke is holding the object; when the player releases
RMB, Duke releases the object. Avoid requiring a second press or another key
to leave manipulation unless an existing technical constraint makes this
necessary. This eliminates the current **E -> LET GO** behaviour.

## Mantling must remain independent

The critical invariant: **the presence of push/pull functionality must never
change the normal mantle input.**

| | Before | After |
|---|---|---|
| Normal object | E -> Mantle | E -> Mantle |
| Pushable object | E -> Grab, Space -> Mantle | E -> Mantle, Hold RMB -> Grab |

This distinction should exist at the input/action level rather than as
object-specific control exceptions.

## Manipulation state

While a valid object is grabbed: **Hold RMB + W -> Push**, **Hold RMB + S ->
Pull**, **Release RMB -> Let Go**.

Determine appropriate behaviour for other inputs while manipulating: A/D,
Space, E, weapon firing, weapon switching, camera movement, taking damage,
falling, the object becoming obstructed, Duke becoming separated from the
object, the object becoming invalid/destroyed, entering menus or pausing.

Do not invent complex new behaviour unnecessarily. The priority is predictable
state cleanup: there must be no circumstance where Duke remains logically
attached to an object after the manipulation should have ended.

## E while currently grabbing

Investigate whether allowing **E while an object is grabbed** to release and
immediately mantle that object is safe and natural:

1. Duke is holding the dumpster with RMB.
2. Player presses E.
3. Manipulation terminates cleanly.
4. Duke performs the normal mantle action against the dumpster.

This would preserve the invariant that E always means the mantle interaction,
even while manipulating. Do not force it if it creates unreliable transitions
or conflicts with existing mantle logic. At minimum, **Release RMB -> E ->
mantle** must work normally and immediately.

## HUD / instructional text

Replace `W/S PUSH/PULL - E LET GO - SPACE CLIMB`. While manipulating, an
appropriate message is approximately:

`W/S PUSH/PULL - RELEASE RMB TO LET GO`

Use terminology and formatting consistent with the existing TTK HUD. Do not
keep telling the player to use Space to climb a pushable object. If the input
system supports dynamic key labels, show the current Grab / Manipulate binding
instead of the literal `RMB`.

## Discoverability

The player should not need to know that an object is pushable to use normal
traversal controls. E against a pushable/mantleable object simply performs the
expected mantle. Manipulation is discoverable as a separate capability through
Grab / Manipulate. Contextual prompts may say an object can be manipulated,
but correct operation must not depend on understanding an invisible object
classification.

## Input binding

Add **Grab / Manipulate** to the normal binding system, default **Mouse2** in
Modernized. Ensure it does not interfere with mouse-look, Mouse1 firing,
menus/UI, pause screens, mouse capture or any platform-level mouse handling.
Do not assume RMB is unused from observed gameplay: audit existing input
handling first. The existing Mouse2 precision-aim path (`original_aim`) must be
identified and cleanly retired or rebound in Modernized, not left racing with
Grab (see the note at the top). Vanilla is unchanged.

## Scope

This task is about **control semantics and interaction consistency**. Do not
unnecessarily redesign the existing E mantle system, Duke's general traversal,
the push/pull physics, object collision, jumping or unrelated interaction
controls. E-to-mantle works for the overwhelming majority of the game and
stays the baseline. We are fixing the exceptional behaviour of objects that
gained push/pull, not redesigning the game around that exception.

## Testing

- **Normal mantleable object:** E -> mantle, unchanged.
- **Pushable + mantleable object:** E -> mantle, consistent with normal objects.
- **Manipulation:** hold RMB -> grab; W/S while held -> push/pull; release ->
  disengage.
- **Manipulate then mantle:** push/pull, release RMB, immediately press E;
  Duke mantles normally.
- **Repeated transitions:** `grab -> push -> release -> mantle` and
  `mantle -> leave object -> grab -> pull -> release`, with no stale
  manipulation/mantle state.
- **Invalid manipulation:** hold RMB against ordinary geometry; nothing
  unexpected happens.
- **Interrupted manipulation:** obstruction, damage, falling/separation and
  pausing end the manipulation safely where appropriate.

## Architecture requirement

Do not add another dumpster-specific special case. **Mantleable** and
**manipulable** are independent capabilities; an object may be mantleable
only, manipulable only, both, or neither. An object with both must expose both
without one silently remapping the controls of the other. The input actions
stay semantically independent: `Interact/Mantle -> E`,
`Grab/Manipulate -> Mouse2`.

## Desired end state

At the dumpster the player chooses between two intentions:

- **"I want to climb this."** -> press **E**, exactly like other mantleable
  objects.
- **"I want to move this."** -> hold **RMB**, then **W/S** to push/pull;
  release RMB and Duke lets go.

Pushability adds functionality without changing any control the player has
already learned.

Before implementation, inspect the current mantle, push/pull and input-state
code and briefly document how the existing E/Space exception is implemented
(start from `recomp/src/ttk/push.inc`, `pc_input.cpp` and the `0x80051cf0`
hook recorded in D08T). Then implement the new action with the smallest clean
architectural change that removes that exception rather than layering another
exception on top of it.
