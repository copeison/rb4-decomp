# Partial light accumulation

The partial-frame branch of `render_target_resource_block_initialize` at
`0x6B2660` creates `Partial Light Accum Buffer` when resource flag `0x08` is
set. It occupies offset `0x08` in the 216-byte per-scene block and can reuse
the corresponding target from an earlier block.

This resource exists only for partial-frame blocks, so it is retained through
the block rather than registered in the primary owner's target list. The
matching portion of `render_target_resources_release` at `0x6AFFE0` invokes
its virtual deleting destructor and clears the slot.

The shared light-accumulation target factory at `0x6B2E80` is reconstructed.
This path uses full resolution and the unassigned attachment value while
sharing its 32- versus 64-bit format selection with the primary targets.
