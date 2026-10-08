# Orbis render system

`orbis_render_system_create` at `0x8D5DF0` allocates 4,352 bytes and invokes
the Orbis-specific constructor at `0x8D77F0`. That constructor first builds the
shared render-system base, installs the Orbis vtable, and initializes the
following platform state:

- video-output and flip state;
- worker state with the initial label `Unknown Thread!`;
- two recursive submission mutexes;
- an intrusive deferred-command list;
- the Orbis renderer singleton at `g_orbis_render_system`.

The Orbis destructor at `0x8D79B0` clears that singleton first, releases every
node in the deferred-command list, destroys both recursive mutexes, releases
profiling and condition-variable state when present, and then invokes the base
render-system destructor.

Runtime shutdown remains a separate phase. `render_system_shutdown` closes the
active backend and owned GPU resources before the virtual deleting destructor
reaches this object destructor.
