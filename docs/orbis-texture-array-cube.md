# Orbis cube texture-array factory

`orbis_create_texture_array_cube` at `0x8D8AA0` is virtual slot 29 of the
Orbis render system. It allocates a 360-byte object and passes the supplied
descriptor to `orbis_texture_array_cube_construct` at `0x8E6640`.

The platform constructor invokes the common cube texture-array constructor at
`0x69AAC0`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The common constructor copies each
six-face, 480-byte cube descriptor and stores the cube count. The wrapper at
`0x69AA60` assigns descriptor type 7 and dispatches through the matching
factory slot.
