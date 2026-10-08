# Orbis resource synchronization

The render context keeps a compact list of resources that have been released
by one command stream and may need to be acquired by another. Each 24-byte
record contains the resource pointer, a four-byte GPU label, and the current
render-system epoch.

`orbis_render_context_signal_resource` allocates and clears a label the first
time it is called for a batch. Graphics command buffers write `1` after the
color/depth reads complete; compute command buffers write the same value after
compute completion. Additional resources in the batch reuse that label, while
each resource still receives its own tracking record.

`orbis_render_context_wait_for_resource` finds the record for a resource and
waits until its label equals `1`. The comparison uses the full 32-bit mask.
After the wait, every record that shares the label is removed in one stable
compaction pass. This matches the batch semantics: one GPU signal can release
several resources and the first acquire consumes the whole group.

The adapter boundary represents the context-owned label pool, command buffers,
and tracking vector whose concrete layout is still being reconstructed. The
queue-specific release events are `0x28` for graphics and `0x2F` for compute.
