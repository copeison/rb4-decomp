# Orbis skinned mesh

The `Skinned` specialization extends the 80-byte unskinned vertex to 100
bytes. Its first seven attributes are identical, followed by skinning data:

```text
offset 0x00  float position[3]
offset 0x0C  float normal[3]
offset 0x18  float tangent[3]
offset 0x24  float bitangent[3]
offset 0x30  float color[4]
offset 0x40  float texture_coordinate[2]
offset 0x48  float secondary_texture_coordinate[2]
offset 0x50  float bone_weights[4]
offset 0x60  uint8 bone_indices[4]
```

The format-table initializer at `0x4430C0` supplies stride `0x64` and the nine
attribute offsets. The final four indices occupy one packed 32-bit field. The
growth routine at `0x8DE0F0` zeroes the skinning fields and every other
component except the inherited color alpha at offset 60, which defaults to
`1.0`.

The format virtual at `0x8DDC00` returns ID 3. CPU accessors run from
`0x8DDC10` through `0x8DDD8F`; finalization and dirty updates are at `0x8DDD90`
and `0x8DDDD0`. The Orbis vertex and index rebuild paths are at `0x8DE6D0` and
`0x8DE8E0` and retain the same double-buffering, deferred-release, and index
width behavior as the unskinned implementation.

IDA now contains the concrete and base vtable methods, secondary-interface
destructors, vector helpers, and backend rebuild boundaries through
`0x8DEBDF`.
