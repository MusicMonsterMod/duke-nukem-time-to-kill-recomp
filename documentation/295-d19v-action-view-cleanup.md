# D19V - Remove redundant interaction view settings

2026-10-28. Done, user-accepted. User: "perfectly done, lock it in and close it out".
Implementation, tests, manual and documentation commit/push closeout authorized.

## Behavior and ownership

**S - Source:** USE INTERACTION VIEW and MIRROR VIEW were camera selectors,
not interaction or reflection enable switches. Their only runtime consumers
were the Use/Mirror perspective overrides and the transient camera eligibility
checks. Both defaulted to First under the global First Person selection.

The two Advanced rows and descriptions are removed. Use and Mirror now retain
the default perspective policy and follow global VIEW. Their action identities,
normal-camera authentication, native animation/events, interaction arms,
reflected actor rendering and world/load/replay ownership remain intact.
The accepted D36 audit's ownership boundaries are preserved. Other action-view
settings retain their choices. Vanilla retains its original control/presentation
gates; no framework, generated game code or retail assets were edited.

Profile schema 57 removes the two keys. Validation strips old keys, including
malformed values, before checking remaining preferences. This also consumes
stale menu sidecars without losing their other valid edits. Existing migration
backup behavior is retained. Old launcher flags remain hidden compatibility
no-ops; help/settings output no longer advertises them. Inherited environment
overrides are removed by the launcher and have no runtime consumers.

## Verification

**Native tests:** player build succeeds. All 105 profile tests pass, including
fresh defaults, schema 48/49/50/56/57 cleanup, malformed obsolete values, both
profiles, persistence/reload, stale menu sidecars, hidden old CLI flags and
inherited environment cleanup. The future-version rejection test now uses 58.

Controls suite passes with the owned executable, clean LEVEL00 guard fixture,
LEVEL01 and all level overlays. Existing interaction pose/arm, door-kick,
global-view, pause/load/replay, semantic limb ownership, reflection and Vanilla
guards pass. Revised cases prove retired overrides cannot suppress the Use or
Mirror eye path. Input, aiming and near-clip suites pass too. These are native
fixture contracts, not exhaustive in-game visual acceptance.

Accepted executable SHA-256:
`a3215dc7b8fd1a7992e96ccaf899284483650e23101a33d99e01581cc496fd87`.

**L - Private runtime:** evidence lives in ignored `recomp/analysis/d19v/`.
Runs use Xvfb, dummy audio, 120 presentation, private settings and copies of
current saves/cards. The original 41 player files retain hashes and mtimes.
No sound quality or performance claim is made from these diagnostic runs.
Automated menu navigation did not reliably reach Advanced; inspected captures
show gameplay and other menu pages, so they do not establish visual acceptance
of the revised page. The final attempt also injected the pause/options selector
in the private runtime, without obtaining a usable Advanced capture. No live
menu-sidecar success is claimed; migration/sidecar evidence above is automated
Python testing. All diagnostic processes are closed.

**U - User acceptance:** the user accepted the delivered cleanup and explicitly
authorized closeout. This does not turn unsuccessful automated menu captures
into visual evidence. No further D19V work remains. Full campaign interaction
coverage is not claimed; this cleanup does not reopen the accepted D36 art work or D35N1's
separate missing mirror-quote job.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.
