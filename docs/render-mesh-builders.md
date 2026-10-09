# Render mesh builders

`RndMeshUtl` (`render/RndMeshUtl.o`) builds procedural meshes. In the source it
is a namespace, which mangles the same as the map's names.

| Function | Address | Builds |
| --- | --- | --- |
| `CreateBox` | `0x5DCBA0` | a box from three axes and segment counts |
| `CreateSphere` | `0x5DD0E0` | a UV sphere |
| `CreateCylinder` | `0x5DDC00` | a capped cylinder |
| `CreateRadialSurface` | `0x5DD5E0` | a surface swept from a contour |
| `ReshapeRadialSurface` | `0x5DF180` | an existing radial surface |
| `CreateTruncatedRoundedCone` | `0x5E0C10` | a capped cone |
| `CreateQuad` | `0x5DB700` | a segmented quad |
| `CreateFacingQuad` | `0x5DC240` | a unit quad facing one of six axes |
| `CreateTriangleFan` | `0x5DC380` | a fan facing one of six axes (name not in the map) |
| `CreateCapsule` | `0x5DE890` | a capsule swept from a contour |
| `CreateNestedCone` | `0x5E0240` | two cones joined at the rims (name not in the map) |

Each builder takes a parameter block that starts with `CreateMeshParams`: the
name, vertex type, flags and usage flags, and an offset. The flags are:

| Flag | Effect |
| --- | --- |
| 1 | skips `SyncStatic` |
| 2 | makes the mesh double-sided |
| 4 | alternates the quad diagonals |
| 8 | circumscribes a sphere around its tessellation |
| `0x10` | keeps the faces |

Each builder finishes in `_CreateMeshCoda`, which computes the bounding sphere.
That bounding sphere is now `RndMesh::mBoundingSphere` at `+0x5C`.

Two quirks are reproduced from the binary:
- In double-sided mode, the radial and quad builders give the second side the
  first side's indices and winding. This is probably an original bug.
- The builders share two scratch vectors, `gTmpContour` and
  `gTmpUVIntervals`.

## Users

- **`RndPrimitiveMeshes`** (`0x460640`) creates the default "Box" and
  "Cylinder" meshes with `CreateBox` and `CreateCylinder`.
- **`RndLightGlobals::_InitMeshes`** builds the 16 × 8 "lighting_sphere" with
  the circumscribe flag. Its scale is
  `(1 / Sine(π/16 + π/2)) / Sine(π/16 + π/2)`, which undoes the
  tessellation's shrinkage.
- **`_InitLightSpotMesh`** builds a cone with a half-angle of π/4, then moves
  every vertex onto the unit circle at z = 0. The vertex colours record where
  each vertex sat on the cone. The cone's segment counts are
  `mSpotlightSegments` and `mSpotlightCapSegments`.

## Not reconstructed

These are declared only:
- `RndMeshUtl::ComputeBoundingSphere`, which takes a transform in this build;
- `Sine`, `Half::Set`, and the `TruncatedRoundedCone` constructor and setter.
