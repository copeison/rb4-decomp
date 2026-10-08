# Default render resources

`render_initialize_default_resources` at `0x6BDCA0` owns the renderer's
fallback scene objects. Its initialization order is:

1. Allocate and retain a 784-byte `RndSceneResource`.
2. Build seven families of render buffers for seven buffer classes.
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

The seven buffer families built at `0x6BDE60` remain under analysis. Their
descriptor constants and ownership slots are retained in the IDA export; the
source does not assign speculative graphics API names to them.
