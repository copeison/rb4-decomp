# Orbis particle-buffer factory

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

The wrapper at `0x6EAFD0` dispatches through slot 33. Its caller clamps the
particle capacity to at least one before creating the buffer.
