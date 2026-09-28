# Executable, assets, and overlay research

## Resident executable

`/SLUS_005.83;1` starts at LBA 175345 and is 768000 bytes. Its 2048-byte header declares a payload of `0xBB000` bytes loaded at `0x80010000`, ending at `0x800CB000` exclusive. Entry is `0x800AB6FC`; initial stack is `0x801FFFF0`; header GP and BSS fields are zero.

Executable SHA-256:

```
b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a
```

The header's zero BSS size does **not** mean there is no uninitialized global memory. Startup instructions explicitly clear memory beginning at `0x800CE0B0` up to `0x800E81E8`. This was decoded with Capstone from the original executable. Continue tracing startup to recover heap and stack setup before treating the header as the entire memory map.

The executable is not wholly compressed: startup, game code, SDK diagnostics, and substantial data are directly visible. This does not rule out compression within individual asset formats.

## Function seeds and generation

The upstream seed list contains 1587 addresses. Every seed is aligned and inside the executable payload. Every seed except the entry point appears as a direct JAL target in a linear scan. That is consistency evidence, not proof of function boundaries: data words can look like calls and indirect targets can be missed.

The initial generator emits 67 game shards and a 3833-entry dispatch table. It also reports reserved-opcode/data-as-code warnings and out-of-function control flow. Preserve those warnings. Do not silence them by inventing function boundaries or declaring arbitrary memory executable.

The original payload includes mixed code/data and SDK routines. A proper Ghidra pass should map these regions, validate direct and indirect callers, and establish return conventions and delay-slot effects.

## Useful diagnostic anchors

| Meaning suggested by string | String address | Candidate referencing instruction |
|---|---|---|
| Camera start room invalid | `0x800137A0` | `0x8003ADB4`, `0x8003B448` |
| CD error | `0x80012FE8` | `0x8001BDFC` |
| Launchpoint allocation | `0x8001324C` | `0x80023DC8`, `0x8002438C` |
| Align with collision object | `0x80016CC0` | `0x8007EC88` |
| Align with object | `0x80016CEC` | `0x8007EE3C` |
| Align with edge | `0x80016D08` | `0x8007F0CC` |
| Update Duke matrices | `0x8001759C` | `0x80097CE0` |

These addresses identify LUI reference candidates found by a short linear instruction search. They are **not** verified function starts or hook points. Branches, intervening register writes, and delay slots must be checked in the real control-flow graph.

## Resident file extent table

453 nonempty directory entries match aligned `(LBA - 23, size)` pairs inside the resident payload. Matches occupy addresses from `0x800BCC00` through `0x800BDA60` (last matching pair start).

Examples:

- `MOVIE.OVR`: LBA 23959, size 6884; pair begins at `0x800BD960` with first value 23936.
- `LEVEL00.OVR`: LBA 645, size 9668; pair begins at `0x800BD968` with first value 622.

33 overlay file occurrences match this pattern. The unmatched overlay occurrences are shared BASEOVR/BONUS families. This is a reason to investigate another descriptor/selection path, not to assume those files are unused.

Next steps: find instruction references to the table bases; trace how the sector offset is converted to an absolute CD location; trace destination pointers and read lengths; record whether the four-byte overlay tag is loaded, skipped, or interpreted; identify entry dispatch and cache-flush behavior.

## Overlay observations

There are 50 `.OVR` files but only 30 unique byte sequences. Shared families include:

- 360-byte `BASEOVR`: DB07–DB13, identical.
- 1420-byte `BASEOVR`: DB15–DB20 and DB31, identical.
- 22084-byte `BONUS`: DB21–DB26, identical.
- 32588-byte `BASEOVR`: DB27–DB30, identical.

The first word often looks like a small tag (e.g. 3 in LEVEL00, 29 in MOVIE). It is not a proven address or format version. Some files begin with strings or pointer tables before code. `LEVEL10.OVR` is 8266 bytes, so blindly rounding every overlay down to a multiple of four would discard real stored bytes.

Raw calls in overlays target both resident addresses and addresses beyond the resident payload. This supports streamed-code analysis, but exact loading bases and coexistence rules remain to be proven.

## Asset families

The directory names strongly suggest the following roles, pending format decoding:

| Family | Working interpretation | Evidence / caution |
|---|---|---|
| `DBxxA.RAW`, `DBxxB.RAW` | Texture/pixel banks | Repeated 229376 / 262144-byte sizes; no ordinary image header |
| `TEXPOS.DAT` | Texture placement/UV metadata | Repeated 3696-byte records per asset group |
| `DBxxMOD.BIN` | Model data | Naming and structured integer/pointer-like data |
| `HIE`, `ANM`, `FRM`, `ROT` | Hierarchy, animation, frames, rotations | Naming, repeated record structures; encoding unverified |
| `LEVELxx.BIN` | World/room geometry | Paired with collision data in many groups |
| `LEVELxx.COL` | Collision data | Naming and coordinate-like values |
| `DBxxAI.BIN` | AI/path data | Structured records; some valid empty files |
| `DBxxLP.BIN` | Launchpoint/object placement | Naming plus executable allocation diagnostics |
| `SND.DAT` | Per-group sound bank/control data | Structured header and substantial payload |
| `.STR`, `.BS` | Movie/still MDEC-family media | Stream signatures and movie overlay diagnostics |
| `.IDF` | XA audio extents | Raw subheaders identify audio and channel interleaving |

Do not write exporters based on names alone. Recover each reader, then test a parser against counts, bounds, coordinate scales, and multiple groups. DB00–DB31 are asset-group identities, not a verified campaign ordering.

`CDFIX.DAT` is 30 MiB of zero logical data. Preserve it in the runtime image because layout and extent assumptions can still depend on its presence.
