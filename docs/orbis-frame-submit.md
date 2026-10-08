# Orbis frame submission

`orbis_render_system_submit_frame` at `0x8D8300` is the Orbis implementation
of the base renderer's frame-submit hook. The base frame lifecycle passes it
the transient render objects collected from every active frame owner.

The function holds the submission mutex while it waits on the submit-thread
condition variable. The worker publishes a single token after processing an
end-of-pipe event; submission consumes that token by clearing the shared value.
It then flushes active command state when present, submits the primary frame
owner, and finalizes each collected render object before releasing the mutex.

This handshake limits CPU-side frame submission until the prior GPU end-of-pipe
event has advanced the shared video-output state.
