# Orbis 3D texture factory

`orbis_create_texture_3d` at `0x8D8A40` is virtual slot 27 of the Orbis
render system. It allocates a 360-byte object and passes the supplied texture
descriptor to `orbis_texture_3d_construct` at `0x8E5870`.

The platform constructor invokes the common 3D texture constructor at
`0x696C00`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The common constructor copies the
descriptor elements into 80-byte records and validates their width, height,
and format compatibility. The adjacent resource registration names the class
`RndTexture3DResource`.
