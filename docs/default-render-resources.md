# Default render resources

`render_initialize_default_resources` at `0x6BDCA0` owns the renderer's
fallback scene objects. Its initialization order is:

1. Allocate and retain a 784-byte `RndSceneResource`.
2. Build seven named fallback-texture families across seven texture shapes.
3. Create two buffers named `Default Compute Buffer`, distinguished by indices
   `0` and `1` in their descriptors.
4. Create `default_cam` and attach an `RndCameraCom`.
5. Create the six default materials.
6. Load the authored default-lighting scene or create a fallback directional
   light when loading fails.
7. Finalize the primary and lighting scene resources.

The small helper at `0x6BEBC0` repeats the camera sequence exactly: it creates
an object in collection zero, names it `default_cam`, attaches an
`RndCameraCom`, and stores the component pointer at owner offset `0x1A0`.
`src/render/default_camera.cpp` expresses this as a reusable helper because the
same sequence is inlined in the main initializer.

`src/render/default_render_resources.cpp` now reconstructs this full ordering.
It accepts the effective rendering flag directly and also checks the binary's
global force-enable hook. Resource construction, compute-buffer allocation,
and the two finalization calls remain narrow runtime adapters while their
ordering and recovered arguments are explicit.

The texture-family loop at `0x6BDE60` is reconstructed separately in
`src/render/default_textures.cpp`. Low-level graphics allocation remains behind
dimension-specific runtime adapters.
