# D34C - Local multiplayer ownership foundation

Status: Closed for now, 2026-10-28. User accepts the current original-style
two-player baseline and this initial ownership foundation. Modern split-screen
gameplay/presentation remain deferred; the full D34C criteria are not Done.

## Implemented

The SDL input sampler now retains both physical assignments as independent
`ControllerSample` records: connection, buttons, all six axes and a device
identity generation. Unplug clears only that assignment. Repeated sampling of
the same device retains its generation; removal/replacement changes it. Invalid
indices return a neutral record. The campaign action machine still consumes
only assignment 1. Raw observations are not input permission: a future seat
consumer must apply focus, capture, epoch and held-button release gates.

D09A remains responsible for mapping keyboard/P1 and first gamepad/P2, or
first gamepad/P1 and second gamepad/P2. Physical assignment and guest seat must
remain distinct. This change does not yet attach a second modern action machine,
change controls, capture the split-screen mouse, or enable independent cameras.

`level_overlay_guards.py --arenas` derives a separate six-entry guard manifest
from the owned disc. Missing/duplicate arenas fail; the existing analysis rejects
writes inside authenticated code or an unaccounted scratch tail. `--arenas
--check` compares the exact manifest body, including order and extra entries.
Campaign guards and their eligibility are unchanged. The new manifest is not
yet used to grant a gameplay hook lease.

## Verified identity and address evidence

Verified static against owned EXE SHA-256
`b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a`:

- `0x80026504` selects player base `0x800d7198`; `0x80026520` advances
  by `0x8a4` while iterating the live player count.
- `0x80026248`-`0x8002626c` derives the normal camera from view index
  `0x800d2654`, base `0x800d6eb0`, stride `0xb0`. The alternate table
  is `0x800d7010`, selected by the actor's `0x20000000` flag.
- `0x800265e8`-`0x8002660c` increments/wraps the drawn viewport index.
  The native alternating draw remains in place.
- All six arena directories DB15-DB20 contain the same 1420-byte
  `BASEOVR.OVR`, tag `0x12`, code ending at `0x800cad3c`, no scratch tail.
  Full SHA-256: `1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65`.
  There are no LEVEL15-LEVEL20.OVR files. Campaign tag-based identity must
  not be extended by guessing such files or assigning six unique tags.

Verified runtime in a private copy of the earlier MESA results snapshot
(mode 2, level 15, two players): live overlay matches that full digest;
normal camera owners are `0x800d7198` and `0x800d7a3c`; actor camera pointers
are `0x800d6eb0` and `0x800d6f60`. These are retained results-state pointers,
not a new live-match, six-arena gameplay or active-hook acceptance test.
Private artifacts are under `recomp/analysis/d34c/`; port 9498. Test cards
were copied from previous diagnostic directories. No player cards/settings were
written. The diagnostic process was closed before building.

## Ownership audit and remaining implementation

| System | Current ownership | Required D34C work |
| --- | --- | --- |
| Input/actions (`pc_input.cpp`) | One InputFrame, action queue, crouch/interaction and capture state | Two seat action contexts, with keyboard routed only to its selected seat and device-release/focus gates |
| Movement/traversal (`modern_controls.cpp` and included adapters) | Fixed actor/camera constants and shared gait, jump, collision and traversal leases | Explicit actor context and independent state; authenticate shared resident hooks plus arena overlay |
| Camera | One orbit, look accumulator, recenter and persistence state | Independent third-person orbits and native collision solves; no P2 writes into campaign preferences |
| Aim (`weapon_aim.cpp`) | Fixed actor, shared query scratch, beam completion/target and reticle | Owner-specific shot/query/beam state and viewport reticle |
| HUD/presentation | Campaign HUD assumptions; native arena HUD retained | Per-viewport ownership, aspect and transition handling |
| Replay (`frame_replay.cpp`) | Split bypasses campaign replay; one campaign timeline | Independent histories/composition and measured cadence per viewport |

Do not remove the existing single-player gates to make the new samples active.
In particular, enabling `input_modern_controls()` alone would still direct
movement, actions, aiming and replay through P1 state. First-person presentation
remains outside this initial third-person work and must follow the D36 audit
before any related change.

## Verification and limits

- Player build succeeds.
- Native SDL input suite passes under private Xvfb, including two actual SDL
  virtual devices with independent axis/button observations, stable generations,
  P1 removal without P2 promotion, reconnect, P2 detach/neutralization and invalid
  indices. Existing campaign action, focus/capture and classic mixed-seat tests
  also pass. This is not physical-controller acceptance.
- Eight overlay tests pass, including all six owned arena guards and unchanged
  campaign guard verification, shared tags, missing arenas and duplicates.
- No framework implementation changes in this slice; the existing dirty
  submodule is preserved. No commit or push.

Independent modern movement/camera/aim, actions, per-viewport replay, 60/120 Hz
measurements, all-arena play and the full D34C acceptance matrix remain to be
implemented and verified. At candidate delivery D34C remained In progress, not Needs playtest or Done.
The current player controls are unchanged, so no manual control revision is
appropriate yet.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

## User acceptance and closeout, 2026-10-28

User: "lock it in, close us out. two player now works and is fundamentally
how it plays off the disk". The current original-style two-player baseline
and D34C ownership foundation are accepted for retention. This explicitly
authorizes implementation, tests and documentation commit/push closeout as
MusicMonsterMod.

D34C is Closed for now: foundation accepted, modern gameplay and presentation
deferred. Independent modern per-player movement/camera/aim, action contexts,
viewport replay and 60/120 Hz acceptance remain unfinished. Its existing scope
and acceptance matrix are retained for a future explicit selection; they are
not marked Done by this baseline acceptance. The user's report does not add
agent-observed physical-device, all-arena or performance tests.

Existing build and bounded verification remain in [note 291](291-d34c-local-multiplayer-foundation.md).
No gameplay changes, rebuild or launch for this closeout. Player settings,
saves and cards remain untouched. Launch: `python3 recomp/tools/local/run.py`.

Closeout verification: the complete framework export is unchanged and applies
to its clean pinned dependency; all 55 modified/added framework files match.
Dependency pins are unchanged. Repository privacy and whitespace checks pass.
GitHub authentication and commit identity are MusicMonsterMod.
