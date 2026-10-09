# Ambient-occlusion target

The target-resource block initializer at `0x6B2660` creates an `AO Buffer`
at the full target extent when resource flag `0x800` is set. The target
occupies offset `0x50` in each 216-byte per-scene block and accepts the
corresponding resource from a previous block for reuse.

The primary scene registers the target with the enclosing resource owner.
Partial-frame scenes retain it through their own blocks. The matching section
of `RndBufferCollection::Destroy` at `0x6AFFE0` invokes the target's virtual
deleting destructor and clears the slot.

The exact format-28 descriptor assembly and owner factory dispatch remain
behind a narrow adapter until the common render-target descriptor is
reconstructed.
