# D17 playtest acceptance and isolated rendering follow-ups

Recorded 2026-10-04, following the 2026-10-03 candidate in
[engineering note 88](88-d17e-k-l-visuals.md). This record supersedes that
note's pending E/K acceptance and its apparent success on the bounded L route.

## User results and preserved baseline

- D17E is Accepted. Repeated corridor travel and deliberate inspection of the
  peripheral subway view no longer reproduce missing geometry. Later similar
  reports should become targeted regressions, not keep this job open.
- D17K is Accepted. Ground blood is stable, the different splatter patterns are
  clearly visible, and the user considers the result excellent. Preserve the
  source world polygon's maximum-SZ ordering key and native blood rendering.
- D17L remains open, returned to Todo for further investigation/fix. The
  specific cup still disappears/flickers slightly and seems largely unchanged.
  Other cups and the bowl on the bar are stable. Prior bounded captures did
  not establish resolution of this isolated case.
- D17C remains Done on the user's camera acceptance; no further camera
  acceptance pass is required. D17D/F shelf and closets remain Accepted.

Accepted E/K behavior is in commit `72085d7`, executable SHA-256
`151ea6aab6957eeb2c3da5cf990700d566a227eb723eb829d13600146c098520`.
This follow-up changes documentation only. No renderer code, executable,
preferences, player saves/cards or original media changed. No game was launched.

## Save identities

Read-only inspection verified the current files and copied all player files
into private `analysis/d17-high-refresh/cards-followup-20261004`. Hashes and
mtimes are recorded in `followup-20261004-manifest.json`. UI N uses file N-1.

| UI slot | Report | Current SHA-256 |
| --- | --- | --- |
| 9 | D17M ladder/platform/ammo | `6af73c5749bb71595e322d695cb1887cc1b8f8de4bfee9ffceade4700521903d` |
| 11 | D17L isolated cup, unchanged state | `267c6a268992f8468179211291031a668908dfa081c70bcf9439280004e933ef` |
| 12 | D17N vibrating walls | `1db85905d93a910ea746aacbc48d042bece5657bcd9a62fe9919dd3b690236dd` |

Slots 9 and 12 changed. Do not use the current files as evidence for the
older accepted shelf/blood routes. The accepted blood state remains in
`cards-ekl-20261003`, file 11, SHA-256
`31271d6284c52c92b449c594241b877810fd77a6bced59adfb184a2237eedc21`.
Earlier accepted shelf/closet copies are documented in note 88.

## D17L data and render-path inspection

The current UI 11 state was decoded offline using the runtime's boot-state
wire format (36-byte header, tagged sections, zlib payload for 2 MiB RAM).
No running game or writable player card was used. Mesh headers, vertices,
polygon groups and instance mesh pointers were inspected directly, alongside
the owned executable's static-object draw routine and `src/ttk/near_clip.cpp`.
Private decoded evidence is `followup-slot10-ram.bin` and
`followup-mesh-inspection.json`; neither is distributed.

| Object | Mesh/type | Actual representation |
| --- | --- | --- |
| Club rectangular tables | `0x8011baa0`, type 79 | 8 vertices, extent 1536 x 512 x 768; five textured quad faces (`0x2c`), a box with no bottom face |
| Cups | `0x80129a3c`, type 198 | 8 vertices, extent 60 x 86 x 60; five textured quad faces, tapered sides and top |
| Bowls | `0x80129880`, type 197 | 17 vertices, extent 210 x 86 x 210; eight textured quads plus eight textured triangles (`0x24`) |

These are polygonal 3D object meshes, not screen-aligned sprites. A textured
quad opcode by itself would not prove that distinction; the nonplanar vertex
sets, indexed faces and static-object transforms do. The table's visible
seams therefore do not establish a sprite-built table.

The original routine loads an instance's mesh from `s2 + 0x44` at
`0x80031e3c` and calls the object polygon renderer at `0x80032280`, returning
to `0x80032288`. Earlier runtime traces recorded these table/cup/bowl meshes
at that caller. Current instance data confirms multiple placements of the
same prototypes, rather than one special cup mesh for the failing area:

- Table-area cups at `0x801de570`, `0x801de870`, `0x801de8d0` share the cup
  mesh and type with bar-row cups `0x801de930`, `0x801de990`, `0x801de9f0`.
  Table-area centers have Y=-9726; the bar-row centers have Y=-9841.
- The table-area bowl `0x801de750` and bar bowl `0x801de7b0` share the bowl
  mesh/type as well, at those respective heights.
- Table instances include `0x801de5d0` at (10795,-9427,-1990) and
  `0x801de6f0` at (11261,-9427,-4101), spatially matching the table-area
  cup/bowl groups. Other table instances are `0x801de630` and `0x801de690`.
  Do not assume the supporting table and a foreground occluding table are
  the same instance in the reported view.

This rules out a different cup prototype or sprite-based table as the
established explanation. It does **not** establish the residual flicker's
cause. The old trace deduplicated by mesh pointer, so it did not identify
which individual cup vanished or the exact overlapping face in a bad frame.
The bar's supporting surface has not yet been matched to an emitted primitive.

Current code gives all qualifying compact static cups/bowls host depth beyond
the usual 3072-unit boundary. Tables exceed the 256-unit compact limit and
retain per-polygon native/enhanced selection based on projection and distance.
Thus different support/foreground geometry and ordering can still matter,
even when cup meshes are identical. This is a source-supported mechanism to
investigate, not proof that it causes this remaining report. Do not broaden
all-object takeover or change the accepted floor ordering on that assumption.

Next D17L implementation work must correlate the particular disappearing cup
instance, its supporting and foreground surfaces, and native/host packets
across consecutive good/bad frames on approach/retreat. Check culling, fade,
face rejection, clipping, depth and ordering against the stable bar objects.
Capture the actual residual failure before declaring a repair; preserve the
accepted regression routes and rendering cost limits from note 88.

## Separate backlog reports

**D17M - UI 9:** begin climbing the ladder; shotgun ammo shows through its
platform. The platform should occlude it from below. Cause remains unclassified:
inspect culling, depth/occlusion, pickup representation and render order when
this job is selected. This report does not reopen D17D's old UI 9 shelf fix.

**D17N - UI 12:** repeatedly walk forwards/backwards, watching the wall ahead
and nearby walls. Jagged diagonal artifacts make static surfaces seem to
vibrate. Cause remains unclassified: trace source polygon/triangle boundaries,
clipping, interpolation, precision and native PS1 geometry behavior. Target
substantially steadier surfaces while retaining the chosen PS1 character.
Neither new backlog job has been implemented or runtime-tested here.
