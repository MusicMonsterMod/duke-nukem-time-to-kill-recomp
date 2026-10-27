# D16 - Texture manipulation and replacement foundation

2026-10-27. Bounded foundation accepted by the user after reviewing the visible
club replacement. D16 is Done at this scope; the user explicitly authorized
commit and push. Scripting/modding and broader authoring remain future work.
This is not a full texture/modding system or whole-game coverage claim.
The separately authorized CI fix was pushed as fa6d502.

## What now works

A real club wall texture was captured, edited in Photon, exported at 40x40 and
160x160, compiled into a local specimen and rendered on the original surfaces.
The original model/UVs, lighting, cutout pixels and STP blend classification remain
in charge. The new artwork is an angular crest/lightning test design, not a
proposal to restyle the game. The other, blank wall panels use different source
regions and remain unchanged in the captures.

The interactive local evidence is
`recomp/analysis/d16-texture-foundation/texture-proof.html`. It switches between
original, same-resolution edit, higher-resolution edit, restored state and
palette-mismatch fallback. These are actual game captures, not mock screenshots.
Canonical wide captures are 1364x480 and displayed at 16:9, matching their intended
presentation aspect. The conceptual `texture-foundation.html` is earlier planning.

Editable source: `recomp/analysis/d16-texture-foundation/sample/edited.photon`.
Reopened successfully with 13 layers: the disc-derived reference background and
12 editable shape layers. Runtime exports are `edited-40.png` and `edited-160.png`.
The source, exports, specimens, captures and private saves all stay local because
the reference/border includes original retail pixels.

## Verified source and lifecycle

The historical D17Q club state is a private copy, not today's player slot 12:
`state_800AB6FC_slot11.pst`, SHA-256
`1db85905d93a910ea746aacbc48d042bece5657bcd9a62fe9919dd3b690236dd`.
No player card or save was written.

The complete captured source and CLUT match the owned disc's
`/D0/DB00/DB00A.RAW` byte for byte. File SHA-256:
`ffd62ebfe295968225388fdb735bafad82ac2ea864d64a34a18fcd9431a05bfd`.
The sampled region starts at file byte 172268, 20 bytes per row with an 896-byte
row stride for 40 rows; the 32-byte CLUT starts at 79648. Local `provenance.json`
and `sample/capture.json` retain the measurements and hashes.

The existing `ttk_anim.vram_from_group` decoder documents the native group upload
layout associated with 0x800238b0: A.RAW -> VRAM (512,0), 448x256 words;
B.RAW -> (512,256), 512x256 words. This work revalidated the selected sample's
bytes against that layout; it did not install a new game hook at that address.
`gpu.c` decodes GP0 CPU-to-VRAM uploads and calls `gr_vram_transfer_in`.
`gpu_gl_renderer.c` stages those words into its raw sampling mirror and high
resolution framebuffer. Textured triangles carry page/depth/CLUT/UV state into
the fragment shader, which decodes indexed pixels, applies cutout/STP behavior,
modulation and blending. Texture pages are storage, not stable asset identities.

Sample locator: 4bpp, page (576,0), UV region (216,192), 40x40 texels,
CLUT (912,88). This corresponds to VRAM (630,192), 10x40 words. The live GP0
capture confirms texpage 0x9 and the corresponding UV/CLUT references.

| Texture class | Known source/render path | Evidence and limits |
| --- | --- | --- |
| World surfaces | DB group RAW -> VRAM -> textured triangles | Selected club region verified end to end |
| Actors/weapons | DB group A/B.RAW; MOD records carry UV/page/CLUT (`ttk_anim`) | Same native sampler; no new replacement gameplay coverage claimed |
| Native sprites/UI | Original GP0 textured rects reach the same triangle sampler | Mechanically eligible; no replacement sample tested |
| Host UI/fonts | Disc-derived font/icon packs and host draw paths | Outside this VRAM specimen path |
| Custom Bio Mask | Dedicated texture, reserved texpage 0x8000/depth 3 | Explicitly excluded |
| Animated/streamed textures | Updates, palettes, copies and rendered VRAM content | Content changes invalidate identity; unmatched frames use original art |
| FMV | MDEC/depth24 presentation | Outside replacement scope |

## Implementation and identity rules

`tools/local/texture_sample.py` provides capture and compile commands.
Capture uses the authoritative GL framebuffer readback, not a potentially stale
CPU VRAM snapshot. Two successive source/CLUT reads must agree. It exports an
RGB reference and explicit source words/palette/locator. Compile converts an
opaque PNG to a bounded RGB specimen; PNG remains the editor interchange.
Pillow is already in the project's local requirements. No new runtime PNG
dependency or automatic asset discovery was added.

`runtime/src/ttk_texture_sample.h` validates the little-endian specimen and
compares exact source/CLUT words. `gpu_gl_renderer.c` accepts the developer-only
`DNTTK_TEXTURE_SAMPLE` path only when `DNTTK_INPUT_MODE=modernized`.
Without the variable, original textures render normally. Missing/malformed input
logs a fallback and leaves original rendering available. Vanilla ignores it.

A page/CLUT locator is only a candidate filter. Complete region and palette
content must match before replacement is admitted. Verification reads the
authoritative GL raw mirror after pending work; matching does not overwrite
source texture words. The renderer caches this decision and invalidates it on
overlapping uploads/draws/fill segments/copies, movie framebuffer clearing,
savestate restaging and replay restoration. A changed palette, even only its STP
bit, rejects the specimen. No hash collision is possible in this byte comparison.
Identical content at the same locator intentionally shares the edit across all
surfaces using it; this is not per-object material selection.

Replacement eligibility becomes a batch boundary, preserving primitive order.
The shader retains original cutout and STP classification, substitutes RGB only
inside the selected region, then applies the original modulation/blending. UVs
retain sub-texel precision so a higher-resolution image can add actual detail.
Outside the region, or when eligibility fails, native sampling is retained.

The existing upstream `hd_texture_pack` module has upload hashes, residency,
decoding and serialization APIs, but no active GPU integration was found. Its
standalone test target does not establish title support. This bounded path avoids
introducing its pack discovery or serialized upload history: old saves can be
revalidated directly from restored texture content. Future work should reconcile
these approaches before creating a general replacement manager.

## Local workflow

With a private diagnostic game running on port 9516 at the specimen surface:

```sh
python3 recomp/tools/local/texture_sample.py capture --port 9516 --region 0 576 0 216 192 40 40 912 88 --output recomp/analysis/d16-texture-foundation/sample
```

Open `original.png` in Photon, author on editable layers, save the Photon source
and export an opaque PNG. Dimensions must be the same size or an integer uniform
multiple, at most 1024x1024. Then:

```sh
python3 recomp/tools/local/texture_sample.py compile recomp/analysis/d16-texture-foundation/sample/capture.json recomp/analysis/d16-texture-foundation/sample/edited-160.png recomp/analysis/d16-texture-foundation/sample/edited-160.ttktx
DNTTK_TEXTURE_SAMPLE="$PWD/recomp/analysis/d16-texture-foundation/sample/edited-160.ttktx" python3 recomp/tools/local/run.py
```

The environment variable is one-launch only and does not save a texture preference.
The sample is visible on the identified club panels, not every level or material.
Ordinary `python3 recomp/tools/local/run.py` uses original textures.
The private `probe.py` records the specific settings/save route for reproduction.

## Verification

- Native parser tests cover 4/8/16bpp specimens, exact source and palette changes,
  invalid dimensions/alignment, bounds, truncated/extra data and dirty-region
  overlap. These are synthetic depth tests, not 8/16bpp gameplay certification.
- The local source suite passes: 201 tests, 2 skipped. Additional dirty-text
  restore, pipeline deadline and render-worker deadline checks pass.
- Candidate builds. Final executable SHA-256:
  `2920def9780facd0375e8e51fe68daa5ef74b6ecf5993373456b1c608761646c`.
- Exported framework patch applies to the pinned clean dependency and reproduces
  all 53 modified/new framework files byte for byte. No codegen-hash input or
  save format was changed.
- Actual 40x40 and 160x160 edits rendered on the club panels at 60 Hz. Exact
  palette mismatch and missing-file probes restored original art. Vanilla and
  software runs ignored the sample and rendered original textures.
- Private 120 Hz lifecycle: save, load a different state, restore, turn; visible
  replacement retained. Initial run had 748 presents, 728 replays, zero failed
  replay sessions. This is state-switch coverage, not a natural level-transition
  playthrough. Final-build lifecycle rerun: 746 presents, 729 replays, zero failed
  sessions; replacement remains visible after restoration.
- Paired idle 120 Hz probes, forensics off: baseline 482 presents / 242 guest
  fields in 4.033 s; sample 482 / 242 in 4.029 s. Mean cadence 8.3467 vs 8.3461 ms.
  Four-second local samples establish no sustained cadence loss here, not a
  campaign performance budget or a statistically meaningful speed improvement.
- Artifact checked at desktop and narrow widths; Photon source reopened and
  layers inspected. The visual is evidence for this sample, not whole-game art
  quality certification.

## Remaining limits and review

This first path supports one non-wrapping, word-aligned region and nearest RGB
sampling. Maximum replacement size is 1024x1024; source comparison scratch is
bounded at 128 KiB plus a 256-word palette. The sample image is only 160x160.
Actual driver texture allocation was not measured. Source and CLUT storage are
bounded; decoded RGB is released after GL upload.

Texture-window draws fall back. Relocated assets need another locator; there is
no relocation search or multi-region registry. Native animation/palette changes
are preserved through fallback rather than authored frame sets. Authored alpha,
new blending semantics, software replacements, cross-renderer/campaign coverage,
dark/gas/mirror/power-up combinations and natural transitions remain unverified
or outside this first implementation. Host rendering performance at 180/240 and
Unlimited has not been measured for this feature.

The user accepted the artifact and visible club sample as sufficient for this
bounded foundation. Further cutout/animated specimens, real transitions and a
multi-region interface are deferred until expansion is selected. Mod tools,
packs, installers and distribution remain outside this accepted D16 scope.

Replacement images retain their RGB colors; compile does not quantize them to
the original CLUT. Exact CLUT matching is an identity safeguard, not automatic
recoloring. A future optional palette-constrained export could support matching
the native art style without restricting higher-color replacements. The user
provided Backrooms launch-mapster32.sh and backrooms.def as a reference for
future declarative authoring; no such parser or mod interface is added here.
