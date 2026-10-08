# Orbis mesh factory

`orbis_create_mesh` at `0x8D85F0` is virtual slot 22 of the Orbis render
system. Every supported format allocates a 472-byte mesh, invokes the common
constructor at `0x5C2700`, installs the format-specific Orbis vtables, and
zeros the shared backend state.

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
`src/render/orbis_position_mesh.cpp`. Its CPU vertex-vector accessors, double-buffered
GPU update path, index-width selection, and missing IDA method boundaries are
documented in `docs/orbis-position-mesh.md`.

The 28-byte position-plus-float4-color specialization is reconstructed in
`src/render/orbis_color_mesh.cpp` and documented in
`docs/orbis-color-mesh.md`.
