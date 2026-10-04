# D17O - unstable sky appearance: intake and investigation plan

2026-10-04. Todo - next. User requested this backlog/handoff before clearing
context. No sky investigation, game launch, build or implementation in this
checkpoint. The statements below are the report and planned work, not findings.

## Report and reproduction

Load **UI save slot 10 (file 09)** and look up at the sky. The user reports
strange, glitchy movement rather than a neat parallax effect, and asks whether
its appearance can be stabilized. It is **not specific to the jetpack**; the
position in this save simply makes the issue easy to see. Do not scope this
as another jetpack fix or presume the game uses a conventional skybox.

A read-only intake copy of all current player cards/states is preserved under
`recomp/analysis/d17o-sky-intake-20261004/cards-intake`, with SHA-256, size and
mtime values in the adjacent `manifest.json`. UI 10 is:
`openbios/state_800AB6FC_slot09.pst`.

SHA-256: `2307f0f6c5bfe7fd976f66e3cbef3fb72c3a25b05e6dfa9252d1414a5dadb73d`.

Use another writable private copy for replays; keep this intake immutable.
Recheck save identity on resumption if the player has saved again. Earlier
D08Q2/D08Q3 slot-10 copies are historical regression fixtures, not an assumed
identity match. Player cards and settings are never test destinations.

Accepted implementation baseline: D08Q3 `f284dc1`, player binary
`5b4add2a896a3ab5ae16e5e02fb8163d551ef3b4a9eaad954ebd6c3837500327`.
D08Q3 is user-accepted: "i accept this! great work."

## Investigation to perform next

1. Record the active profile, view, refresh target, resolution, widescreen and
   save/overlay identities. Reproduce looking up while stationary, slowly
   turning/pitching and walking. Separate intended sky animation from motion
   induced by the camera. Start grounded; compare flight only as a control.
2. Trace how this game's sky is authored and drawn using the owned executable,
   active overlays and local retail assets. Establish whether it uses world
   geometry, a dome/box, textured background primitives, or another mechanism.
   Document authenticated routines/callers, coordinate spaces, camera coupling,
   texture/UV animation, depth/order, clipping and update cadence. Asset names
   or renderer options alone do not establish the draw path.
3. Capture full presented-frame sequences with the sky and horizon visible,
   preserving camera/input and frame timing. Compare Vanilla/Modernized,
   original/view cameras as relevant, 60 and 120 first, then 180. Determine
   whether the defect exists in original guest frames, replayed frames, or both.
   Isolate bob, widescreen, near clipping and precision only where evidence
   points to them. These are candidate factors, not established causes.
4. Distinguish intentional parallax/cloud scrolling and original PS1 precision
   from UV jumps, seams, jitter, clipping, ordering or stale/interpolated sky
   transforms. Relate visible good/bad frames to actual primitive/transform
   evidence before choosing a fix. Do not assume the D17N wall issue shares it.
5. Implement a bounded repair if supported by evidence. Preserve intended sky
   art/motion, horizon and legitimate occlusion, with Vanilla available. Avoid
   freezing the whole scene, broad rendering bypasses or a replacement sky
   asset unless separately justified and requested. Verify active overlays
   before hooks; never hand-edit generated C.

## Acceptance and documentation

A detailed engineering write-up must explain how the original sky works,
what causes the reported motion, what changed and why, verified addresses and
asset provenance, reproduction settings and save identity, before/after
presented-frame evidence, rejected hypotheses and remaining limits. Keep local
retail assets, captures and save files out of Git; commit authored source,
tests and documentation. Export and verify the accepted-source patch against
the pinned clean framework if framework source changes.

The result should look stable during repeated look-up, slow yaw/pitch and
walking routes while preserving deliberate sky motion. Check first/third
person, ground/flight, horizon edges and nearby world geometry; use other sky
views where available to avoid a slot-specific workaround. Retain accepted
D17C/D/E/F/K presentation and D08Q2/Q3 flight/recovery/aiming behavior. Record
60/120/180 behavior and any frame/audio cost without judging GPU performance
from Xvfb. User confirmation is required for Done; use Needs playtest if a
candidate is verified but visual acceptance remains outstanding.

Update this note with evidence as work proceeds, plus the canonical job board,
status/handoff and manual if player-facing behavior/settings change. Clearly
separate measured findings, user observations and hypotheses. Commit/push the
finished bounded result and stop without starting another backlog job.
