# GBuffer targets

`RndBufferCollection::_AllocGBuffer` at `0x6B3040` creates full-resolution
`GBuffer Color` and `GBuffer Pixel Normals` targets. It also creates
`GBuffer Vertex Normals` when `RndConfig::use_gbuffer_vertex_normals` is
enabled. The targets occupy offsets `0x28`, `0x30`, and `0x38` in each
216-byte per-scene resource block.

Each target can reuse the corresponding resource from a previous block. The
primary scene registers non-null targets with the enclosing owner; partial
frames retain them through their block. The matching portion of
`RndBufferCollection::Destroy` at `0x6AFFE0` invokes the virtual deleting
destructor for each non-null target and clears all three slots.

The target format and descriptor assembly remain behind a narrow adapter
until the common render-target descriptor is reconstructed.
