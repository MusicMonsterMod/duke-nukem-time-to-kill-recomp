# Bio Mask first-person mesh

Original editable Blender mesh for D08A26, reviewed by the user on 2026-10-25.
2400 triangles after modifiers, exact mirrored hoses, eight-sided tubing and
flat face shading. Runtime backface rejection emits 1174 triangles. The 256x128
atlas uses nearest sampling and baked per-face lighting for the PS1/N64 look.
This is real geometry in the weapon render stream, not a rendered PNG overlay.

## Texture sources

Both source photographs are CC0-1.0, from Poly Haven:

- Rubber Tiles, Amal Kumar: https://polyhaven.com/a/rubber_tiles
- Rusty Metal 02, Rob Tuytel: https://polyhaven.com/a/rusty_metal_02
- License: https://polyhaven.com/license

`provenance.json` records downloads, hashes and processing. `sources/` contains
the source JPGs; `biomask-atlas.photon/` is the editable Photon atlas.
The original model uses no retail art or UltimateDoomVisor pixels.
Source texture licensing does not grant any rights to the Duke Nukem franchise.

## Rebuild

From the workspace root, with Blender 4.0.2, Python Pillow, and Photon installed:

```sh
python3 recomp/assets/models/biomask/prepare_textures.py
blender -b --python recomp/assets/models/biomask/build_model.py
blender -b --python recomp/tools/local/export_biomask_mesh.py
python3 recomp/tools/local/export_biomask_atlas.py
```

Set `PHOTON_BIN` if Photon is not on PATH or at `~/.photon/bin/photon`.
The model script also makes preview renders and a GLB for local review. The
runtime consumes the exported mesh/1555 atlas headers under `recomp/src/ttk/`.
Edit the Blender model directly and run only the exporters to retain hand edits;
rerunning `build_model.py` recreates the procedural model.
