# Orbis GPU synchronization

`orbis_render_system_wait_idle` at `0x8D8100` performs the renderer's full idle
barrier. It waits for GPU submissions, retires deferred allocations, flushes
an active frame when necessary, and resets the newly selected platform frame
slot at `0x8E8450` for recording.

The GPU wait at `0x8D8140` checks ten submission counters associated with the
active buffer. It yields the current thread until all ten counters reach zero.
If a frame is active while waiting, it invokes the renderer's flush path so
work can continue to drain.

Deferred allocations are protected by a recursive mutex. The retirement pass
at `0x8D8200` starts only once the frame index reaches two and releases entries
whose recorded frame is no newer than `current_frame - 2`. This preserves a
two-frame safety window before GPU-owned memory is returned to the allocator.

`orbis_defer_allocation_release` at `0x8D83F0` appends a non-null allocation
and the current frame index to this queue. `orbis_release_all_retired_allocations`
at `0x8D84B0` ignores frame age and drains every entry; it is used when the
renderer needs a complete cleanup rather than normal rolling retirement.
