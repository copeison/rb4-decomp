# Orbis shader state

`orbis_render_context_set_sampler` at `0x8EA830` builds one 16-byte Gnm sampler
descriptor and binds it to the requested shader slot. The implementation
supports the vertex, pixel, and compute stages. Compute samplers are routed to
the active standalone compute context while a compute dispatch is being
recorded; otherwise they use the compute stage of the graphics context.

The five engine address modes map to Gnm wrap on all three axes, clamp to the
last texel, mirror, clamp-border with opaque black, and clamp-border with opaque
white. Seven numeric filter modes select the point/bilinear/anisotropic fields
and the maximum anisotropy bits in the packed descriptor. Unsupported values
retain the initialized sampler defaults.

`orbis_render_context_clear_shader` at `0x8EA920` removes the shader bound to
one supported stage. It has dedicated Gnmx paths for vertex, geometry, pixel,
and compute shaders. Hull and domain stages are ignored because the Orbis
shader factory does not create either stage. Clearing the pixel shader also
updates the context's cached pixel-shader command state.

The resource-clear methods use a six-bit mask whose bit numbers match the
engine shader-stage enum. `0x8E9810` clears all 128 read/write texture slots for
each selected stage. `0x8E9940` clears all 128 read-only texture slots and all
128 read-only buffer slots. Both operate only while the graphics context owns
resource state; standalone compute recording maintains its resources in the
selected compute context instead.
