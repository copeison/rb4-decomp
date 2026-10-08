# Default render textures

`render_create_default_textures` at `0x6BDE60` creates seven fallback texture
families. The name table at `0x1936800` fixes their order:

| Index | Name | Extent |
| ---: | --- | ---: |
| 0 | `White` | 8 |
| 1 | `Black` | 8 |
| 2 | `Zero` | 8 |
| 3 | `Flat Normal` | 8 |
| 4 | `Error` | 64 |
| 5 | `Error Greyscale` | 64 |
| 6 | `Error Normal` | 64 |

Each entry creates one resource in seven dimensional families: 1D, 2D, 3D,
cube, 1D array, 2D array, and array cube. This accounts for the 49 consecutive
resource pointers at owner offsets `0x008` through `0x190`.

The binary stores those pointers dimension first as seven adjacent arrays. The
cleaned `DefaultTextureSet` groups them by semantic kind so all shapes for one
fallback value stay together.

The four normal defaults use an extent of 8. The three visible error patterns
use an extent of 64 and initialize alternate color values. Cubes and cube
arrays build six matching face descriptors. The cleaned implementation keeps
the dimension-specific allocation behind runtime adapters while preserving
the exact family order, names, and extents.

The descriptor type values are corroborated by the resource-class switch at
`0x6ACF40`: `0` through `5` map to `RndTexture1DResource`,
`RndTexture2DResource`, `RndTexture3DResource`, `RndTextureCubeResource`,
`RndTextureArray1DResource`, and `RndTextureArray2DResource`. Descriptor type
`7` is the six-face array form used by the light-probe texture-array path.

`render_get_default_texture` at `0x6C00F0` indexes these tables by the raw shape
values `0` through `5` and `7`, returning null for unsupported values.
