# Orbis GPU timing

The render context allocates an 8 KiB GPU-visible timestamp buffer at
`0x8E7DF0`. It divides that buffer into 512 begin/end pairs and associates
active pairs with the 64-bit keys issued by the engine's GPU-stat block.

`orbis_render_context_begin_gpu_stat` at `0x8EBA20` advances through the 512
records as a ring, marks the selected record active, writes the GPU core clock
to its begin address, and inserts the key-to-record association. The matching
method at `0x8EBBB0` finds that key and writes the end clock value.

Both writes use a release-memory packet in the active frame's draw command
buffer. Graphics recording uses the Gnm flush-color/depth event `0x04`;
compute recording uses the compute-complete end-of-pipe event `0x28`. The
destination is uncached memory and the write source is the 64-bit GPU core
clock counter.

`orbis_render_context_resolve_gpu_stat` at `0x8EBC70` subtracts the two 64-bit
clock values and multiplies the difference by `1.25e-9`, the reciprocal of the
PS4 GPU's 800 MHz core clock. It returns elapsed seconds, leaves the six generic
hardware-counter fields at zero, marks the timestamp record free, and removes
the key from the active map.
