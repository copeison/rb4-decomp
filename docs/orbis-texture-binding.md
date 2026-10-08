# Orbis texture binding

The shared functions at `0x8E1EE0` through `0x8E246F` bind a Gnm texture and,
when applicable, its sampler to the vertex, hull, domain, geometry, pixel, or
compute stage. Their numeric stage values match SDK 5.500's `Gnm::ShaderStage`
ordering: compute 0, pixel 1, vertex 2, geometry 3, hull 5, and local 6. The
engine domain stage uses the Gnm local-shader stage.

Sampler slots are limited to 0 through 15. Binding flag bit `0x10` suppresses
sampler creation while retaining the texture binding. Vertex binding does not
receive that flag and always installs a sampler for a valid slot.

Writable flag bit 0 changes pixel and compute bindings from sampled textures
to read/write textures. Compute bindings select either the graphics CUE or the
standalone compute context from the render context's active command mode. Both
sampled and writable compute textures follow that selection.

The adapter boundary retains command-buffer layout and Gnm packet details,
while the reconstruction exposes the stage routing, sampler conditions, and
graphics-versus-compute decisions directly.
