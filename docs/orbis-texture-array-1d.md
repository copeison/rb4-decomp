# Orbis 1D texture-array factory

`orbis_create_texture_array_1d` at `0x8D8A70` is virtual slot 28 of the Orbis
render system. It allocates a 392-byte object and passes the supplied texture
array descriptor to `orbis_texture_array_1d_construct` at `0x8E5D40`.

The platform constructor invokes the common 1D texture-array constructor at
`0x698160`, replaces the object vtable with the Orbis implementation, and
zeros the final 48 bytes of backend state. The common constructor copies the
descriptor's array elements into 80-byte texture records and stores the
element count. The adjacent resource registration names the class
`RndTextureArray1DResource`.
