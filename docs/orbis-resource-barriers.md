# Orbis resource barriers

The render-context barrier method consumes 32-byte records. Their first two
fields match the three common barrier kinds (transition, aliasing, and
unordered access) and the immediate, begin-only, and end-only phases. The
transition payload contains a resource pointer, a 64-bit subresource selector,
and the state before and after the barrier.

The observed state bits use the familiar cross-platform values for render
targets (`0x4`), unordered access (`0x8`), depth writes (`0x10`), stream output
(`0x100`), copy destinations (`0x400`), and resolve destinations (`0x1000`).
The Orbis backend batches immediate barriers and accumulates their Gnm cache
actions. Begin-only barriers emit one shared release label for compatible
resources; end-only barriers consume that label through the resource-sync
tracking list.

Leaving render-target or depth-write state requires metadata work on the
graphics command stream. The color path waits for color-buffer writes, flushes
pixel metadata, and performs DCC decompression or fast-clear elimination plus
FMASK decompression. The depth path waits for depth-buffer writes, flushes
HTILE metadata when required, and decompresses the depth surface. A selected
subresource narrows the temporary Gnm descriptor to one array slice; the
all-subresources value keeps the full range.

When a compute command stream requests metadata work, the backend signals the
resource from the graphics stream and restores the prior compute queue. An
immediate barrier waits on that signal; a begin-only barrier leaves it for the
matching end-only barrier. The adapter boundary in the reconstruction contains
the concrete Gnm descriptors and command-buffer packets while the recovered
source keeps the barrier state machine visible.
