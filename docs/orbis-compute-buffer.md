# Orbis compute-buffer factory

`orbis_create_compute_buffer` at `0x8D8BC0` is virtual slot 32 of the Orbis
render system. It allocates a 136-byte object and passes the supplied
descriptor to `orbis_compute_buffer_construct` at `0x8E3250`.

The platform constructor invokes the common compute-buffer constructor at
`0x636CC0`, replaces the vtable with the Orbis implementation, and zeros the
final 24 bytes of backend state. The public wrapper at `0x636C70` confirms the
resource identity: after calling this factory, it allocates a CPU-side block
from the descriptor's element count and stride and invokes the buffer's
initialization method.

The common object is 80 bytes. Its first 16 bytes are the render-resource base,
followed by a byte-for-byte 48-byte descriptor copy and a CPU staging pointer
at offset `+64`. The descriptor carries the element count, stride, initial CPU
data, optional external GPU address, flags, and name. The common destructor at
`0x636D10` releases the staging allocation, while the deleting destructor at
`0x636D50` also frees the object. Virtual slot 2 returns the unsigned sentinel
`0xFFFFFFFF`.

Backend initialization at `0x8E3350` first releases both GPU banks. Flag bit
`0x10` selects two banks; otherwise one bank is created. Each bank receives a
16-byte Gnm buffer descriptor using the authored element count and stride.
When no external GPU address is supplied, the backend allocates
`element_count * stride` bytes under the `ComputeBuffer` name and copies the
initial CPU data when present. Flag bit 3 supplies the special initial counter
value when there is no CPU data. Flags 0 or 3 select resource memory type
`109`; other buffers use type `16`.

The descriptor banks occupy offsets 80 and 96, their allocations occupy 112
and 120, and the active bank index is at 128. The update method at `0x8E3510`
flips that index and copies the current CPU staging range into the selected
allocation.

Binding methods at `0x8E3580` through `0x8E37CF` route the active descriptor
to every engine shader stage. Vertex binding updates both the Gnm export and
vertex stages. Pixel and compute methods use flag bit 0 to choose read/write
instead of read-only binding. Compute binding also selects the graphics CUE or
standalone compute context from the active command mode.

Destruction at `0x8E3290` defers both GPU allocations, clears their fields,
and invokes the common compute-buffer destructor. The deleting destructor
follows at `0x8E32F0`.
