# Source layout

The tree follows the original codebase's modules, taken from the object paths
in the reference linker map. Inside each module, code is grouped by a coherent
responsibility. Broad module folders contain domain folders rather than
accumulating implementation files. Names and classes follow
[naming.md](naming.md).

| Module | Contents | Domains so far |
|---|---|---|
| `src/math` | Math types | `color`, `hash`, `random`, `vector` |
| `src/utl` | Engine utilities | `containers`, `options`, `streams`, `text`, `threading`, `time` |
| `src/os` | Platform services | `files`, `memory` |
| `src/render` | Platform-neutral renderer (`Rnd*`) | `audio`, `buffers`, `context`, `debug`, `defaults`, `depth`, `distance_fields`, `frame`, `lighting`, `masking`, `meshes`, `postprocessing`, `queries`, `shaders`, `system`, `targets`, `textures`, `video` |
| `src/renderps4` | PS4 backend (`PS4*`) | `buffers`, `context`, `meshes`, `queries`, `shaders`, `system`, `textures`, `video` |
| `src/audio` | Audio engine and FMOD integration | `core`, `fmod` |
| `src/rockband`, `src/rb_*` | Game | planned; currently `src/game` |
| `src/ui` | UI system | `layout` |

Every renderer file now sits in a domain folder of `src/render` or
`src/renderps4`; the transitional `src/render/core`, `src/render/resources` and
`src/render/platform/orbis` trees are gone. A few files in those domains still
carry descriptive names (`render_debug_mode.cpp`, `shader_cache_validation.cpp`,
`primitive_mesh_set.cpp`, `bink_render_manager.cpp` and similar) until they
are converted to their original classes. Audio utilities are under
`src/audio/core`, and FMOD integration is under `src/audio/fmod`.

New work should enter the narrowest fitting domain in its original module.
Create a clearly named domain folder when none fits, and use
source-root-qualified includes when code crosses folders.

The temporary alias header used during the conversion
(`render/core/transition_aliases.h`) is gone; every type it aliased is now a
real class.
