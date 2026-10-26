# D19Q2 - Paused-save investigation

Status: Done (user-accepted 2026-10-23). No confirmed pause-state regression
and no gameplay changes.

Current policy update (2026-10-26): the user explicitly requested that Modernized
gameplay loads always resume. D19Q3 supersedes this report's intentional pause
preservation policy; see [note 256](256-d19q3-always-resume.md). The evidence below
is historical and does not describe the new behavior.

## Finding

The reported UI slot 10 is a SAVE GAME save with `level 0` and `resume 1`
in its `.ttk` sidecar. Its guest state is deliberately paused. Loading the
private copy through the debug loader restores the pause list. Quickload of
that same copy resumes play correctly. This matches the accepted
[D19Q design](171-d19q-save-game.md) and GAME_MANUAL.md.

SAVE GAME captures the paused guest state and records the intention to resume
in the sidecar. LOAD GAME and quickload read that marker. F7 and the debug
loader restore the actual saved state without requesting automatic resume.
A debug/F7 save made while paused has no resume marker; even player loads of
that save preserve its deliberate pause. Missing legacy metadata therefore
must not be treated as permission to resume every paused state.

## Evidence

Existing accepted executable, unchanged:
`482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
Private cards, profiles and savestates under `recomp/analysis/d19q2-pause/`.
Source fixtures copied from earlier isolated D17M, D19K and D20 runs.
No player saves/cards or preferences modified. No build, commit or push.

| Route | Observed result |
| --- | --- |
| Reported UI slot 10, debug load | Paused 1, screen 8, working modern pause list |
| Same slot, explicit Escape | Paused 0, screen 0; play resumes |
| Same slot, quickload | Paused 0, screen 0; movement works |
| New pause-menu SAVE GAME | Stays on SAVE GAME; sidecar includes `resume 1` |
| F3 LOAD GAME of that save | Resumes play; movement works |
| Debug load of that save | Preserves pause |
| Quick save/load keys while paused | Ignored, no save/load generation change |
| Quicksave from gameplay | Remains playing; sidecar has level only |
| Quickload and debug load of gameplay quicksave | Both remain playing |
| F2 SAVE GAME into empty slot | Saves and returns to play; subsequent quickload resumes |
| Title LOAD GAME of F2 save | Mode 1, paused 0, screen 0, successful load generation |
| Debug save while paused, then quickload and F3 LOAD GAME | Both preserve pause; sidecar has level only |
| Real F9 and F3 keyboard loads in Xvfb | Resume and automatically capture mouse; real W movement works |
| Real F7 load of SAVE GAME save | Pause list, mouse free; Escape resumes and recaptures |
| Vanilla gameplay save/load | Remains playing |
| Vanilla original pause save/load | Preserves pause (screen 12); Start resumes normally |

Modernized probes used OpenGL at 120 FPS, offscreen and isolated Xvfb;
Vanilla used 60 FPS. Real-input checks used scale 1. Automated steering was
used for the broader menu matrix; real-input tests had steering disabled.

Local evidence: `matrix.json` / `matrix.txt`, `real-input.json` /
`real-input.txt`, `title.txt`, `legacy.txt`, `vanilla.json` / `vanilla.txt`,
plus runtime logs and scripts in that directory. Early real-input probes
sampled capture too soon; the final probe waits for capture and allows a
longer movement interval. The matrix's last title attempt lacked its fixture;
the separate `title.txt` run supplies the successful title-load evidence.
The real-input run's final title row stayed in mode 0 and is not title-load
proof; use `title.txt` for that route.

## Scope and review

No confirmed regression justifies a gameplay fix. Do not globally clear pause
on load: that would change F7/debug, legacy and Vanilla behavior. No manual
change is needed because it already documents this distinction.

This investigation does not establish every historical paused-save report's
cause. A SAVE GAME save that remains paused after LOAD GAME or F9 would be a
new reproduction to investigate, including its sidecar and load route.
Physical-display mouse feel and audible sound were not assessed; Xvfb used
dummy audio. No full campaign or statistics-screen sweep was performed.

User confirmed "it seems to be working" and accepted D19Q2 on 2026-10-23,
then explicitly authorized documentation closeout, commit and push.

A separate subsequent user report - loading and pressing Escape shortly
thereafter loses Modern controls - is tracked as **D04B Todo** in the job
board. It requires robust control recovery, Modern by default, settings-only
Modern/Legacy selection and removal of F10 scheme switching. This new timing
report is not disproved by the ordinary load/resume checks above. No D04B
implementation has started.
