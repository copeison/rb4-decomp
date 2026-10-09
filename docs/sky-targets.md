# Sky targets

`RndBufferCollection::_AllocAtmosphereBuffers` at `0x6B0E80` creates four `Sky Buffer` targets at
full, half, quarter, and eighth resolution. Each shifted dimension has a
minimum of one pixel. The targets occupy resource-owner offsets `0x1B0`,
`0x1B8`, `0x1C0`, and `0x1C8`, and every result is registered with the owner.

When a previous resource owner is supplied, each level uses the matching old
target as its reuse input. During first-time creation, the three reduced
targets instead use the newly created full-resolution target as their reuse
input. This distinction is preserved in the reconstruction.

The matching portion of `RndBufferCollection::Destroy` at `0x6AFFE0`
invokes each target's virtual deleting destructor and clears all four slots.
The exact shared target descriptor remains behind a narrow adapter.
