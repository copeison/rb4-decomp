# Orbis GPU synchronization

`orbis_render_system_wait_idle` at `0x8D8100` performs the renderer's full idle
barrier. It waits for GPU submissions, retires deferred allocations, flushes
an active frame when necessary, and finally waits on the primary frame owner.

The GPU wait at `0x8D8140` checks ten submission counters associated with the
active buffer. It yields the current thread until all ten counters reach zero.
If a frame is active while waiting, it invokes the renderer's flush path so
work can continue to drain.

Deferred allocations are protected by a recursive mutex. The retirement pass
at `0x8D8200` starts only once the frame index reaches two and releases entries
whose recorded frame is no newer than `current_frame - 2`. This preserves a
two-frame safety window before GPU-owned memory is returned to the allocator.
