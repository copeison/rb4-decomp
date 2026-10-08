# Source layout

The reconstruction tree groups code first by engine subsystem and then by a
coherent responsibility. This is a project-wide rule. Broad subsystem and layer
folders contain domain folders rather than accumulating implementation files,
including game, input, UI, rendering, audio, and future reconstructed systems.

Shared renderer code is divided under `src/render/core` into buffers, capture,
debug, frame, platform, settings, shaders, synchronization, system, targets,
and textures. Default resources are divided under `src/render/resources` into
camera, lighting, materials, system, and textures. The PS4 backend is rooted at
`src/render/platform/orbis` and split into buffers, context, meshes, shaders,
synchronization, system, textures, and video.

Platform-neutral audio utilities are divided under `src/audio/core` into
format, output, and runtime. FMOD integration is rooted at `src/audio/fmod` and
divided into its API surface, system state, I/O, mixing, playback, resources,
input, and platform integration.

New work in every subsystem should enter the narrowest existing domain folder.
Create a clearly named domain folder when no suitable one exists, and use
source-root-qualified includes when code crosses folder boundaries.
