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

## Context frame start

`RndContext::BeginFrame(unsigned int)` (`0x6BC3B0`) resets the context before
a frame. The name is not in the map, and it may be the map's `Reset()`.
1. It clears the target mode to -1, the camera stack and override, the
   render-target size and the two cameras (`RndCameraContext::Clear`).
2. It writes the camera defaults for 2D targets
   (`RndCameraContext::SetDefaultShaderConstants`).
3. It resets the blend mode to Source, the shading mode to standard, the
   active stages, and the slot limits.
4. It passes the flags to `_BeginFrameImpl`.
5. It selects the device's default camera buffer and disables the four clip
   planes.
6. It writes an environment index of -1 and a zero solid color into the
   draw-state buffer, and selects the device's last two default buffers.

`_SyncClipPlanes` (`0x6BC590`) only runs where the PS4 capabilities are
enabled. It writes each plane in its mask to the clip-plane buffer, or zero
when the plane is disabled. If any plane is enabled it syncs and selects that
buffer; otherwise it selects the device's default.

The four 20-byte slots at `+18888`, previously read as light slots, are these
clip planes.

## Binding render targets

`RndContext::SetRenderTargets(const RenderTargetParams&)` (`0x6BC730`) records
the colour and depth targets, stamping each with the frame count. It takes
the target mode from the textures, and sets the viewport. The viewport is the
first target's size, or 1 x 1, unless the parameters set it
(`mViewportSet` at `+21`). The function then:
1. updates the cameras' target info;
2. writes the target size to the render-target constants
   (`_SyncRenderTargetCBuffer`, inlined);
3. calls the platform `_SetRenderTargetsImpl`.

The overloads at `0x6BCF10` and `0x6BD0D0` build the parameters from a texture
list or a single texture.
