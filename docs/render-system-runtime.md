# Render-system runtime lifecycle

`render_system_initialize` at `0x3DDAE0` receives the same 16-byte
`GameSystemInitOptions` block created by `game_initialize`. It copies the block
into the renderer, starts the resource manager, invokes the platform backend,
creates backend resources and built-in buffers, initializes every frame owner,
and records a timestamp when the runtime epoch counter changes from zero to
one.

The built-in buffer setup at `0x3DDC20` acquires four mapped resources:

- the first receives two zero 32-bit values;
- the second receives four 16-byte vectors filled with `0xFF`;
- the third receives a `-1.0f` sentinel and a zero vector;
- the fourth is passed to its type-specific default initializer.

Each dirty mapping is committed immediately after its initial data is written.

`render_system_shutdown` at `0x3DDE60` first drains the deferred-release queue
under its recursive mutex. It then releases default resources, backend-owned
objects, the four built-in buffers, the primary and additional frame owners,
and finally the platform backend. The surrounding game-system shutdown routine
destroys the render-system object and clears `g_render_system` afterward.
