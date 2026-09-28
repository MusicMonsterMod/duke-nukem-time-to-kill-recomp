# Disc identity and integrity

## Original inputs

Drop the owned USA dump in `game/` (created on first run of `run.py` or `build.py`). Accepted layouts: Redump `.cue` + MODE2/2352 `.bin`, CloneCD `.img`/`.ccd`/`.sub`, or a raw MODE2/2352 `.bin`/`.img`. Europe/PAL `SLES-01515` is a different SKU and is rejected. All tooling opens those files read-only. The importer creates a separate raw BIN copy under the ignored `recomp/disc/` directory; it does not rename, repair, or normalize the source image.

| Property | Value |
|---|---|
| Serial / ISO volume | SLUS-00583 / SLUS00583 |
| Image bytes | 449,772,960 |
| Sector bytes / count | 2,352 / 191,230 |
| Subchannel bytes | 18,358,080 = 191,230 × 96 |
| Sessions / tracks | 1 / 1 |
| Mode | Mode 2, unscrambled, track index at image sector zero |
| Files | 548; eight have zero length |
| Form 1 / Form 2 sectors | 126,372 / 64,858 |

Image SHA-256 (CloneCD dump used for local bring-up):

```
230a34c2512c6db1708657f9452c42cc4b0db95caefd7ebd32035d8ba9933f5d
```

Image MD5: `52783a66e40cb36b551d496ff9a1b318`

Image SHA-1: `59a03640ea2d80c6fac7f63dd8cf17e5a75de787`

Redump USA `.cue`/`.bin` (same `SLUS_005.83`): SHA-256 `708c040436c4a8bfe0ef43379e934172d0b94131ee3db47e406767c9d306a8c1`, MD5 `82d6ef06544ac72aab26ba8b94345f8e`, SHA-1 `a782825520939da0560ef031daa78db8ef720957`. The importer accepts either image when the executable hash matches.

Subchannel SHA-256 (CloneCD `.sub` only): `faf5edc202d4fc0bfb4af5441c3ee73b7a8153ec6dd6f84b30796b2e3c82f3aa`

Descriptor SHA-256: `3aa9cdccfb6b6995767f194c8a40c235260f757033267e32f03ea3b092014621`

## Difference from upstream

The upstream disc probe records the same image length, serial, executable layout, and executable SHA-256. Its whole-image MD5 is `82d6ef06544ac72aab26ba8b94345f8e`, which is the Redump USA dump now also accepted. The CloneCD image remains a second known-good copy of the same `SLUS_005.83`. Whole-image bytes still differ between those two dumps; sector parity on either does not prove they are identical masters. Europe/PAL `SLES-01515` matches the USA image length but is a different SKU and is rejected. Do not replace identity checking with size-only matching or `--skip-hash-check`.

## Validation actually performed

`tools/local/sector_check.c` scans every sector. It checks sync bytes, Mode 2, duplicate XA subheaders, EDC, and Form 1 P/Q error-correction parity. Mode 2 ECC calculation temporarily zeros the sector address/mode bytes in an in-memory buffer as required by that layout. It never writes the image.

Observed: zero invalid sectors, zero bad EDC, zero bad P/Q parity, zero missing Form 2 EDC, zero trailing bytes. The checker supports reporting zero Form 2 EDC separately; this image did not use it.

The ISO reader independently checks primary-descriptor identity, volume length, file extent bounds, directory record lengths, repeated/cyclic directory extents, unsafe names, and paired little-/big-endian size and extent fields.

The subchannel file's length and identity were recorded. Its internal Q-channel CRCs and other subchannel semantics have **not** been decoded. Sector validation also does not establish semantic correctness of game data or a reference-mastering match.

## Why retain raw sectors?

Mode 2 Form 1 exposes 2,048 logical bytes per sector. Form 2 carries 2,324 payload bytes. This disc uses XA audio, including eight interleaved channels in `MUSIC1.IDF` and audio sectors within movie extents.

A naive 2,048-byte ISO conversion would omit audio payload bytes and XA file/channel/coding metadata. The runtime input therefore remains a raw 2,352-byte-per-sector BIN with a `MODE2/2352` CUE. The original SUB remains preserved separately; CUE import does not convey its contents to the runtime.

In the manifest, `logical_sha256` means the ISO reader's first 2,048 payload bytes per sector, truncated to the directory size. `raw_extent_sha256` includes complete raw sectors covering the extent. For media identity, prefer the raw hash. Sector-rounding bytes are included in raw hashes by design.

## Reproduction

From the workspace root:

```bash
cc -O2 -Wall -Wextra -Werror recomp/tools/local/sector_check.c -o recomp/build-tools/sector_check
recomp/build-tools/sector_check game/your-dump.bin
python3 recomp/tools/local/disc_lab.py inspect game/your-dump.cue --output documentation/reports/disc-manifest.json
python3 recomp/tools/local/disc_lab.py import --output recomp/disc --validator recomp/build-tools/sector_check
```

Use the actual `.cue` or `.img` path under `game/` for inspect and sector_check. Import with no image argument searches `game/` itself.

The portable build wrapper uses CMake to compile the checker instead of assuming `cc` on Windows.
