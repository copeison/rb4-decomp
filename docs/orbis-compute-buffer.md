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
