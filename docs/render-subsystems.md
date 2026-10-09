# Render subsystems

`Rnd::Init` (`0x402C30`) starts the renderer's subsystems in order, and
`Rnd::Terminate` stops them. These are reconstructed:

| Subsystem | Init | Terminate | Creates |
| --- | --- | --- | --- |
| `RndDrawUtl` | `0x3DF170` | `0x3DF820` | the debug-draw meshes, including six facing quads |
| `RndEditorDrawUtl` | `0x461D10` | `0x461F50` | the control-point texture and arrow mesh |
| `RndDebugFont` | `0x65C1A0` | `0x65C790` | the built-in bitmap font from `gBitmapData` |
| `RndDebugFont` extended fonts | `0x65CAF0` | `0x65CE70` | the font resources for the system language |
| `RndOcclusionQueryMgr` | `0x5F7ED0` | `0x5F7FB0` | the query sphere and its two shaders |
| `RndOverlayMgr` | `0x5F9150` | | the debug overlays |
| `RndBufferInspection` | `0x6B54A0` | `0x6B54E0` | the inspection shader |
| `RndTexturedQuadCom` | `0x44C8B0` (post-init) | `0x44C930` | the shared quad mesh |
| bounce-plane editor | `0x601880` | `0x601910` | a box mesh |

`RndPixelCanvas` is a real class with a vtable. It is created uninitialized
or filled with a colour, and `RndPixelData::CopyFrom` copies pixel data
between textures.

## Quirks

- **`RndDrawUtl::Terminate`** never frees its nested-cone mesh. This is kept
  from the binary.
- **The control-point texture** is a `ResourcePtr` in the source, so the
  source registers an exit destructor the binary does not have.

## Invented names

These names are not in the map:
- the extended-font functions;
- the bounce-plane editor helpers;
- `RndShaderOcclusionQuery`;
- most overlay classes and their bases. These are declared with their sizes,
  but their constructors are not reconstructed.

## Not yet reconstructed

These are declared only:
- the overlay constructors.

## Fonts

`RndFont` keeps one size per resolution. Each size has:
- the tile size, glyph height, space size, spacing and fixed width;
- a list of pages and sorted kerning pairs.

Each `RndFontPage` holds a texture and its glyphs. A glyph's UV rectangle is
left, top, width and height.

`Finalize` sorts the kerning pairs with `std::sort`; the binary uses EASTL's
sort, so pairs with equal keys may end up in a different order.
