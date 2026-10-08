# Orbis 1D texture-array factory

`orbis_create_texture_array_1d` at `0x8D8A40` is virtual slot 27 of the Orbis
render system. It allocates a 360-byte object and passes the supplied texture
array descriptor to `orbis_texture_array_1d_construct` at `0x8E5870`.

The platform constructor invokes the common 1D texture-array constructor at
`0x696C00`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x696BA0` assigns
descriptor type 4, which the type-to-resource map identifies as
`RndTextureArray1DResource`.
