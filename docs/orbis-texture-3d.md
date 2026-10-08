# Orbis 3D texture factory

`orbis_create_texture_3d` at `0x8D89E0` is virtual slot 25 of the Orbis
render system. It allocates a 408-byte object and passes the supplied texture
descriptor to `orbis_texture_3d_construct` at `0x8E53C0`.

The platform constructor invokes the common 3D texture constructor at
`0x6F5CB0`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x6F5C50` assigns
descriptor type 2. The type-to-resource map at `0x6ACF40` identifies type 2
as `RndTexture3DResource`.
