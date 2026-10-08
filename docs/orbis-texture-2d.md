# Orbis 2D texture factory

`orbis_create_texture_2d` at `0x8D89B0` is virtual slot 24 of the Orbis render
system. It allocates a 520-byte object and passes the supplied texture
descriptor to `orbis_texture_2d_construct` at `0x8D62C0`.

The platform constructor first invokes the common 2D texture constructor at
`0x6900B0`, replaces the object vtable with the Orbis implementation, and
zeros its platform-specific backend state. The common resource registration
near that constructor identifies the class as `RndTexture2DResource` and
describes its `texture_path` property, providing direct class evidence for the
factory name.
