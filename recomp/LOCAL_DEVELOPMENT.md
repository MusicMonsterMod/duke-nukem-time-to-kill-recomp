# Native PC development

The root repository tracks this directory's authored source/tools/tests/patches.
Start with [the build guide](../documentation/04-build-and-run.md) and
[current status](../documentation/00-status.md).

```bash
git submodule update --init --recursive
.venv/bin/python recomp/tools/local/build.py --jobs 4
python3 recomp/tools/local/run.py
```

Place your owned USA SLUS-00583 dump under root `game/`; it is ignored.
Build applies the complete tracked framework patch before generating game code.
After editing framework source, run `python3 recomp/tools/local/export_runtime_patch.py`
and include the updated patch in your root commit. Test application on the pinned
clean dependency. Do not commit only the unchanged upstream gitlink and assume
it includes modified runtime files. Do not export generated or retail data.

Never hand-edit generated game C or use player cards as writable test inputs.
Linux is validated; Windows execution and complete campaign fidelity remain
unverified. Preserve the user-accepted 120 FPS quality baseline and 180+ support.
