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

A one-record buffer is represented with zero stride. Gnm then expects the
element byte size in its record-count field. Larger buffers use the complete
mesh stride and vertex count.

The related helper at `0x8E1BD0` builds nine descriptors over a fixed 120-byte
instance record. Its fields begin at offsets 0, 16, 32, 48, 60, 72, 84, 88,
and 104. The first three and final two are float4, the middle three are
float3, and offset 84 is a 32-bit unsigned value. Transform-related names in
the reconstruction describe the observed layout; the remaining parameter
semantics have not yet been proven.
