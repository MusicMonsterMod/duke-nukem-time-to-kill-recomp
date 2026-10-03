# Framework modifications

`time-to-kill-accepted-source.patch` is the complete authored delta against
psxrecomp `08ec704a974b1f3a16335b4afeb340b9eff19926`. Build applies it through
`tools/local/apply_runtime_patches.py`, which recognizes an already-applied patch
without undoing live source and fails on conflicting revisions.

The historical incremental stack under `history/` is retained for engineering
history only. It is not applied: clean-checkout closeout testing found that the
old stack depended on unrecorded context. The consolidated patch was applied to
a pristine pinned checkout and all 38 modified/added source files matched the
accepted working tree byte-for-byte.

After framework edits, run `python3 recomp/tools/local/export_runtime_patch.py`
from the workspace root. It uses a temporary Git index, preserves the dependency's
real index, and includes reviewed untracked runtime/recompiler source. Stage the
result in the root repository. Dependency licenses are unchanged.
