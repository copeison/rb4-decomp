# Orbis back buffer

The 40-byte Orbis back-buffer object is constructed at `0x8E24A0` during
render-system startup. It owns the engine render-target state and tracks the
active display buffer at offset `0x20`.

Its first 32 bytes are the shared `RenderTarget` base reconstructed from
`0x11B2CD0`-`0x11B2E08`. That base stores an attachment index initialized to
`-1`, an ownership flag, and owned and active state pointers. When requested,
construction allocates a 1,552-byte target-state object and places it in both
pointer fields. Destruction invokes that object's deleting virtual method and
clears both pointers. Its state accessor returns the active-state slot together
with a count of one. The Orbis subclass adds only the active-buffer index.

Construction creates two 64-byte Gnm render-target descriptors with the current
display width and height. Their data format is `0x00F2E90A`, the Gnm packed
value for `B8G8R8A8Srgb`. Each texture receives a GPU allocation named
`BackBuffer`; the allocation alignment is the larger of the Gnm requirement
and 64 KiB. Their unused CMASK/FMASK auxiliary addresses are cleared.

`PS4Texture2D::CreateAsBackBuffer` at `0x8D5E30` (the map's name) creates one
520-byte `PS4Texture2D` around both render targets:
- The texture's description is a 1×1 `BackBuffer` render target in data
  format 12, `BGRA_UNorm8_sRGB`, sized to the targets.
- A new shared `PS4Texture2D::Storage` records each target's base address and
  color size.
- Each buffer's texture views its target through
  `Texture::initFromRenderTarget`.
- The pending-presentation counters start at zero.

The window constructor (`PS4Window::PS4Window`, `0x8E24A0`) heap-allocates
the two `RenderTarget`s from one `RenderTargetSpec`, allocates their memory
from the `"gpu"` heap with the CMASK and FMASK addresses cleared, and installs
the wrapping texture as its buffer collection's back buffer. Unless the
configuration overrides the resolution, the output resolution becomes the PS4
capability list's last (largest) entry on PS4 Pro and its first otherwise.

Video-output registration derives width, height, and pitch from the first
target (`sceVideoOutSetBufferAttribute` with the sRGB BGRA pixel format,
tiled mode and 16:9 aspect, plus `OPTION_STRICT_COLORIMETRY`) and passes both
base addresses to `sceVideoOutRegisterBuffers` in one two-buffer set. The helper at `0x8E2890`
toggles the active index between zero and one after frame submission.

The destructor thunk at `0x8E2860` delegates to the render-target base. The
deleting destructor at `0x8E2870` then frees the object. The render system
stores this owned object at offset `0x70` and releases it through virtual slot
one during shutdown.

Common lifecycle evidence is preserved in
`analysis/exports/render-target.asm` and
`analysis/exports/render-target.c`.
