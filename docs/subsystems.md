# Subsystem map

This is the first coarse map of the executable. Address ranges are working
clusters, not final translation-unit boundaries. Each anchor below comes from a
string cross-reference inspected in IDA; names are retained only when the
evidence supports them.

| Subsystem | Address anchors | Evidence |
| --- | --- | --- |
| Runtime and game loop | `0x20`-`0x3DC`, entry `0x920` | Static initialization, `config/rockband.dta`, and the recovered initialize/frame loop. |
| Core data and scripting | `0x239CC0`, `0x23FF10` | Assertions reference `core/src/utl/DataNode.cpp` and `DataUtl.cpp`. |
| Rendering and lighting | `0x6BDCA0`-`0x6C0160` | Builds default resources, loads `system/data/render/default_lighting.scene`, and configures directional, spot, and probe lights. |
| Voting and setlists UI | `0x9751E0`, `0x975ED0`, `0x9DE110`, `0x9DE680` | Direct references to `ui/voting/vote_setlist.scene`. |
| Overshell UI | `0xBF7B30` | Direct reference to `ui/overshell/overshell.scene`. |
| Audio DSP | `0x1099BE0` | Three assertions name Positive Grid's `FDConvolver.cpp`; the function builds FFT plans and partition buffers. |
| RakNet networking | approximately `0x11B7500`-`0x11D2000` | Dense references to `RakPeer.cpp` and `ReliabilityLayer.cpp`, plus paths for RakNet plugins and serializers. |

## Named anchors

- `FDConvolver_ctor` at `0x1099BE0` is supported by its object initialization,
  block-size validation, FFT-plan creation, and source assertions.
- `render_load_default_lighting` at `0x6BEF40` is descriptive. It is supported
  by the scene path and the lighting objects accessed after loading it.
- `render_apply_default_lighting_mode` at `0x6BFA60` is supported by the two
  object-ID lists and the mode comparisons against `0` and `1`.

## Embedded third-party code

The source strings identify RakNet networking and Positive Grid convolution
code as statically linked components. Keeping these regions separate from
Harmonix game code should make matching public upstream source possible and
avoid manually reconstructing known library implementations.

Other embedded headers point to EASTL-style containers and shader sources. The
shader paths describe runtime assets and do not by themselves establish C++
translation-unit boundaries.
