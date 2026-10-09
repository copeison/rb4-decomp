# Orbis GPU synchronization

`PS4Device::_BeginFrameImpl` at `0x8D8100` performs the renderer's full idle
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

`PS4Device::DeferredDelete` at `0x8D83F0` appends a non-null allocation
and the current frame index to this queue. `PS4Device::_ProcessDeferredDeletion`
at `0x8D84B0` ignores frame age and drains every entry; it is used when the
renderer needs a complete cleanup rather than normal rolling retirement.
The exact 32-byte node allocation, sentinel links, unlinking, GPU allocation
release, sized node deallocation, and count updates are now source-owned.
