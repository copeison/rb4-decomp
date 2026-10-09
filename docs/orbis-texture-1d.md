# Orbis 1D texture backend

`orbis_create_texture_1d` at `0x8D8980` is virtual slot 23 of the Orbis
render system. It allocates a 408-byte object and passes the supplied texture
descriptor to `orbis_texture_1d_construct` at `0x8E4F60`.

The platform constructor invokes the common 1D texture constructor at
`0x6F5870`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x6F5810` assigns
descriptor type 0 and dispatches through the matching factory slot.

The common type is exactly 392 bytes: the 168-byte `RenderTexture` base, a
144-byte shape-specific descriptor snapshot, and an 80-byte mip-chain state.
The Orbis subclass adds the Gnm descriptor and allocation pointers, preserving
the observed 408-byte size. The descriptor and mip-chain internals remain
opaque where member semantics are not yet supported by callers, while their
sizes, lifetime, and placement are exact.

Backend initialization at `0x8E5050` (`PS4Texture1D::_SyncStaticImpl`)
uses the SDK texture path. `sce::GpuAddress::computeSurfaceTileMode` picks a
tile mode from `sce::Gnm::getGpuMode()`, `PS4RenderUtl::GetSurfaceType` and
`PS4RenderUtl::GetDataFormat`. A `sce::Gnm::TextureSpec` (type
`kTextureType1d`, the authored width, unit height, depth and slice count,
`GetNumMips() + 1` levels) initializes a new `sce::Gnm::Texture`. Its
`getSizeAlign()` sizes a `"gpu"`-heap allocation named after the texture.
Each mip level is tiled into its surface with
`TilingParameters::initFromTexture`, `computeTextureSurfaceOffsetAndSize` and
`tileSurface`. The texture then receives the allocation through
`setBaseAddress` and `kResourceMemoryTypeRO`.

The 3D (`0x8E54B0`), 1D-array (`0x8E5960`) and cube-array (`0x8E6730`)
backends follow the same steps:
- The 3D texture uses `kTextureType3d` with width, height and depth. It
  borrows the storage of a texture passed for reuse, and uses
  `kResourceMemoryTypeGC` when the format is a render target.
- The 1D array uses `kTextureType1dArray`, with one slice per pixel-data
  element.
- The cube array uses `kTextureTypeCubemap` with
  `kSurfaceTypeTextureCubemap`, and slice `6 * cube + face`.

In both arrays the first element's format and mip count apply to every
slice.

`PS4RenderUtl::GetDataFormat` (`0x8E1790`) maps the engine's data formats to
Gnm formats with a switch the compiler turned into a masked table; unsupported
formats (the 24-bit RGB layouts, RGBX, ETC2, depth and ASTC) give
`kDataFormatInvalid`. `RndDataFormatName` (`0x68F200`) holds the 86 format
names. `RndPixelData::GetNumMips` (`0x6832D0`) counts the levels after the
first.

The final 16 bytes hold the Gnm texture descriptor pointer and its allocation.
Destruction at `0x8E4F90` defers the allocation through the Orbis render system,
releases the descriptor, clears its object field, and invokes the common 1D
texture destructor. The deleting destructor follows at `0x8E4FF0`.

The six virtual methods at `0x8E52D0` through `0x8E538F` forward the texture
view and common sampler state to the shared vertex, hull, domain, geometry,
pixel, and compute binding functions.

Common lifecycle evidence is preserved in
`analysis/exports/render-texture-1d.asm` and
`analysis/exports/render-texture-1d.c`.
