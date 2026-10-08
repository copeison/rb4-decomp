# Orbis 2D texture-array factory

`orbis_create_texture_array_2d` at `0x8D8AA0` is virtual slot 29 of the Orbis
render system. It allocates a 360-byte object and passes the supplied texture
array descriptor to `orbis_texture_array_2d_construct` at `0x8E6640`.

The platform constructor first invokes the common 2D texture-array constructor
at `0x69AAC0`, replaces the object vtable with the Orbis implementation, and
zeros its final 16 bytes of backend state. The common constructor copies the
descriptor's slice list into 480-byte per-slice records and stores the slice
count. Adjacent registration strings name this class
`RndTextureArray2DResource`, its property group `TextureArray2DProperties`, and
its source field `texture_array`.
