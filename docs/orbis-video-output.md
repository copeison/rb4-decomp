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
4. Create `DefaultVBuffer` and the 120-byte `IdentityInstanceVBuffer`.
5. Create and register the double-buffered display render target.
6. Register the Orbis render factories and initialize a condition variable.
7. Start `SubmitDoneThread` at priority 699 and initialize its profiling state.
8. Wait until the worker publishes readiness, then hide the system splash
   screen.

The worker at `0x8D7340` waits for up to four events at a time. Flip-complete
events retire per-buffer fences. End-of-pipe events call `sceGnmSubmitDone`,
advance submission fences, signal the waiting condition, apply changes to the
runtime vsync flag and mode, and submit the next video flip. A failed or timed
wait also calls `sceGnmSubmitDone` under the submission lock.

The deleting destructor at `0x8D7B00` runs the Orbis object destructor and then
frees the 4,352-byte allocation.

The matching platform shutdown at `0x8D8040` clears the submit-thread run flag
and joins the worker. It conditionally destroys the worker condition variable,
releases frame-runtime objects, removes GNM event 64, deletes the event queue,
and closes the video-output handle. The base render-system shutdown invokes
this routine only after its shared GPU resources have been released.
