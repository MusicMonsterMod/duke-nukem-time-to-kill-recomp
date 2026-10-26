# Bio Mask - PS1 authenticity revision (D08A28)

Accepted revision 5, 2026-10-26. Original design and procedural Blender pipeline
adapted locally, not replaced by a sprite. The accepted D08A26 asset remains one
directory above, untouched. All art in this revision is original; no photograph
or retail texture pixels are used. Retail weapons informed the visual study only.

## Changes - current revision

- Plain five-sided tubes with constant radius and three broad bend spans.
  No geometric corrugation or ridges. Painted grip bands use the Buffalo Rifle
  pump's near-black 8/16/24/33 greys and narrow groove highlights as a reference.
  Six small worn flecks retain that same dark palette.
- Flamethrower-inspired metal texture contrast: stepped steel reflection bands,
  bright edge strips, dark inset panel, scuffs, nicks and painted fasteners. These are original
  pixels, not copied retail textures.
- Central housing has a darker, slightly blue steel tint inspired by the Buffalo
  barrel. A narrow cool highlight rolls into deep steel across the shell and
  grille, replacing the broad flat bright bands. Collar highlights retain their
  previous finish.
- Dedicated housing UV island and longitudinal tube unwrap. Effective front
  panel 32x16, secondary steel 32x16, tube strip 32x16; nearest-expanded into the
  existing 256x128 atlas. 48 authored RGB colours. No global filtering changes.
- 184 evaluated triangles / 90 emitted triangles per draw. Same housing outline
  and tube centerline; accepted framing, lighting and behavior retained.
- The earlier 488-triangle ribbed/flat-grey demake was rejected. Its source is
  retained locally in `recomp/analysis/d08a28-revision2/previous-ps1/`.
- The startup packet-buffer reservation fix remains active, including for loads
  of older player saves. The original accepted high-detail asset remains intact
  in the parent directory and can still be restored with the commands below.

## Rebuild accepted PS1 asset

From the repository root:

```sh
blender -b --python recomp/assets/models/biomask/ps1/build_model.py
blender -b --python recomp/tools/local/export_biomask_mesh.py -- --asset-dir recomp/assets/models/biomask/ps1
python3 recomp/tools/local/export_biomask_atlas.py --asset-dir recomp/assets/models/biomask/ps1
cmake --build recomp/build-local --parallel 4
```

The generator recreates the model and texture in this directory only. For hand
edits, save the revision `.blend` and PNG, then run only the exporters. The blend
packs its texture. Preview PNGs, Blender backups and GLB exports are ignored;
`mesh-stats.json` records geometry metadata, not in-game visual acceptance.

## Restore previous high-detail asset

The accepted Blender model, atlas, Photon source and original generator remain
in the parent directory. Restore their runtime data without regenerating art:

```sh
blender -b --python recomp/tools/local/export_biomask_mesh.py
python3 recomp/tools/local/export_biomask_atlas.py
cmake --build recomp/build-local --parallel 4
```

Reapply the accepted PS1 asset using the two `--asset-dir` export commands above and
rebuild. The exporters also accept `--output PATH` to compare exported headers
without changing the build. Neither revision changes native saves or preferences.

Research, before/after evidence and validation limits are in
`documentation/264-d08a28-biomask-ps1-authenticity.md` at the repository root.

Foundational style recipe: `documentation/265-native-mesh-art-style.md`.
`accepted-revision.json` identifies this accepted source and runtime data.
