# Render frame lifecycle

The main game loop uses four entry points on the global render system. Their
clean reconstruction now lives in `src/render/core/render_system_frame.cpp`. Before
beginning a normal frame, it also services the pending screenshot capture
described in `docs/screenshot-capture.md`.

## Poll and skipped frames

`render_system_poll` at `0x3DE0E0` locks the render system, increments its
active-call counter, invokes virtual slot `0x20` on the frame owner at offset
`0x70`, then decrements the counter and unlocks. The function also observes
whether the global render system survived the virtual call.

`render_system_skip_frame` at `0x3DEAA0` takes the same lock and increments the
64-bit skipped-frame counter at offset `0xA0`. No render submission occurs.

## Begin and end

`render_system_begin_frame` at `0x3DE130` first calls the preparation helper at
`0x3DE170`. That helper records the calling thread, updates instantaneous and
smoothed frame-rate values, begins the `GPU Total` timing scope, and prepares
the platform command context.

The attach helper at `0x3DE3A0` activates the current frame owner and validates
that it has a nonzero output size. It records the owner and copies its transient
render-object list into the render system. A failed validation immediately
runs the normal finish path and makes `render_system_begin_frame` return false.

`render_system_end_frame` at `0x3DE7C0` clears the active owner and transient
object list, then calls the shared finish helper at `0x3DE4A0`. The finish path
submits the frame, advances the frame counter, drains deferred material work,
releases the render lock, and polls both default scene resources.

The low-level platform context, frame-owner virtual interface, and mutex fields
remain behind narrow adapters until their complete class layouts are recovered.
