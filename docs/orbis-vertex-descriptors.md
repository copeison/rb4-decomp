# Orbis vertex descriptors

The shared helper at `0x8E1840` converts an engine mesh-format descriptor into
as many as eight 16-byte `sce::Gnm::Buffer` vertex descriptors. It walks the
ten engine attribute slots through this fixed stream map:

```text
attribute  0  1  2  3  4  5  6  7  8  9
stream     0  1  2  3  4  5  5  6  7  6
```

Attributes 5 and 6 share one stream, as do attributes 7 and 9. The layout
helper at `0x443880` merges each pair by adding its component counts. This
turns the two float2 texture-coordinate attributes into one float4 Gnm
descriptor while retaining the byte offset of the first attribute.

The engine storage codes select float32, float16, UNorm8, UNorm16, SNorm8,
SNorm16, UInt8, or UInt16 data. The recovered lookup supports two- and
four-component forms for every storage code and the float32 three-component
form. Each descriptor uses `kResourceMemoryTypeRO`, and the helper records
active descriptors in an eight-bit mask.

The format table initialized at `0x4430C0` and indexed at `0x4435E0` is also
source-owned. Its exact strides are 28 bytes for Color, 36 for ColorTex, 80
for Unskinned, 100 for Skinned, 12 for PosOnly, 52 for Particle, 52 for
UnskinnedCompressed, and 64 for SkinnedCompressed. The table records all ten
attribute slots, including unused `-1` offsets, and the recovered float32,
float16, signed-normalized 16-bit, and unsigned 8-bit storage codes. Compressed
color, texture-coordinate, and bone-weight fields use the engine's float16
storage code; packed normals, tangents, and bitangents use signed-normalized
16-bit storage.

A one-record buffer is represented with zero stride. Gnm then expects the
element byte size in its record-count field. Larger buffers use the complete
mesh stride and vertex count.

The related helper at `0x8E1BD0` builds nine descriptors over a fixed 120-byte
instance record. Its fields begin at offsets 0, 16, 32, 48, 60, 72, 84, 88,
and 104. The first three and final two are float4, the middle three are
float3, and offset 84 is a 32-bit unsigned value. Transform-related names in
the reconstruction describe the observed layout; the remaining parameter
semantics have not yet been proven.

The platform startup uses both helpers to create its built-in fallback streams.
`orbis_create_default_vertex_buffer` at `0x8D7DB0` allocates one 100-byte
skinned vertex. Its position and texture coordinates are zero, its normal,
tangent, and bitangent point along +Z, +X, and +Y, its color is white, and its
first bone weight is one. The resulting eight descriptors occupy
`OrbisRenderSystem + 0xF0C`, followed by the allocation pointer at `+0xF90`.

`orbis_create_identity_instance_buffer` at `0x8D7EB0` allocates one instance
record with identity transform and normal-transform rows. Its packed state and
two parameter vectors are zero. Its nine descriptors begin at
`OrbisRenderSystem + 0xF98`, and its allocation pointer is at `+0x1028`.
