# Orbis transient drawing

`PS4Context::_DrawPrimitivesImpl` at `0x8EA2D0` implements immediate draws
without constructing a persistent mesh. It selects one of the active frame's
eight format-specific transient buffers, appends the supplied vertices, and
uses the previous cursor as the draw's first vertex.

The binding helper at `0x8EC910` installs all eight mesh streams. Streams
present in the selected format use its transient descriptors; absent streams
use the renderer's default descriptors. The normal nine instance streams are
then bound to the default instance data.

The draw allocates embedded command-buffer memory for one 16-bit index per
vertex and fills it with the sequence `first_vertex + 0` through
`first_vertex + vertex_count - 1`
(`GfxContext::allocateFromCommandBuffer` with `kEmbeddedDataAlignment4`). It
sets the index size to 16-bit (`GfxContext::setIndexSize`, which passes the
bypass cache policy), calls `PS4Context::SetupDraw` with the requested
primitive, and issues `GfxContext::drawIndex`, whose inline body is the CUE's
`preDraw`, the command buffer's `drawIndex`, and the CUE's `postDraw`.

The append helper at `0x8EC8C0` copies `vertex_count * stride` bytes and advances
the buffer's used-vertex cursor. Active-frame reset clears that cursor for all
eight transient formats before recording the next frame.

Each frame owns eight exact 168-byte transient-buffer records at render-context
offset `0x40DB8`, with a frame stride of `0x540`. A record contains eight Gnm
descriptors, an active-stream mask, the allocation pointer, vertex stride,
capacity, and used count. Initialization at `0x8EC7E0` allocates `0x40000`
vertices for each mesh format and builds its descriptors. Binding at `0x8EC910`
selects each active descriptor and substitutes the renderer's typed default
descriptor for every absent stream. The draw then binds the nine identity
instance descriptors directly from `PS4Device + 0xF98`.

Construction at `0x8EC7C0` clears the allocation pointer, stride, capacity,
and count. Destruction at `0x8EC7D0` releases the tracked allocation. The
render context constructs all 16 records in forward order and destroys them in
reverse order.

## Draw setup

`PS4Context::SetupDraw` at `0x8EA560` (the map's
`SetupDraw(RndContext::Primitive)`) runs before every PS4 draw. It derives the
active shader stages from `RndContext::mActiveShaderStages` at `0x4960`: bits 1
and 2 select tessellation and bit 3 the geometry shader, giving
`kActiveShaderStagesVsPs`, `EsGsVsPs`, `LsHsVsPs` or `LsHsEsGsVsPs`. It calls
`GfxContext::setActiveShaderStages` only when the value differs from the cache
at `0x44880`. Without a geometry shader it turns geometry-shader mode off
(`GfxContext::setGsModeOff`) when the flag at `0x44888` is set. Finally it maps
the primitive through `PS4RenderUtl::GetPrimitiveType` at `0x8E1770` (point
list, line list, line strip, triangle list, triangle strip; anything else is a
triangle strip) and calls `GfxContext::setPrimitiveType` only when it differs
from the cache at `0x44884`.

The PS4 backend uses the SDK 5.500 Gnm and Gnmx types directly:
`PS4Context::mGfxContexts` at `0x5728` is `sce::Gnmx::GfxContext[2]` (`0xE888`
bytes each, CUE at `+0x2E8`), and the constructor and destructor at `0x8E72B0`
and `0x8E8070` call the SDK's `GfxContext` constructor and destructor for both.
