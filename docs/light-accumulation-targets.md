# Light-accumulation targets

`RndBufferCollection::_AllocLightAccumBuffers` at `0x6B0B20` creates two primary
`Light Accum Buffer` targets followed by half-, quarter-, and eighth-scale
`Blurred Light Accum Buffer` targets. The primary pair uses attachment choices
zero and one; the blurred targets use the shared unassigned attachment value.

The targets occupy resource-owner offsets `0x178`, `0x180`, `0x190`, `0x198`,
and `0x1A0`. Offset `0x188` belongs to a separate resource. Every target can
reuse the corresponding target from a previous owner and is registered in the
new owner's resource list.

The matching portion of `RndBufferCollection::Destroy` at `0x6AFFE0`
virtually deletes and clears all five slots. The common factory at `0x6B2E80`
selects the 32- or 64-bit format, scales each dimension with a minimum of one,
assigns sequential attachments to the primary pair, and advances the owner's
attachment cursor from the created target's allocation range. Only the final
platform target descriptor remains behind an adapter.
