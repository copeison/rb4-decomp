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

The four ordinary defaults use an extent of 8. Their float colors are white,
opaque black, transparent black, and the `(0.5, 0.5, 1, 1)` flat normal. The
three error textures use an extent of 64 and alternate every eight texels in
all dimensions:

| Kind | Primary | Secondary |
| --- | --- | --- |
| Error | `(1, 0.5, 0, 1)` | `(0, 1, 1, 1)` |
| Error Greyscale | `(0.25, 0.25, 0.25, 1)` | `(0.75, 0.75, 0.75, 1)` |
| Error Normal | `(1, 0, 0, 1)` | `(0, 1, 0, 1)` |

All families resolve the `{32, 4, 0, 2, -1}` data-format descriptor for
resource class 7. A temporary float4 image is converted into owned mip source
data for each dimensional extent. The 1D and 2D mip data is shared by the
matching single-layer array descriptors, while cubes and cube arrays each
build six owned face descriptors. The normal families set texture creation
mode 3; every family sets creation values 8 and 9 to 2 and 1 respectively.

The cleaned implementation calls each dimension's typed descriptor constructor
and common 1D, 2D, 3D, cube, 1D-array, 2D-array, and cube-array factory
directly. Only the shared mip pixel-format conversion remains at the engine
boundary.

The descriptor type values are corroborated by the resource-class switch at
`0x6ACF40`: `0` through `5` map to `RndTexture1DResource`,
`RndTexture2DResource`, `RndTexture3DResource`, `RndTextureCubeResource`,
`RndTextureArray1DResource`, and `RndTextureArray2DResource`. Descriptor type
`7` is the six-face array form used by the light-probe texture-array path.

`render_get_default_texture` at `0x6C00F0` indexes these tables by the raw shape
values `0` through `5` and `7`, returning the common `RenderTexture` base or
null for unsupported values.

## Table layout

The default textures are stored by shape first: one array of seven pointers
per texture shape, indexed by default type. The address is
`this + 8 + 56*shape + 8*default`.

| Array | Offset |
| --- | --- |
| `mTextures1D` | `+8` |
| `mTextures2D` | `+64` |
| `mTextures3D` | `+120` |
| `mTexturesCube` | `+176` |
| `mTexturesArray1D` | `+232` |
| `mTexturesArray2D` | `+288` |
| `mTexturesArrayCube` | `+344` |

`_CreateTextures` (`0x6BDE60`) fills a pixel canvas with each default's
colour, overlays an 8-texel checkerboard for the error textures
(`RndTextureUtl::FillCheckerboard`), and builds every shape in turn.

The checkerboard's row loop is bounded by the height rather than the width,
so wide one-row textures get only their first texel patterned. This is kept
from the binary.

The direct default binding in the tiled-light dispatches is the zero 2D
texture.
