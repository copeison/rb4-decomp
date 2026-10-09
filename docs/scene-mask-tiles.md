# Scene-mask tiles

`RndBufferCollection::_AllocSceneMaskTileBuffers` at `0x6B2140` creates two `Scene Mask`
render targets and a position-only `Scene Mask Mesh` when target-resource flag
`0x4000` is set. Their extent is the output width and height divided upward by
the renderer's 32-pixel light tile size.

The mesh contains one clip-space quad per tile: four 12-byte position vertices
and two triangles with indices `(0, 1, 2)` and `(1, 3, 2)`. Vertex positions
span `-1` to `1` across the target, with the vertical axis descending from
`1` at the top to `-1` at the bottom. Tile edges are computed in pixels and
clamped to the original target extent, so a partial final tile maps to the
correct clip-space edge. The mesh is finalized after all vertices and
triangles are populated.

The two targets occupy owner offsets `0x270` and `0x278`; the mesh occupies
`0x280`. Target creation accepts the matching resources from a prior owner for
reuse. The corresponding portion of `RndBufferCollection::Destroy` at
`0x6AFFE0` virtually deletes and clears all three slots.
