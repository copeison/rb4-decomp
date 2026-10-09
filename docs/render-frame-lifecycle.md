# Render frame lifecycle

The main game loop uses four entry points on the global render system. Their
clean reconstruction now lives in `src/render/system/RndDevice.cpp`. Before
beginning a normal frame, it also services the pending screenshot capture
described in `docs/screenshot-capture.md`.

## Poll and skipped frames

`RndDevice::PollMainWindow` at `0x3DE0E0` locks the render system, increments its
active-call counter, invokes virtual slot `0x20` on the frame owner at offset
`0x70`, then decrements the counter and unlocks. The function also observes
whether the global render system survived the virtual call.

`RndDevice::ForceIncrementFrameCount` at `0x3DEAA0` takes the same lock and increments the
64-bit skipped-frame counter at offset `0xA0`. No render submission occurs.

## Begin and end

`RndDevice::BeginMainWindowFrame` at `0x3DE130` first calls the preparation helper at
`0x3DE170`. That helper records the calling thread, updates instantaneous and
smoothed frame-rate values, begins the `GPU Total` timing scope, and prepares
the platform command context. The common GPU-stat manager is the exact 128-byte
block at render-system offset `0xE00`. Its query ID at core-state offset
`0x118` now remains an integer throughout begin/end tracking. A normal frame
resolves the accumulated queries and advances the manager's four-slot history
ring before constructing the primary context's submission resources.

The two audio-analysis textures are prepared immediately after `GPU Total`
begins. Their typed 72-byte owner is reached through the pointer at render
system offset `0xDF8`. The common path queries both requested widths, compares
them with the live texture widths, rebuilds mismatched resources, and updates
them for the current context. See `docs/audio-analysis-textures.md` for the
resource layout and eight-source update behavior.

The global 72-byte Bink render manager follows the audio-analysis update. Its
four fixed conversion slots are scanned directly; each video whose pending
byte at `0x88` is set is converted for the current context and marked clean.
See `docs/bink-render-resources.md` for the manager layout and conversion
boundary.

Frame-phase transitions dispatch the global callback registry at `0x249D40`.
When `partial_framerate_enabled` is set at renderer-settings offset `0xB4`, the
same transition publishes the low bit of the primary frame epoch as the
partial-frame phase. Preparation performs this for platform phase zero;
primary finish performs it for platform phase one.

The attach helper at `0x3DE3A0` activates the current frame owner and validates
that it has a nonzero output size. It records the owner and copies its transient
render-object list into the render system. A failed validation immediately
runs the normal finish path and makes `RndDevice::BeginMainWindowFrame` return false.

`RndDevice::EndMainWindowFrame` at `0x3DE7C0` clears the active owner and transient
object list, then calls the shared finish helper at `0x3DE4A0`. The finish path
submits the frame, advances the frame counter, drains deferred material work,
releases the render lock, and polls both default scene resources.

Before primary submission, the finish helper builds the original 12-entry
inline resource list. Each 32-byte record contains the source texture from
render-target resource offset `0x170`, resource index `-1`, and flags `4`.
Render-context vtable slot `0xA8` receives the completed list. This path now
uses the typed target-resource owner and direct context dispatch rather than a
whole-function adapter.

Low-level audio sample extraction, Bink conversion draw setup, and the global
phase callback registry remain behind focused adapters.
