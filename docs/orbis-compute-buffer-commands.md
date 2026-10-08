# Orbis compute-buffer commands

`orbis_render_context_copy_compute_buffer_count` at `0x8EA740` copies the
source buffer's hidden append/consume counter into the first four bytes of the
destination buffer. This is the engine's Orbis equivalent of a structured
buffer count-copy command.

The method selects each buffer's active backing allocation. It binds the source
descriptor to compute-stage RW slot 0, which exposes its counter through GDS,
then emits a blocking four-byte Gnm DMA from GDS offset 0 to the destination's
GPU address. Finally it clears the temporary RW binding.
