# Orbis back buffer

The 40-byte Orbis back-buffer object is constructed at `0x8E24A0` during
render-system startup. It owns the engine render-target state and tracks the
active display buffer at offset `0x20`.

Construction creates two 64-byte Gnm render-target descriptors with the current
display width and height. Their data format is `0x00F2E90A`, the Gnm packed
value for `B8G8R8A8Srgb`. Each texture receives a GPU allocation named
`BackBuffer`; the allocation alignment is the larger of the Gnm requirement
and 64 KiB. Their unused CMASK/FMASK auxiliary addresses are cleared.

`orbis_wrap_back_buffer_textures` at `0x8D5E30` creates one 520-byte engine
`OrbisTexture2D` around both Gnm render targets. It publishes both base addresses
and allocation sizes through the texture's two-image backing object, then the
back-buffer constructor attaches that texture to its render target.

Video-output registration derives width, height, and pitch from the first
Gnm descriptor and passes both 256-byte-scaled base addresses to
`sceVideoOutRegisterBuffers` in one two-buffer set. The helper at `0x8E2890`
toggles the active index between zero and one after frame submission.

The destructor thunk at `0x8E2860` delegates to the render-target base. The
deleting destructor at `0x8E2870` then frees the object. The render system
stores this owned object at offset `0x70` and releases it through virtual slot
one during shutdown.
