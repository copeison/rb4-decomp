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

The common cube object is exactly 792 bytes. It contains the 168-byte texture
base, a normalized 144-byte descriptor snapshot, and six typed 80-byte
mip-chain faces. Construction publishes the first face's shared mip properties
and selects resource index two. The Orbis subclass adds five pointers for the
texture view, two allocations, color target, and depth target.

Backend initialization at `0x8E6CE0` (`PS4TextureCube::_SyncStaticImpl`)
has separate color and depth paths, inlined from the helpers the
reconstruction calls `_SyncRegular` and `_SyncDepthStencil`:
- **Color cubes** build a `kTextureTypeCubemap` `TextureSpec`, allocate the
  tiled storage from the `"gpu"` heap, and tile every mip of all six faces
  (slice = face). A render-target cube adds a `sce::Gnm::RenderTarget` from
  `RenderTarget::initFromTexture` and uses `kResourceMemoryTypeGC`; other
  cubes use `kResourceMemoryTypeRO`.
- **Depth cubes** build a six-slice `DepthRenderTargetSpec` from
  `PS4RenderUtl::GetZFormat` and `GetStencilFormat` (`0x8E17C0`, `0x8E17F0`).
  They allocate the Z and stencil surfaces, view them through
  `Texture::initFromDepthRenderTarget(target, true)`, and use
  `kResourceMemoryTypeGC`.

The final object fields contain the Gnm texture descriptor, primary and
secondary allocations, color-target descriptor, and depth-target descriptor.
Accessors at `0x8E7270` and `0x8E7280` expose the color and depth target views.
Destruction at `0x8E6BE0` defers the render target's CMASK and color
surfaces (`getCmaskAddress`, `getBaseAddress`) and both allocations through
`PS4Device::DeferredDelete`, releases the descriptor objects, and then invokes the common cube
texture destructor. The deleting destructor follows at `0x8E6CC0`.

The six virtual methods at `0x8E71A0` through `0x8E725F` forward the single
texture view at offset 792 to the shared vertex, hull, domain, geometry,
pixel, and compute binding functions. They also forward the common address
mode, filter mode, binding flags, and sampler border color.

Common lifecycle evidence is preserved in
`analysis/exports/render-texture-cube.asm` and
`analysis/exports/render-texture-cube.c`.
