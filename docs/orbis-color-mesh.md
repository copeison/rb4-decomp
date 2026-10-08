# Orbis color mesh

The `Color` mesh specialization uses 28-byte vertices:

```text
offset 0x00  float position[3]
offset 0x0C  float color[4]
```

The vector-growth routine at `0x8DA790` establishes the layout. It initializes
each new vertex to a zero position and RGBA `(0, 0, 0, 1)`, then advances by
`0x1C` bytes. The count, resize, clear, indexed-access, and copy methods occupy
`0x8DA2B0` through `0x8DA42F`.

Backend finalization at `0x8DA430` records the logical vertex count, rebuilds
the vertex buffers at `0x8DAC80`, and rebuilds the index buffer at `0x8DAE90`.
The vertex path allocates one `VBuffer` and an optional second buffer for
dynamic meshes, copies the CPU vector, and builds eight Orbis vertex-resource
descriptors. A dirty vertex update at `0x8DA470` flips the active buffer before
copying the new data.

The index path is shared in shape with the position-only specialization. It
stores 16-bit indices while the mesh has at most 65,535 vertices and retains
32-bit indices for larger meshes. Replaced `VBuffer` and `IBuffer` allocations
enter the render system's deferred-release queue.

IDA had no functions for this specialization even though its vtables were
present at `0x195EFC8`, `0x195F038`, `0x195F060`, and `0x195F0D0`. The concrete
and base methods, secondary-interface thunks, vector helpers, and both rebuild
functions have now been separated and named.
