# D08K — original animation and collision audit

2026-09-27. Investigation complete; safe true crouch-walk prototype blocked.
No original assets or existing crouch/roll behavior were changed.

The owned SLUS-00583 executable and DB00ANM/FRM/ROT/HIE and LEVEL00 collision
assets were inspected. The local read-only decoder and full derived evidence are
[audit_crouch_assets.py](../recomp/tools/local/audit_crouch_assets.py) and
`recomp/analysis/iteration42/asset-audit.json`. They record
asset hashes, track mappings, frame events/durations, root translation and packed
joint components. Original readers 800593b4, 80058c70 and 800597e4 establish the
ANM start/count pairs, eight-byte FRM records and eight-byte ROT commands. ROT
word0 bit29 selects translation; word1 top five bits select the joint. Rotation
components are consistent with Q12 quaternions, but a complete skeleton importer
and round-trip animation override have not been validated.

| Original animation | Four resident clips | Finding |
| --- | --- | --- |
| 176 | 109/111/112/110 | Crouch transition/reverse |
| 178 | 105/106/107/108 | Stationary crouch |
| 179 | 417/418/419/420 | Stationary low pose |
| 181 | 277/279/280/278 | Forward full-body roll, 10 frames, 250 ticks |
| 182 | 281/283/284/282 | Backward roll, 10 frames, 260 ticks |
| 183 | 269/271/272/270 | Side roll, 11 frames, 250 ticks |
| 184 | 273/275/276/274 | Opposite side roll, 11 frames, 250 ticks |

The roll clips have substantial root travel and joint-zero rotation spanning a
full tumble, not alternating planted feet. Live private Software captures under
`analysis/pc-input/iteration42-edge-candidate/crouch-*` confirm the visible tumble.
Modern camera-relative direction turns Duke toward travel, so all four tested
keys entered original forward roll181; this is not live proof of all four original
roll variants. All recovered to178. The final release stayed178 in that apartment
position: safe stand-up clearance still applies; do not call that unrestricted
stand-up coverage.

The 181/182 handler80054308 calls movement collision8007926c; 183/184 use80054218.
That collision route contains fixed standing clearance0x370 and head-ray offsets
at800794d0,80079530,800795dc. Existing stand-up checks alone cannot make a moving
low hull safe. Slowing a roll leaves tumbling; translating a crouch idle leaves
sliding feet; lowering only the model leaves standing collision. None meets the
requested prototype criteria.

A safe next implementation needs (1) a validated HIE joint/mesh mapping and pose
preview/round-trip local override, (2) an authored or validated blended low gait
with coherent feet/root travel and original event semantics, (3) bounded low-height
movement/headroom probes and safe stand-up, then (4) low ceiling, stairs/edges,
weapon/camera/traversal interruption tests. The original media must remain read-only.
This is a concrete animation-import and variable-height collision dependency,
not a claim that new animation is impossible. No new binding was introduced.
