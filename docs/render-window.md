# Render windows

`RndWindow` (`src/render/system/RndWindow.{h,cpp}`) is the presentation
surface the device draws each frame. The device keeps the main window,
the window being drawn, and the windows drawn this frame (see
[render-device.md](render-device.md)).

## Classes

| Class | Vtable | Size | Notes |
| --- | --- | --- | --- |
| `RndWindow` | 0x1901F38 | 16 | Constructor 0x4486F0 sets the field at 8 to -1. Slots 2 and 3 are pure. |
| `RndBufferedWindow` | 0x19A5B28 | 32 | Not in the reference map. Optionally owns one 1,552-byte buffer collection. |
| `PS4Window` | 0x195F708 | 40 | The main window: two video-out back buffers. |

## Virtual slots

| Slot | Method | `RndWindow` | `RndBufferedWindow` | `PS4Window` |
| --- | --- | --- | --- | --- |
| 0, 1 | destructors | 0x448710, 0x448720 | 0x11B2D40, 0x11B2D90 | 0x8E2860, 0x8E2870 |
| 2 | `_GetActiveBufferIndex() const` | pure | 0x11B2DE0 (returns 0) | inherited |
| 3 | `GetBufferCollections() const` | pure | 0x11B2DF0 (one collection) | inherited |
| 4 | `Poll()` | 0x448820 (empty) | inherited | inherited |
| 5 | `CheckForResize()` | 0x448830 (empty) | inherited | inherited |
| 6 | unknown, returns true | 0x448840 | inherited | inherited |

The names of slots 2, 3, and 6 are not in the reference map.
`RndDevice::_DoBeginDrawingWindow` calls slot 5 before reading the window's
size, which is why slot 5 is identified as `CheckForResize`.

This build's `PS4Window` overrides only the destructors. The map also lists
`PS4Window::Poll`, `CheckForResize`, and `_InitBuffers`; the first two are no
longer overridden, and the third is inlined into the constructor.

## Non-virtual helpers

The helpers at 0x448730-0x4487E0 read the first collection's size, and read
or write every collection's shading mode (offset 16) and buffer-inspection
mode (offset 20). The debug commands and the screenshot path use them. Their
names are not in the reference map.

`PS4Window::AdvanceFrame` at 0x8E2890 flips the active back buffer.
`PS4Device::_EndFrameImpl` calls it for each window submitted, and
`PS4Texture2D::GetRenderTarget` reads the active index to pick the matching
Gnm render target.
