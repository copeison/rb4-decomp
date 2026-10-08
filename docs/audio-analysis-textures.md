# Audio-analysis render textures

The render system owns a pointer at offset `0xDF8` to the 72-byte
`AudioAnalysisTextureSet`. The object uses two inline texture-pointer slots and
an overflow-capable float vector beginning at object offset `0x28`. Its
constructor, destructor, frame preparation, rebuild, update, and width query
are the function family at `0x457620` through `0x458540`.

Frame preparation queries eight audio sources and records the maximum width
requested for each of two channels. A positive width rebuilds its texture when
the slot is empty or the existing texture width differs. The rebuilt resources
are 2D float textures named `Audio Analysis`, with eight rows populated by
replicating each source's channel values across the requested row width.

After rebuilding when needed, the update path copies current source values
into each texture payload. It then invokes texture vtable slot `0x90` for each
active texture with the current render context. The common frame path now owns
the width check and rebuild decision directly; data extraction and texture
population stay behind focused adapters while their audio-source types remain
unidentified.
