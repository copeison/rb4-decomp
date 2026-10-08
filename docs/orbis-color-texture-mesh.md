# Orbis color-texture mesh

The `ColorTex` specialization uses 36-byte vertices:

```text
offset 0x00  float position[3]
offset 0x0C  float color[4]
offset 0x1C  float texture_coordinate[2]
```

The vector-growth routine at `0x44B2F0` establishes both the stride and default
values. New vertices contain a zero position, RGBA `(0, 0, 0, 1)`, and a zero
texture coordinate. The specialization's format virtual at `0x8DB600` returns
format ID 1.

The CPU-vector methods span `0x8DB610` through `0x8DB78F`. Finalization at
`0x8DB790` records the vertex count and invokes the vertex-buffer rebuild at
`0x8DBE60` followed by the index-buffer rebuild at `0x8DC070`. Dirty vertex
updates at `0x8DB7D0` alternate between the two mapped GPU buffers.

The backend allocation and index-width rules match the other recovered mesh
specializations: `VBuffer` and `IBuffer` allocations use four-byte alignment,
dynamic meshes receive a second vertex buffer, and indices are narrowed to 16
bits when the vertex count is at most 65,535.

IDA function recovery now covers the concrete and abstract-base vtables,
secondary-interface thunks, vector swap/assignment helpers, and both buffer
rebuild routines through `0x8DC36F`.
