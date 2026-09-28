# Validation matrix

## Automated local tooling

From the workspace root:

```bash
DNTTK_IMAGE="$PWD/Duke Nukem - Time to Kill [U] [SLUS-00583]/Duke Nukem - Time to Kill [U] [SLUS-00583].img" python3 -m unittest discover -s recomp/tests/local -v
```

Without `DNTTK_IMAGE`, media-dependent integration tests skip and synthetic parser tests still run. Build `sector_check` before enabling integration tests. No test modifies original media: deliberate corruption uses a sector copied into a temporary directory.

Covered cases:

- Valid ISO record enumeration and payload access.
- Truncated/empty image rejection.
- Wrong primary descriptor rejection.
- Extents outside image and negative read bounds.
- Unsafe filenames and cyclic directory references.
- Inconsistent dual-endian extent fields.
- Unsupported input rejection before preparation.
- Exact local disc/executable identity, inventory counts, and candidate table counts.
- A known-good real sector, single-byte payload corruption, P-parity corruption, Q-parity corruption, and truncated sector detection.

Initial result: all 13 tests passed. See `logs/tests.log`. Exact-byte code generation and disc parity checks are separate from these tests.

## Build evidence

Record compiler, source pins, configuration, generated marker, binary identity, and complete log. A setup-host build can link without generated game code; verify the configure log explicitly says it links the generated game C. Do not infer Windows success from Linux compilation.

## Runtime acceptance ladder

| Stage | Required evidence |
|---|---|
| Process start | Correct config, disc, and BIOS selected; no immediate fatal error |
| Boot | Guest frame counter progresses; captures show expected boot content |
| Menu | Title/menu visible; controlled input changes menu selection |
| Gameplay | Player appears in a level and responds to movement/actions |
| Playable slice | Movement, jump, fire, damage, restart, audio, and save/load exercised |
| Progression | Level transitions, bonus/shared overlays, bosses, campaign completion |
| Platform support | Native run of that platform's build; input, presentation, audio, exit |
| Native coverage | Runtime-compiler state and interpreter/fallback counters measured for the exercised route |

A boot movie with zero resident dispatch misses is not evidence that level overlays are all native. The overlay loader's inactive state also does not prove there is no interpreter execution elsewhere. Capture dirty-code/dispatch counters appropriate to each code path.

## Comparison protocol

1. Use the exact same image in a reference emulator and recomp.
2. Start from blank, separate cards and default original timing.
3. Record the input route and relevant frame/scene checkpoints.
4. Compare camera, position, collision, health, inventory, progression, audio timing, and representative frames.
5. Explain legitimate display differences before relaxing comparisons; do not use pixel tolerance to conceal logic divergence.
6. Test transitions back into previously loaded areas to expose stale overlay dispatch.

No reference-emulator comparison has yet been performed. No emulator was preinstalled in PATH during initial inspection.

## Regression boundaries

Keep the disc/profile, BIOS, compiler, renderer, input route, and runtime commit fixed when investigating one issue. Change one setting at a time, preserve failing evidence, and label each run. A clean exit through the TCP debug command verifies that command path, not the window-close path.

Future first-person tests must include a disabled-feature baseline, ceilings/doorways, room transitions, ledges, crouching/swimming/climbing if applicable, projectile alignment, and scripted cameras. Save compatibility remains a separate test.
