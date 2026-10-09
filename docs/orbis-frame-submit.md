# Orbis frame submission

`PS4Device::_EndFrameImpl` at `0x8D8300` is the Orbis implementation
of the base renderer's frame-submit hook. The base frame lifecycle passes it
the back buffers collected from every active frame owner.

The function holds the submission mutex while it waits on the submit-thread
condition variable. The worker publishes a single token after processing an
end-of-pipe event; submission consumes that token by clearing the shared value.
It then flushes active command state when present, submits the platform render
context at `0x8E82D0`, and advances every collected back buffer before
releasing the mutex. Advancing a back buffer toggles its active render target,
so the next frame renders into the other displayable surface.

This handshake limits CPU-side frame submission until the prior GPU end-of-pipe
event has advanced the shared video-output state.
