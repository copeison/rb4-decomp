# Orbis transient drawing

`orbis_render_context_draw_transient` at `0x8EA2D0` implements immediate draws
without constructing a persistent mesh. It selects one of the active frame's
eight format-specific transient buffers, appends the supplied vertices, and
uses the previous cursor as the draw's first vertex.

The binding helper at `0x8EC910` installs all eight mesh streams. Streams
present in the selected format use its transient descriptors; absent streams
use the renderer's default descriptors. The normal nine instance streams are
then bound to the default instance data.

The draw allocates embedded command-buffer memory for one 16-bit index per
vertex and fills it with the sequence `first_vertex + 0` through
`first_vertex + vertex_count - 1`. It selects the requested primitive type,
sets the index format to 16-bit with the bypass cache policy, and wraps the
indexed call with the standard Gnmx prepare/finish pair.

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
instance descriptors directly from `OrbisRenderSystem + 0xF98`.

Construction at `0x8EC7C0` clears the allocation pointer, stride, capacity,
and count. Destruction at `0x8EC7D0` releases the tracked allocation. The
render context constructs all 16 records in forward order and destroys them in
reverse order.
