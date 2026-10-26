# D19Q3 - Resume Modernized gameplay after every load

Status: Done. User confirmed completion and requested board closeout on
2026-10-26. Earlier candidate/playtest statements below are historical.

Session closeout update (2026-10-26): user authorized commit and push of the current
work. Needs playtest status remains; D19Q4 records the new UI-flash reports as
backlog only. Earlier no-commit statements below describe the implementation turn.

Date: 2026-10-26. Status: Done (user-confirmed).

The user reported that slot 9 still restored paused after D37A and explicitly
requested that games never start paused. This supersedes D19Q2's previous policy
of preserving paused F7/debug saves and saves without a resume marker.

## Change

`recomp/src/ttk/game_menu.cpp` now derives a one-shot resume request from every
successfully applied savestate load. The request does not depend on the caller or
`.ttk` sidecar. In paused, living gameplay it normalizes the pause/options screen
and uses the existing guarded resume finalizer, including pause-reset, player flags,
sound and music restart. The pause page is not drawn during automatic resume.

The request is consumed once, so an ordinary later Escape pause is unaffected.
Death screens are excluded, statistics remain in their own mode, and the existing
Modernized enable guard preserves Vanilla. Loads of already-running gameplay do
not arm a future resume. Failed load requests cannot arm a resume either.

No save rewrite, sidecar migration or slot deletion is needed. Existing resume
metadata can remain for compatibility, but it no longer controls this policy.
GAME_MANUAL.md now describes the consistent Modernized behavior, and note 204
identifies its former pause-preservation rules as historical.

## Verification

Candidate SHA-256:
`736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.
Incremental build passed, movie and geometry shards current. This title-only
follow-up does not alter the framework delta already exported/verified for D37A.

Private evidence is in `recomp/analysis/d19q3-resume/`. Tests use copied savestate
files, private settings and private cards; player cards are not read or written.

| Check | Result |
| --- | --- |
| Direct startup load of slot 9 | Pause 0, automatic mouse capture, W movement |
| Repeat slot-9 and slot-8 loads | Pause 0 and movement |
| Newly created debug save made while paused | Sidecar contains only `level 0`; direct load resumes |
| Real F9 loading that paused save | Resumes and W moves |
| Real F3 LOAD GAME, confirm load | Resumes, capture and movement work |
| Real F7, load the paused save | Resumes, capture and movement work |
| Save made during gameplay, then load | Remains playing, movement works |
| Real Escape after loading | Pauses normally, releases mouse |
| Real Escape again | Resumes and captures mouse |
| Later Escape after the complete load sequence | Still pauses normally; resume still works |
| Save/load level-complete statistics | Remains mode 2; no automatic level continuation |
| Vanilla load of paused save | Pause remains 1, original policy unchanged |

`real-results.json` / `real.log` contain the real X11 keyboard tests in a private
Xvfb display. `results.json` contains the initial offscreen samples, including a
failed synthetic Escape assertion: the test-input file does not simulate the real
SDL Escape event. That assertion was resolved by testing actual keyboard input,
not by changing the game's ordinary Escape behavior. `boundary-results.json`
records statistics and Vanilla checks. Logs confirm actual successful load events
for F9, F3 and F7, not merely closing the menus.

The private virtual display uses dummy audio. Native sound/music resume calls were
exercised, but audible quality and physical-display feel remain user playtest items.
Death-screen preservation is established by the explicit branch guard, not a new
live death-route test. D37A's animation-memory repair remains in this candidate.

## Player check

Load slot 9 normally. It should be immediately playable, with no Escape press to
unpause it. Try F7 or quickload too, then confirm Escape still pauses when wanted.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

D19Q3 awaits user playtest. No acceptance closeout, commit or push performed.
