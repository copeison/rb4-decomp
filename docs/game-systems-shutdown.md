# Game-system lifecycle

`game_systems_initialize` at `0x402C30` creates the 4,352-byte Orbis renderer,
warms the platform configuration for ID 7, initializes the shared render
runtime, and conditionally creates default resources from the second byte of
`RndInitParams`. It then runs the Orbis backend post-initializer and
nine render-dependent subsystem initializers before registering the cleanup
callback.

`game_systems_shutdown` at `0x402D30` is registered as a cleanup callback by
`game_systems_initialize`. It exits immediately when the renderer does not
exist or when another shutdown is already running.

Once guarded, it shuts down eight render-dependent game subsystems in reverse
startup order, calls `RndDevice::Terminate`, invokes the renderer's virtual
deleting destructor, and clears `gRndDevice`. The in-progress flag is
cleared only after the global renderer pointer has been reset.

The individual dependent subsystem types remain unnamed, so their fixed
shutdown sequence stays behind a narrow adapter until call-site or RTTI
evidence identifies each owner.
