# Orbis unskinned mesh

The `Unskinned` specialization uses an 80-byte, all-float vertex:

```text
offset 0x00  float position[3]
offset 0x0C  float normal[3]
offset 0x18  float tangent[3]
offset 0x24  float bitangent[3]
offset 0x30  float color[4]
offset 0x40  float texture_coordinate[2]
offset 0x48  float secondary_texture_coordinate[2]
```

Two independent pieces of evidence establish this layout. The format-table
initializer at `0x4430C0` records stride `0x50` and attribute offsets 0, 12,
24, 36, 48, 64, and 72. The vector-growth routine at `0x8DCCD0` zeros every
component except color alpha at offset 60, which defaults to `1.0`.

The format virtual at `0x8DC7E0` returns ID 2. CPU-vector accessors occupy
`0x8DC7F0` through `0x8DC96F`, while finalization and dirty-buffer upload are at
`0x8DC970` and `0x8DC9B0`. Vertex and index rebuilds at `0x8DD280` and
`0x8DD490` follow the same allocation, double-buffering, descriptor-build, and
16/32-bit index rules recovered for the simpler mesh formats.

IDA now contains boundaries and descriptive names for the concrete and base
vtables, both secondary-interface destructor pairs, the vector helpers, and
the backend rebuild functions through `0x8DD78F`.
