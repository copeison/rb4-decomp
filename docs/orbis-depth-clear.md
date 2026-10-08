# Orbis depth and stencil clearing

The depth clear helper has two paths selected by the depth target's HTILE
acceleration bit.

The HTILE path emits the `0x2c` depth-metadata flush event and uses a built-in
compute shader. If the target has stencil storage, it clears the selected
stencil slices with the stencil byte replicated across a 32-bit word. It then
clears the matching HTILE range to zero. Both ranges are treated as dword
buffers with resource memory type `109`, and each dispatch covers 64 dwords per
thread group. The helper returns `true` so render-target binding can emit the
shared completion barrier used for accelerated color and depth clears.

Targets without HTILE use a raster fallback. It enables depth and stencil
writes, selects always-pass comparisons and replace stencil operations, sets
the requested clear values, disables color writes, and submits a built-in
depth-clear draw. The helper then restores the cached render-context state and
returns `false` because this path does not need the accelerated-clear barrier.

The draw helper binds the renderer's depth-clear shader, temporarily selects
its draw state, unbinds the pixel shader, submits the clear geometry, and
restores the normal draw state. Concrete shader objects and Gnm packets remain
behind adapters until their owning render-system structures are reconstructed.
