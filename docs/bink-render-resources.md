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

The conversion routine opens the `Bink Convert` GPU timing scope, prepares a
single source-texture submission record, performs the conversion draw, and
restores the context resource state. The detailed shader and draw-state
construction remains behind the conversion adapter while its embedded types
are recovered.
