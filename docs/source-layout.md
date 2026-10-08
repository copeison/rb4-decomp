# Source layout

The reconstruction tree groups code first by engine subsystem, then by platform
and responsibility. Broad roots such as `src/render` and `src/audio` contain
folders rather than implementation files.

Renderer code shared across platforms lives in `src/render/core`, while default
engine resources live in `src/render/resources`. The PS4 backend is rooted at
`src/render/platform/orbis` and split into buffers, context, meshes, shaders,
synchronization, system, textures, and video.

Platform-neutral audio utilities live in `src/audio/core`. FMOD integration is
rooted at `src/audio/fmod` and divided into its API surface, system state, I/O,
mixing, playback, resources, input, and platform integration. New reconstructed
files should follow the same hierarchy and use source-root-qualified includes
when they cross folder boundaries.
