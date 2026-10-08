# Repository organization

Keep reconstructed source grouped by subsystem and responsibility. Do not add
new implementation files directly to a broad subsystem root such as
`src/render` or `src/audio`.

- Put platform-neutral renderer code under `src/render/core` or
  `src/render/resources`.
- Put PS4 renderer code under `src/render/platform/orbis`, then use the
  matching domain folder: `buffers`, `context`, `meshes`, `shaders`,
  `synchronization`, `system`, `textures`, or `video`.
- Put platform-neutral audio code under `src/audio/core`.
- Put FMOD code under `src/audio/fmod`, grouped into `api`, `input`, `io`,
  `mixing`, `playback`, `resources`, or `system`. Platform integration belongs
  under `src/audio/fmod/platform/<platform>`.
- Use source-root-qualified includes, such as
  `render/platform/orbis/context/orbis_render_context.h`, when crossing
  folders.
- Add a new domain folder when a coherent group would otherwise accumulate in
  a broad parent directory.
