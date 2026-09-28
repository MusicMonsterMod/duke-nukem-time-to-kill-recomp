# Current status — 2026-09-28

**D08O closed — Done, accepted by playtest 3.** Modernized swimming is complete:
A/D surface strafe, Ctrl dive (original native state machine `80045564`),
underwater W/A/S/D where the camera looks, Space up / Ctrl down, auto-surface,
mantle-only exit; shallow → subway-platform ledge jump is Duke's ordinary
directed 98, a head taller, with XZ held until the feet clear the lip near the
wall (15/15 lab, accepted in play); plain wall + W+Space is a vertical hop. The
item switcher shows the user's Bio Mask art (`biomask-small.png`). Vanilla
unchanged. Details: documentation/55-swim-controls-research.md §8–§9,
54-swim-redesign.md; job board D08O.

Open jobs: see the Todo/Needs-playtest rows in `MODERNIZATION_JOBS.md` (D08N
scuba item is the natural swim follow-on; D08L, D09 open; several D07/D08
rows still await their own playtests).

Binary `78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```
