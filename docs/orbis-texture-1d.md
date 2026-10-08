# Orbis 1D texture factory

`orbis_create_texture_1d` at `0x8D8980` is virtual slot 23 of the Orbis
render system. It allocates a 408-byte object and passes the supplied texture
descriptor to `orbis_texture_1d_construct` at `0x8E4F60`.

The platform constructor invokes the common 1D texture constructor at
`0x6F5870`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x6F5810` assigns
descriptor type 0 and dispatches through the matching factory slot.
