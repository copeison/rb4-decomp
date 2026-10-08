# Orbis mesh factory

`orbis_create_mesh` at `0x8D85F0` is virtual slot 22 of the Orbis render
system. Every supported format allocates a 472-byte mesh, invokes the common
constructor at `0x5C2700`, installs the format-specific Orbis vtables, and
zeros the shared backend state.

The common mesh base is exactly 128 bytes. It owns an update-listener link, a
vector of 12-byte triangle records, vertex and triangle counts, geometry and
usage state, an all-ones metadata sentinel, a pending-update mask, the last
frame that used the mesh, and its diagnostic name. The complete destructor at
`0x5C27B0` releases the triangle vector and unregisters the update link; the
deleting destructor is at `0x5C2870`. Secondary-base destructor entries at
`0x5C2810` and `0x5C28D0` account for the update-link subobject at offset 8.

The typed base lives in `src/render/core/meshes/render_mesh.cpp`. The Orbis
layout now embeds it directly before the platform vertex arrays and GPU
descriptors instead of representing the first 128 bytes as padding. Combined
IDA evidence is preserved in `analysis/exports/render-mesh.asm` and
`analysis/exports/render-mesh.c`.

The format-name parser at `0x442930` fixes the enum values:

| Value | Format | Factory result |
| ---: | --- | --- |
| 0 | `Color` | Orbis mesh |
| 1 | `ColorTex` | Orbis mesh |
| 2 | `Unskinned` | Orbis mesh |
| 3 | `Skinned` | Orbis mesh |
| 4 | `PosOnly` | Orbis mesh |
| 5 | `Particle` | null |
| 6 | `UnskinnedCompressed` | Orbis mesh |
| 7 | `SkinnedCompressed` | Orbis mesh |

Particle geometry uses the separate particle-buffer factory. Generic callers
populate the mesh's triangle and vertex arrays, select a format descriptor
from the table at `0x4435E0`, and then finalize the backend data. Recovered
call-site names include `Arrow Mesh`, `Text`, `Scene Mask Mesh`, and
`vocal_tube_mesh`.

The position-only specialization is reconstructed in
`src/render/platform/orbis/meshes/orbis_position_mesh.cpp`. Its CPU vertex-vector accessors, double-buffered
GPU update path, index-width selection, and missing IDA method boundaries are
documented in `docs/orbis-position-mesh.md`.

The 28-byte position-plus-float4-color specialization is reconstructed in
`src/render/platform/orbis/meshes/orbis_color_mesh.cpp` and documented in
`docs/orbis-color-mesh.md`.

The 36-byte `ColorTex` specialization adds a float2 texture coordinate. It is
reconstructed in `src/render/platform/orbis/meshes/orbis_color_texture_mesh.cpp` and documented in
`docs/orbis-color-texture-mesh.md`.

The 80-byte unskinned specialization carries four float3 basis vectors, a
float4 color, and two float2 texture coordinates. It is reconstructed in
`src/render/platform/orbis/meshes/orbis_unskinned_mesh.cpp` and documented in
`docs/orbis-unskinned-mesh.md`.

The 100-byte skinned specialization appends float4 bone weights and four
packed bone indices. It is reconstructed in `src/render/platform/orbis/meshes/orbis_skinned_mesh.cpp`
and documented in `docs/orbis-skinned-mesh.md`.

The 52-byte compressed unskinned specialization retains float positions and
packs the remaining six attributes into 16-bit components. It is reconstructed
in `src/render/platform/orbis/meshes/orbis_unskinned_compressed_mesh.cpp` and documented in
`docs/orbis-unskinned-compressed-mesh.md`.

The 64-byte compressed skinned specialization adds packed bone weights and
four packed bone indices. It is reconstructed in
`src/render/platform/orbis/meshes/orbis_skinned_compressed_mesh.cpp` and documented in
`docs/orbis-skinned-compressed-mesh.md`.
