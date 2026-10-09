# Orbis render-context state

The first three platform-specific state methods in the Orbis render-context
vtable are:
- slot 7, `PS4Context::_BeginFrameImpl` (`0x8E8850`), which restores the
  default pipeline state;
- slot 8, `_SetRenderTargetsImpl(RndTargetMode, const RenderTargetParams&)`
  (`0x8E8D20`), which binds a render-target set;
- slot 9, `_SetBlendModeImpl(RndBlendMode, const Hmx::Color&)` (`0x8E92D0`),
  which programs color blending. The blend color is passed but not read.

## Render-target binding

`RndContext::RenderTargetParams` is 288 bytes:

| Offset | Field |
| --- | --- |
| +0 | clear color (`Hmx::Color::GetZero()` by default) |
| +16, +20 | depth and stencil clear values |
| +24-+36 | viewport x, y, width and height |
| +40, +44 | depth range, 0 and 1 |
| +48 | `FixedVector<Target, 8>`; a target is the texture, a clear mode (1 clears) and an array slice (-1 for none) |
| +264, +272, +280 | depth texture, depth clear mode, depth slice |

The `RndTargetMode` selects how textures resolve to Gnm targets:
- **Mode 0** takes 2D textures and slices of 2D arrays. `PS4TextureArray2D`'s
  `GetRenderTarget(slice)` (`0x8E6590`) and `GetDepthStencilTarget(slice)`
  (`0x8E65C0`) set the target's array view to the slice.
- **Mode 2** takes cube textures.
- **Mode -1** binds nothing.

The method binds all eight color slots (null for unused ones), the depth
target, and `setupScreenViewport` with the right and bottom edges at least one
pixel past the origin.

It then clears the targets the binding marks:
- Each color target is cleared by `RndCShaderClearBuffer::Dispatch` with the
  binding's clear color as float4. Before the first, the method triggers
  `kEventTypeFlushAndInvalidateCbMeta`.
- The depth target is cleared by `_ClearDepthStencil`.

After any clear, the method allocates a label from the draw command buffer and
writes it at `kEopCsDone` with `kCacheActionWriteBackAndInvalidateL1andL2`. The
graphics pipe then waits for the label to equal 1.

## Blending

`_SetBlendModeImpl` maps the engine's blend-mode number to a packed Gnm
`BlendControl` and writes it to all eight color slots. Mode 11 builds the
control separately for each target; the other modes reuse one value.

| Mode | Enabled | Source multiplier | Function | Destination multiplier |
| --- | --- | --- | --- | --- |
| 0, SourceAlpha | Yes | Source alpha | Add | One minus source alpha |
| 1, SourceAlphaAdd | Yes | Source alpha | Add | One |
| 2, PremultipliedAlpha | Yes | One | Add | One minus source alpha |
| 3, Screen | Yes | One minus destination color | Add | One |
| 4, Destination | Yes | Zero | Add | One |
| 5, Source | No | One | Add | Zero |
| 6, Add | Yes | One | Add | One |
| 7 | Yes | One | Subtract | One |
| 8 | Yes | Destination color | Add | Zero |
| 9 | Yes | One | Subtract | One minus source color |
| 10 | Yes | One | Subtract | Source color |
| 11, per-target | Yes | Source alpha | Add | One |

`_BeginFrameImpl` installs the baseline before a frame records draw work:
1. It resets `SetupDraw`'s caches to -1, turns the GS mode off, and enables
   color writes (`kCbModeNormal`, `kRasterOpCopy`).
2. It binds a default `RenderTargetParams` in mode -1, which unbinds every
   target.
3. It selects the Source blend mode with `Hmx::Color::GetWhite()`.
4. It resets depth, stencil (masks `FF`), counter-clockwise winding, no
   culling and solid fill, and sets the render-target mask to `FFFF`.
5. It deselects the read-write and source textures of all six stages.

## Depth, stencil, and raster state

The setters at `0x8E9F60` and `0x8EA030` cache the engine depth/stencil modes
and rebuild three related Gnm registers together: `DepthStencilControl`, the
front/back `StencilControl`, and `StencilOpControl`. Stencil reference values
are stored directly. The ten authored read/write-mask indices map to `FF`,
`07`, `08`, `10`, `0F`, `1F`, `20`, `28`, `30`, and `C0`; an out-of-range
index becomes zero.

The raster setters at `0x8EA150`, `0x8EA1C0`, and `0x8EA230` likewise rebuild
one combined Gnm `PrimitiveSetup` register whenever winding, culling, or fill
changes. The cull mapping is none, back, and front. Filled polygons select Gnm
fill mode 2; disabling fill selects line mode 1 for both faces.

Color-write control at `0x8E96F0` takes an eight-bit render-target selection.
It expands each selected bit to one Gnm nibble: `F` writes RGBA, `7` writes RGB,
and zero disables color writes. The eight nibbles form the 32-bit mask passed
to `setRenderTargetMask`.

## Builders

The Gnm controls come from the map's `PS4RenderStateUtl` builders:
- **`InitBlendControl` (`0x8EC310`).** Covers the twelve blend modes the map
  names in `RndShaderGraphUtl::GetBlendModeName`'s table at `0x1911F20`:
  - SrcAlpha: `SrcAlpha + OneMinusSrcAlpha`
  - SrcAlphaAdd and DecalLitSrcAlpha: `SrcAlpha + One`
  - Pre-Mult Alpha: `One + OneMinusSrcAlpha`
  - Screen: `OneMinusDestColor + One`
  - Dst: `Zero + One`
  - Src: blending off, `One + Zero`
  - Add: `One + One`
  - Subtract: `One - One`
  - Multiply: `DestColor + Zero`
  - Lighten and Darken: `One max/min One`

  Decal-lit blending rebuilds the control for each of the eight targets.
- **`InitDepthStencilControl` (`0x8EC450`).** Depth modes 1 to 4 test with
  greater-or-equal (write), equal, greater-or-equal (no write), and always
  (write). Stencil modes 1 to 5 use always, equal, equal, not-equal and
  less-or-equal.
- **`InitStencilControl` (`0x8EC5A0`).** Uses the reference as both test and
  replacement value.
- **`InitStencilOpControl` (`0x8EC5B0`).** Modes 1 and 3 replace on pass.
- **`InitPrimitiveSetup` (`0x8EC600`).**

The setters cache the state at `0x40D98`-`0x40DB7`; the constructor's
defaults are depth and stencil off, counter-clockwise front faces, no
culling, solid fill, stencil masks `FF`, and all four targets writing RGBA.
