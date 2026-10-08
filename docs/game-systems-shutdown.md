# Game-system shutdown

`game_systems_shutdown` at `0x402D30` is registered as a cleanup callback by
`game_systems_initialize`. It exits immediately when the renderer does not
exist or when another shutdown is already running.

Once guarded, it shuts down eight render-dependent game subsystems in reverse
startup order, calls `render_system_shutdown`, invokes the renderer's virtual
deleting destructor, and clears `g_render_system`. The in-progress flag is
cleared only after the global renderer pointer has been reset.

The individual dependent subsystem types remain unnamed, so their fixed
shutdown sequence stays behind a narrow adapter until call-site or RTTI
evidence identifies each owner.
