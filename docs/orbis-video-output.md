# Orbis video-output startup

The Orbis platform initializer begins at `0x8D7B20`. IDA originally treated
this range as unowned code; its boundary is now defined as `0x8D7B20` through
`0x8D7D22`. Hex-Rays still cannot lift its stack-call pattern, so the exact
instructions are tracked in
`analysis/exports/orbis-render-system-initialize.asm`.

Startup performs the following work:

1. Open the default video-output port and set its initial flip rate to zero.
2. Apply the 1,080-line window-mode margin configuration.
3. Create the `EOP QUEUE`, register GNM event 64, and add the video flip event.
4. Create the typed fallback mesh and identity-instance buffers documented in
   `orbis-vertex-descriptors.md`.
5. Allocate and publish the eight-byte Orbis resource factory, create the
   double-buffered display target, and allocate the platform render context.
6. Initialize the submit condition variable.
7. Configure and start the joinable `SubmitDoneThread` at round-robin priority
   699 through the shared 136-byte engine thread wrapper. Its embedded runtime
   receives the engine's six-processor default affinity mask and 128-KiB
   minimum stack.
8. Wait until the worker publishes readiness, then hide the system splash
screen.

The wrapper at `0x259210` stores the submit entry point and render-system
context separately from the embedded pthread callback. The callback at
`0x2593A0` registers the running thread as `SubmitDoneThread`, invokes the
entry point, and records its zero result. This replaces the former broad
submit-thread configuration adapter with the same shared path used by other
engine workers.

The worker at `0x8D7340` waits for up to four events at a time. Flip-complete
events read `SceVideoOutFlipStatus::flipArg`, accept buffer indices zero and
one, and retire that buffer's pending-presentation count on every attached
output texture. End-of-pipe events inspect the ten submission counters for the
worker's current buffer. They call `sceGnmSubmitDone` immediately when all ten
are clear or after pending time accumulates to 1,000 ms, advance the current
buffer's presentation counts, publish the submit token, and wake its waiter.

The runtime vsync enable byte is at `RndConfig + 0x98`; when enabled, the
configured mode comes from `+0x14`, otherwise mode zero is used. The worker
caches that mode at `OrbisRenderSystem + 0x10F8`. Mode two selects video flip
rate one; all other modes select rate zero. Mode zero submits with
`SCE_VIDEO_OUT_FLIP_MODE_HSYNC`, while every other value uses
`SCE_VIDEO_OUT_FLIP_MODE_WINDOW_2`. The previous buffer index is carried as
the flip argument so the later flip-complete event can retire it. A failed or
timed wait also calls `sceGnmSubmitDone` under the submission lock and resets
the accumulated timer.

The deleting destructor at `0x8D7B00` runs the Orbis object destructor and then
frees the 4,352-byte allocation.

The matching platform shutdown at `0x8D8040` clears the submit-thread run flag
and joins the worker through its typed SCE pthread handle. It conditionally
destroys the worker condition variable,
deletes the active back buffer, deletes the primary and additional render
contexts in reverse order, removes GNM event 64, deletes the event queue, and
closes the video-output handle. The base render-system shutdown invokes this
routine only after its shared GPU resources have been released.
