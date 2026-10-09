# Repository organization

## Work continuity

- Continue reconstruction autonomously across milestones. Do not stop after a
  progress report, successful build, documentation update, or commit.
- Stop only when further progress genuinely requires information, files,
  credentials, hardware interaction, or a decision that only the user can
  provide. State the exact blocker when that happens.
- Keep making focused milestone commits, updating the documentation, and
  validating changes without waiting for the user to ask again.

## Original names

The source is reconstructed as the original C++ codebase. Names, classes and
module layout follow the linker map of an older release build
(`files/rockband_ps4_r.map`, never committed). The rules are in
`docs/naming.md`, and they apply to all new and converted code:

- Use the original class, method, free-function and global names in the global
  namespace. Mark names that the map does not contain.
- Write real classes. Declare virtual methods in the binary's vtable order, so
  the compiler-generated vtable matches the recovered slots.
- Name each file after its original object file (`RndShaderBlur.cpp`).

## Folder layout

Group every source file first by its original module (the directory of its
object file in the map), then by a coherent responsibility inside that module.
Broad module folders contain domain folders rather than accumulating
implementation files.

- `src/math`, `src/utl` and `src/os` hold the engine foundation: the math
  types, the streams, symbols and threads, and the file system, memory and
  joypads.
- `src/render` holds the platform-neutral renderer (`Rnd*`), grouped into
  domains such as `buffers`, `context`, `debug`, `frame`, `meshes`, `shaders`,
  `system`, `targets`, `textures`, `lighting`, `postprocessing` and
  `defaults`.
- `src/renderps4` holds the PS4 backend (`PS4*`), grouped into `buffers`,
  `context`, `meshes`, `shaders`, `system`, `textures` and `video`.
- `src/audio` holds the audio engine, with FMOD integration under
  `src/audio/fmod`. `src/mic` holds the microphone layer.
- The game lives under `src/rockband` (startup and main loop) and the `rb_*`
  modules. `src/ui` holds the UI system.
- Joypads and controllers live under `src/os/joypads`.
- Use source-root-qualified includes, such as
  `renderps4/context/PS4Context.h`, when crossing folders.
- Add a new domain folder whenever a coherent responsibility would otherwise
  accumulate in a broad module directory.
