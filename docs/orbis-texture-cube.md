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

Backend initialization at `0x8E6CE0` has separate color and depth paths. Color
cubes create a 32-byte Gnm texture descriptor, allocate tiled storage, upload
every available mip for all six faces, and optionally create a 64-byte render
target view. Depth cubes create a 52-byte depth-target descriptor, allocate its
depth and optional stencil surfaces, then create the matching 32-byte texture
view. Both paths select resource memory type `109`.

The final object fields contain the Gnm texture descriptor, primary and
secondary allocations, color-target descriptor, and depth-target descriptor.
Accessors at `0x8E7270` and `0x8E7280` expose the color and depth target views.
Destruction at `0x8E6BE0` defers GPU allocations through the Orbis render
system, releases the descriptor objects, and then invokes the common cube
texture destructor. The deleting destructor follows at `0x8E6CC0`.

The six virtual methods at `0x8E71A0` through `0x8E725F` forward the single
texture view at offset 792 to the shared vertex, hull, domain, geometry,
pixel, and compute binding functions. They also forward the common address
mode, filter mode, binding flags, and sampler border color.
