# Orbis cube texture factory

`orbis_create_texture_cube` at `0x8D8A10` is virtual slot 26 of the Orbis
render system. It allocates an 832-byte object and passes the supplied texture
descriptor to `orbis_texture_cube_construct` at `0x8E6BA0`.

The platform constructor first invokes the common cube texture constructor at
`0x6A0DA0`, replaces the object vtable with the Orbis implementation, and
zeros the final 40 bytes of platform-specific backend state. Nearby resource
registration identifies the common class as `RndTextureCubeResource`, with
cube capture and flattened-output properties that corroborate the factory's
type.
