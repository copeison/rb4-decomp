# Orbis particle buffer

`orbis_create_particle_buffer` at `0x8D8BF0` is virtual slot 33 of the Orbis
render system. It allocates a 360-byte object and forwards the requested
particle count and context to `orbis_particle_buffer_construct` at
`0x8E2AA0`.

The platform constructor initializes the common particle-buffer state, then
creates two GPU allocations of 208 bytes per particle. A third allocation
holds 12 bytes per particle as six 16-bit indices. Each group describes one
quad with triangles `(0, 1, 2)` and `(0, 2, 3)`, offset by four vertices for
each subsequent particle. All three allocations use the embedded name
`ParticleBuffer`.

The common particle-buffer object is 64 bytes. It records capacity, active
particle count, particle-data and context pointers, two signed mode fields,
and three generation flags. Construction at `0x6EB000` clears the active data,
sets both modes to `-1`, enables world-space generation, and leaves velocity
alignment and authored rotation data disabled. The generic factory at
`0x6EAFD0` dispatches through the active render system.

The wrapper at `0x6EAFD0` dispatches through slot 33. Its caller clamps the
particle capacity to at least one before creating the buffer.

The platform vtable contains three methods. `orbis_particle_buffer_destruct`
at `0x8E2D30` defers release of both vertex-stream allocations and the index
allocation. The deleting destructor at `0x8E2D80` performs the same release
before freeing the 360-byte object. The common base destructor is empty.

`orbis_particle_buffer_upload_vertices` at `0x8E2DE0` toggles the active
vertex bank and invokes the common particle vertex generator at `0x6EBD70`.
The generator writes four vertices per active particle directly into the
selected GPU allocation. Each particle occupies 208 bytes, so every generated
vertex is 52 bytes and matches mesh format 5 (`Particle`).

`orbis_particle_buffer_draw` at `0x8E2E10` inlines that upload operation. If
the generated active count is nonzero, it binds eight mesh vertex streams.
Streams present in the particle descriptor mask use the active bank; absent
streams use the renderer's default descriptors. It then builds and binds nine
instance streams beginning at vertex slot 8, selects triangle-list rendering,
selects 16-bit indices, and submits six indices per particle from the static
quad-index allocation. The draw is enclosed by the usual Gnmx prepare/finish
pair.

IDA evidence is preserved in
`analysis/exports/orbis-particle-buffer-backend.asm` and
`analysis/exports/orbis-particle-buffer-backend.c`.
