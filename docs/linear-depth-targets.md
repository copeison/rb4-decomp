# Linear-depth targets

`RndBufferCollection::_AllocLinearDepthBuffer` at `0x6B2C90` creates the per-scene
`Linear Depth Buffer` at the full target extent. When tiled lighting is
enabled, it also creates `Tiled Depth Range` at
`ceil(width / light_tile_size)` by `ceil(height / light_tile_size)`.

The targets occupy offsets `0x40` and `0x48` in each 216-byte per-scene
resource block. Both accept the corresponding resource from a previous block
for reuse. Resources for the primary scene are registered with the enclosing
target owner; partial-frame scene resources remain owned through their block.

The matching portion of `RndBufferCollection::Destroy` at `0x6AFFE0`
invokes both targets' virtual deleting destructors and clears their slots.
Target descriptor details remain behind a narrow adapter until the common
render-target descriptor and resource-owner dispatch are reconstructed.
