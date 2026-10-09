# Orbis GPU timing

The render context allocates an 8 KiB GPU-visible timestamp buffer at
`0x8E7DF0`. It divides that buffer into 512 begin/end pairs and associates
active pairs with the 64-bit keys issued by the engine's GPU-stat block.

`PS4Context::_BeginGpuStatsImpl` at `0x8EBA20` advances through the 512
records as a ring, marks the selected record active, writes the GPU core clock
to its begin address, and inserts the key-to-record association. The matching
method at `0x8EBBB0` finds that key and writes the end clock value.

Both writes are `GfxContext::writeAtEndOfPipe` on the active frame's graphics
context, even while compute is being recorded. Graphics recording uses the Gnm flush-color/depth event `0x04`;
compute recording uses the compute-complete end-of-pipe event `0x28`. The
destination is uncached memory and the write source is the 64-bit GPU core
clock counter.

`PS4Context::_EvalAndRetireGpuStatsImpl` at `0x8EBC70` subtracts the two 64-bit
clock values and multiplies the difference by `1.25e-9`, the reciprocal of the
PS4 GPU's 800 MHz core clock. It returns elapsed seconds, leaves the six generic
hardware-counter fields at zero, marks the timestamp record free, and removes
the key from the active map.

The state lives at the end of `PS4Context`: `mGpuStatBlocks[512]` at `0x41838`,
the timestamp allocation (`"GpuStatBlock timestamps"`, from the `"gpu"` heap)
at `0x44838`, the ring cursor at `0x44840`, and
`eastl::map<unsigned long, GpuStatBlock*>` at `0x44848`. The map is
modelled in `src/utl/containers/Map.h`, following EASTL's red-black tree:
- lookups inline a lower bound;
- inserts go through `DoInsertKey` with a hint (`0x8EC140`);
- the tree functions are EASTL's library code (`RBTreeIncrement` `0x253100`,
  `RBTreeDecrement` `0x253140`, `RBTreeInsert` `0x253270`, `RBTreeErase`
  `0x2534A0`);
- the destructor frees the nodes with `DoNukeSubtree` (`0x8EBF90`).

The constructor also sets `SetupDraw`'s caches to `-1` and the
color-buffer-enable cache to true.
