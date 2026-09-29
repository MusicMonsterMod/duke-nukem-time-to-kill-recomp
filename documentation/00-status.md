# Current status — 2026-09-29

**D08P Done (user-accepted) — area-wide control loss was overlay identity;
mid-depth wade now runs at land pace.** The user's session log showed `ident=0` from the
flooded-corridor ledge onward. The LEVEL00 zone script writes a hit position
into the overlay's trailing 16-byte scratch vector (`0x800ccf1c`), which the
9668-byte overlay guard covered, so identity failed permanently the first time
that script fired. The guard is now the 9652-byte code/table body. Separately,
that water is depth 512: the original swaps every gait for wade clips 80/81
(Vanilla 8–14 units/frame, forward only). The wade handler `0x800539f8` is
now hooked (new generated entry) and its root retargeted to camera-relative
WASD at the land run band: 45–58 units/frame, mouse steering, strafe, Space
jump; 0 identity refusals across and past the ledge. Wading straight into a
wall idles as in the original; turn the view to resume. Details:
documentation/56-scripted-camera-controls.md (iteration 6).

Pending playtest jobs D07A, D07B, D08A, D08H and D10 distance were **Done** on
user acceptance of ongoing play. D08N scuba remains cancelled. D08B/D10 broader
work stays In progress.

Binary `79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
