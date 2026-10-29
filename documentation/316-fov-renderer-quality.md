# 90 FOV baseline and renderer quality investigation

2026-10-29. Selected batch; implementation and investigation in progress.

Jobs: D14F settings; D17AE ceiling (11); D17AF water (10); D20H colours (5);
D17AG panelling (6); D17AH sky (8); D17AI cube wall (12); D17AK shared sweep.

All causes are initially unconfirmed. Preserve latest player fixtures and use
private copies. Capture hashes/mtime and exact slot mapping before testing.
Check previous default, 90, 120 and contrasting aspects; preserve fixed weapon
presentation and 120-Hz compatibility. Shared code changes have one coordinator.
No replacement renderer, acceptance, commit or push is authorized by this note.

## Fixture intake and initial evidence

Originals are under `recomp/saves/local-play/openbios/`; private immutable intake
is under ignored `recomp/analysis/d17ak/fixtures/`. UI slots are one-based; the
file suffix is zero-based. All original files were hashed with modification
timestamps before testing, including settings and the reference image.

| Job | UI slot | PST suffix | SHA-256 | Modified UTC |
| --- | --- | --- | --- | --- |
| D17AE | 11 | 10 | `efe0b4bb9575137ba92b305b026877396a61342ae28687721e1b6c355d9445aa` | 2026-10-29 10:33:34 |
| D17AF | 10 | 09 | `988ba429c714b0469fd84bb3c262d542f165eea1b7f0286273bfffd15b98657c` | 2026-10-29 10:35:04 |
| D20H | 5 | 04 | `0b0976d72279c424dbfa1b488357cca3bb7bd3fd9c3b2c2bfa1960d92d1a0335` | 2026-10-29 10:32:14 |
| D17AG | 6 | 05 | `7835791aef22f3c76634b02f736e2e17fb2ace4b3417c41b7fd7cab57c13046f` | 2026-10-29 10:39:45 |
| D17AH | 8 | 07 | `50884ee019d7ddbdda95f4da6270d41e563c160839a0f7806ab19582755a711d` | 2026-10-29 10:40:30 |
| D17AI | 12 | 11 | `1e176e4eca8ee4ee8c61695e7dc0c9e1189e791623b9becc80072b7ee3208b3a` | 2026-10-29 10:45:52 |

Screenshot located at `research/screencaps/crazy-colors.png`. No fixture is
missing. Slot 10 uses this intake's latest live save, not historical analysis
copies. The first root sweep used 16:9/120 setting and 64/90, with private
Corrected precision; it records zero host packet skips or mesh fallbacks.
These contended diagnostic runs are not performance benchmarks.

Capture caveat: the inherited harness stages presented screenshots asynchronously.
The final slot-10 movement capture picked up the next slot-12 load. Those early
images are orientation/reproduction leads, not matched validation. Subsequent
harnesses poll `present_shot_seq` and require `wrote`, and poll save-load
generation/pending/success. Trace is disabled except for bounded diagnostic
windows; it can materially slow execution.

D14F settings implementation and focused tests: [note 317](317-d14f-fov-settings.md).
D20H load investigation: [note 318](318-d20h-colour-restore.md).
D17AH sky correction: [note 319](319-d17ah-sky.md).
Other faults remain individually under investigation; no shared cause is yet
asserted for them.
