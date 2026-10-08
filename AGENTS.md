# Repository organization

This organization rule applies project-wide. Group every reconstructed source
file first by subsystem and then by a coherent responsibility. Broad subsystem
and layer folders contain domain folders rather than accumulating implementation
files. This includes `src/game`, `src/input`, `src/ui`, and future subsystems as
they grow, not only rendering, audio, or platform code.

- Put platform-neutral renderer code under a matching domain in
  `src/render/core`: `buffers`, `capture`, `context`, `debug`, `frame`, `meshes`,
  `platform`, `settings`, `shaders`, `synchronization`, `system`, `targets`,
  or `textures`.
- Group default renderer resources by responsibility under
  `src/render/resources`, such as `camera`, `lighting`, `materials`, `system`,
  and `textures`.
- Put PS4 renderer code under `src/render/platform/orbis`, then use the
  matching domain folder: `buffers`, `context`, `meshes`, `shaders`,
  `synchronization`, `system`, `textures`, or `video`.
- Put platform-neutral audio code under a matching domain in `src/audio/core`,
  such as `format`, `output`, or `runtime`.
- Put FMOD code under `src/audio/fmod`, grouped into `api`, `input`, `io`,
  `mixing`, `playback`, `resources`, or `system`. Platform integration belongs
  under `src/audio/fmod/platform/<platform>`.
- Use source-root-qualified includes, such as
  `render/platform/orbis/context/orbis_render_context.h`, when crossing
  folders.
- Add a new domain folder in any subsystem when a coherent responsibility would
  otherwise accumulate in a broad parent directory. Do not wait for the broad
  directory to become crowded before placing new work correctly.
- Put engine-wide utilities under a matching domain in `src/core`, such as
  `memory` for shared allocation APIs, `threading` for shared thread runtime
  code, and `time` for performance-counter access shared by rendering and
  audio.
