# Orbis position-only mesh

The position-only mesh selected by `RenderMeshFormat::kPositionOnly` stores
12-byte vertices containing three floats. Its CPU vertex vector begins at
object offset `0x80`; the two mapped Orbis vertex buffers are at `0x1A0` and
`0x1A8`, and the active-buffer index is at `0x1B0`.

The recovered virtual interface is:

| Address | Behavior |
| --- | --- |
| `0x8D90F0` | Return format ID 4 (`PosOnly`). |
| `0x8D9100` | Return the CPU vertex count. |
| `0x8D9130` | Resize the CPU vertex vector. |
| `0x8D9180` | Clear and release the CPU vertex vector. |
| `0x8D9210` | Return the vertex at an index. |
| `0x8D9220` | Make a 4-byte-aligned allocation tagged `VerticesCopy`. |
| `0x8D9280` | Record the count and rebuild vertex and index buffers. |
| `0x8D92C0` | Apply a pending vertex update to the alternate GPU buffer. |

`orbis_position_mesh_rebuild_vertex_buffers` at `0x8D9920` keeps the GPU
allocation capacity synchronized with the CPU vector. It allocates one or two
buffers depending on the mesh flags, uploads the active copy, and produces the
eight vertex-resource descriptors consumed by the draw path.

`orbis_position_mesh_rebuild_index_buffer` at `0x8D9B30` chooses the hardware
index width from the vertex count. Meshes with at most 65,535 vertices use a
16-bit `IBuffer`; larger meshes keep the original 32-bit indices. Both GPU
allocations use four-byte alignment and are retired through the render
system's deferred-release queue when replaced or destroyed.

The class has a secondary interface at object offset eight. IDA originally
treated the complete `0x8D8C80`-`0x8D95D4` implementation as data because the
ELF lacks usable RTTI and function metadata there. The primary and secondary
destructors, all eleven primary virtual methods, and the abstract base variants
have now been restored as individual functions.
