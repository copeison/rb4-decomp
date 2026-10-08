# Orbis render system

`orbis_render_system_create` at `0x8D5DF0` allocates 4,352 bytes and invokes
the Orbis-specific constructor at `0x8D77F0`. That constructor first builds the
shared render-system base, installs the Orbis vtable, and initializes the
following platform state:

- video-output and flip state;
- worker state with the initial label `Unknown Thread!`;
- two recursive submission mutexes;
- eight fallback mesh-stream descriptors, nine identity-instance descriptors,
  and their two GPU allocation pointers;
- an eight-byte Orbis resource factory stored in the common render-system
  factory slot at offset `0x130`;
- an intrusive deferred-command list;
- the Orbis renderer singleton at `g_orbis_render_system`.

The Orbis destructor at `0x8D79B0` clears that singleton first, releases every
node in the deferred-command list, destroys both recursive mutexes, releases
profiling and condition-variable state when present, and then invokes the base
render-system destructor.

Runtime shutdown remains a separate phase. `render_system_shutdown` closes the
active backend and owned GPU resources before the virtual deleting destructor
reaches this object destructor.

The source-owned Orbis factory vtable contains destructors followed by creation
methods for fences, meshes, all seven texture shapes, constant and compute
buffers, shaders, particle buffers, and occlusion queries. Startup allocates
the exact eight-byte factory object, installs that 16-entry table, and
publishes it through the typed common renderer field. Each entry forwards to
the reconstructed concrete factory for that resource type.

The common constant-buffer, shader, compute-buffer, particle-buffer, and
occlusion-query factories now dispatch directly through entries 11 through 15
of this table. Their original call sites use offsets `0x58`, `0x60`, `0x68`,
`0x70`, and `0x78` from the vtable respectively.
