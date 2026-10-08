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
`src/render/resources/camera/default_camera.cpp` expresses this as a reusable helper because the
same sequence is inlined in the main initializer.

`src/render/resources/system/default_render_resources.cpp` now reconstructs
this full ordering. It accepts the effective rendering flag directly and also
checks the binary's global force-enable hook. Each compute buffer now uses the
typed 48-byte common descriptor directly: a four-byte stride, one element, the
buffer index as initial data, zero flags, and the shared name. The two
finalization calls remain narrow scene-resource boundaries.

The texture-family loop at `0x6BDE60` is reconstructed separately in
`src/render/resources/textures/default_textures.cpp`, using the typed common
texture factories for every shape.

## Per-frame and shutdown paths

`render_poll_default_resources` at `0x6BFA00` forwards the resource poll to the
primary scene and the separate lighting scene when present. The shared helper
at `0xFD8B0` invokes virtual slot `0x90`; its use in `sound_manager_update` for
every active sound scene confirms that this is the normal per-frame resource
poll rather than a renderer-only callback.

`render_release_default_resources` at `0x6BF860` releases the two scene
resources, clears material and lighting references and both light-ID lists,
releases all 49 texture resources, and finally releases the two compute
buffers. The cleaned source also clears the camera pointer with the other
borrowed scene components.

The complete block is 568 bytes and begins at render-system offset `0x7B8`.
The common frame and shutdown paths now use that typed block directly for
polling and release.

Construction at `0x6BDB30` is source-owned through placement construction of
the typed block. This initializes every pointer, both light-ID vectors, the
directional lighting mode, and the `100.0f` scale using their declared types.
The destructor at `0x6BDC20` destroys both vector allocations in reverse order
and releases any retained lighting and primary scene resources, matching the
binary's fallback cleanup when runtime shutdown has not already cleared them.
