# Orbis mesh draw path

All seven general Orbis mesh classes contain the same draw body. The copies
begin at `0x8D8E00`, `0x8D9FB0`, `0x8DB310`, `0x8DC4F0`, `0x8DD910`,
`0x8DED60`, and `0x8E0200`. The cleaned reconstruction expresses that body
once as `PS4MeshTyped<T>::_DrawBatchImpl`.

The draw first binds mesh vertex streams 0 through 7. A bit in the mask at
mesh offset `0x1C0` selects the descriptor from the active mesh buffer;
missing streams use the render system's default descriptors. The active mesh
buffer selects one of two eight-descriptor banks at offsets `0xA0` and
`0x120`.

Instance records are copied into the active `GfxContext`'s embedded-data ring.
Nine temporary descriptors expose the fields of each 120-byte record in
slots 8 through 16. The draw then sets the instance count and calls
`PS4Context::SetupDraw` with the triangle primitive (see
[orbis-transient-draw.md](orbis-transient-draw.md)).

An index buffer selects the indexed path. Draw ranges are measured in
triangles, so both the requested triangle count and first-triangle offset are
multiplied by three. Format 0 advances by two bytes per index and format 1 by
four bytes. A range count of `-1` uses the mesh's complete triangle count.
Meshes without an index buffer issue `drawIndexAuto` with the vertex count.

Both paths use `GfxContext::drawIndex` or `drawIndexAuto` (the CUE's
`preDraw` and `postDraw` around the packet), restore the instance count to one, and record the current
render frame at mesh offset `0x70`.
