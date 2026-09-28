# D08M / D08O — swim redesign notes

Canonical research: [55-swim-controls-research.md](55-swim-controls-research.md).

## D08M — accepted foundation (closed)

| Item | Status |
| --- | --- |
| Shallow-water jump height (`8003e2d0` 97/98) | Done |
| Space-facing shallow ledge hop (anti-bounce) | **Done** — user closed; revisit only if needed |
| Deep Space ascend to surface | Works |
| Deep W / S | Works |
| E / mantle exit | Works |
| Research artifact | Stood up in `55` |

Binary: `47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`.

## Iteration 9 — root/input bridge (shipped baseline)

| Rule | Behavior |
| --- | --- |
| Water ownership | Original `+0x20c` only |
| Fall VY | Never cleared on entry |
| World XYZ | Not used for swim locomotion |
| Free swim horizontal | D-pad + stick tables + `+0xfc/+0x100` rewrite |
| Free swim vertical | Suppress Square; soft Y Space↑/Ctrl↓; surface clamp |
| Wade ledge | Longer water holdoff + XZ reassert |

## Iter 9 playtest (deep water)

| Check | Result |
| --- | --- |
| Space → surface | Pass |
| W forward / S back | Pass |
| A/D strafe | **Fail** |
| Ctrl descend | **Fail** |
| E mantle out | Pass |
| Space over ledge exit | Pass (to be **removed**) |

## D08O — implemented 2026-09-28 (all rounds accepted; D08O Done)

| Item | Root cause | Fix | Status |
| --- | --- | --- | --- |
| A/D strafe | A/D injected D-pad Left/Right = **turn** (71/70); stick X also turn | Inject the layout's strafe words (L2/R2 via `800d1a68`/`800d1b80`, resolved per pad); stop writing stick X | Accepted playtest 1 (pad-injection proof: L2→91, R2→90 sideways) |
| Ctrl descend | Gate required body already >0x100 under surface | Gate on floor margin `+0x1c8 − 0x1e0`; clamp `surface + 0x1800`; Space still ascends | Failed playtest 1 → superseded by the native dive below (accepted) |
| Mantle-only exit | `swim_try_mantle` hopped out on Space at a lip | Removed; free-swim Space = ascend only; E / original mantle exits | Accepted playtest 1 |
| Shallow regression | — | W/S/A/D wade, Space 98, Ctrl 105, heading stable on subway state | Verified |

Binary: `d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e`.

## D08O playtest 1 → rounds 2–3 (2026-09-28, accepted playtest 3)

Playtest: A/D strafe **Pass**, Space ascend **Pass**, mantle exit **Pass**,
Ctrl **Fail** (dead), shallow → subway platform jump **unreliable** (regression
reported → ledge hop reopened).

| Item | Root cause | Fix | Status |
| --- | --- | --- | --- |
| Ctrl dive | Native swim states (`800455bc`, `+0x22c` 4/5) pin body Y to the surface each frame (`80044c18`); soft-Y cannot start a dive | Ctrl calls original dive `80045564` → state 5 / anim 133; underwater host writes body yaw/pitch from WASD/Space/Ctrl (camera-relative) and injects Square for the original thrust; auto-surface original | Verified in zone 35 by teleport; **accepted** playtest 2 |
| Wade ledge jump | 98 rises ~560 vs ledge 635; wall-touch → frame-one cancel (splash); 300–600 → 107 bump, 105 fall-back; only 700–1250 landed; probe reach ~350 so `swim_climb_ahead` never saw the ledge | `swim_ledge_ahead` (probe from 0/320/…/1280 along view, XZ restored) → directed 98 with VY −9000 at the ballistic hook (rise ≈840 vs 560); XZ held at 0 until the feet clear the lip within one probe step of the wall, then 3400/4000; else 1900 + 3.5·distance (max 6400). Round-2 vertical 97 → 98 dropped as far too high (user) | 15/15 lab landings; **accepted** playtest 3 |
| Plain wall + W+Space | Directed 98 into a wall is cancelled by the original (splash) | Vertical hop 97 when the forward probe is refused and no ledge is found | Verified |
| Shallow regression | — | W/S/A/D wade, Space 98 in open water, Ctrl 105 | Verified |
| Deep regression | — | Dive, 4-way thrust, look-up rise, Ctrl descend, Space surface, 125/123 surface swim | Verified |

Binary: `78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.
