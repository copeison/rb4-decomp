# Bink render resources

The global Bink render manager allocated at `0x5F28C0` is 72 bytes. It owns a
dynamic list, four fixed conversion slots at offsets `0x20` through `0x38`,
and the working-buffer count at `0x40`. Initialization configures five Bink
working buffers and installs the engine allocation and file-I/O callbacks
before publishing the global manager pointer.

`BinkRenderMgr::PrepareFrame` at `0x5F2B40` runs immediately after the
audio-analysis texture update. It first clears the context's camera with
`RndContext::SetCamera(nullptr)` (`0x6BD220`), then walks the four conversion
slots. A video object with its
byte at offset `0x88` set enters `bink_video_convert_frame`; the manager clears
that byte after conversion so each pending frame is submitted once.

`BinkRenderMgr::ConvertFrame(RndContext&, BinkRenderVideo&)` (`0x5F2BF0`) does the
conversion. Its first argument register is unused, which fits a manager member
whose `this` is never read. It:
1. opens the "Bink Convert" GPU stat scope;
2. moves the output texture from pixel-shader resource to render target;
3. binds it with `RndContext::SetRenderTargets`;
4. selects `RndShaderBinkConvert` with the video's Y, Cr, Cb and A planes and
   its four colour-space constants;
5. draws a full-target quad, keeping the shader;
6. moves the texture back.

`BinkRenderMgr::sInstance` (`0x1AA76F8`) is the manager the device's frame start
reads.
