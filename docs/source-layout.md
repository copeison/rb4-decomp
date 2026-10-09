# Source layout

The tree follows the original codebase's modules, taken from the object paths
in the reference linker map. Inside each module, code is grouped by a coherent
responsibility. Broad module folders contain domain folders rather than
accumulating implementation files. Names and classes follow
[naming.md](naming.md).

| Module | Contents | Domains so far |
|---|---|---|
| `src/math` | Math types | `color`, `hash`, `random` |
| `src/utl` | Engine utilities | `containers`, `options`, `streams`, `text`, `threading`, `time` |
| `src/os` | Platform services | `files`, `memory` |
| `src/render` | Platform-neutral renderer (`Rnd*`) | see below |
| `src/renderps4` | PS4 backend (`PS4*`) | planned; currently `src/render/platform/orbis` |
| `src/audio` | Audio engine and FMOD integration | `core`, `fmod` |
| `src/rockband`, `src/rb_*` | Game | planned; currently `src/game` |
| `src/ui` | UI system | `layout` |

The renderer is being converted to the original class names. Until each part
is converted, shared renderer code stays where it is now:

- Domains under `src/render/core`: buffers, capture, context, debug, frame,
  meshes, platform, settings, shaders, synchronization, system, targets and
  textures.
- Default resources under `src/render/resources`.
- Pass-specific folders such as `lighting`, `postprocessing`, `depth` and
  `masking`.

The PS4 backend is under `src/render/platform/orbis`. Audio utilities are under
`src/audio/core`, and FMOD integration is under `src/audio/fmod`.

New work should enter the narrowest fitting domain in its original module.
Create a clearly named domain folder when none fits, and use
source-root-qualified includes when code crosses folders.
