# Render shader lifecycle

The common render shader object is 40 bytes. It stores its dispatch table at
offset 0, owner at 8, initialized flag at 16, variant index at 24, and metadata
pointer at 32. Construction at `0x642270` clears the optional fields and sets
the variant index to -1.

`render_shader_initialize` at `0x6422C0` updates the owner and metadata,
releases any initialized backend, and dispatches the supplied binary to the
platform initializer. A null binary is accepted as an empty successful state.
The initialized flag records whether the platform initializer succeeded.

`render_shader_release` at `0x642310` calls virtual slot 4 only when the flag
is set, then clears it. This supplies the release step invoked by all four
Orbis shader destructors and by reinitialization. The base destructor itself
is empty; the deleting destructor releases the object storage.

The generic factory at `0x642250` dispatches through the active render system.
`RenderShaderStage` and the base layout therefore live under `render/shaders`,
while compiled binary parsing and command-context binding remain under the
Orbis shader backend.
