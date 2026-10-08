# Orbis render-context state

The first three platform-specific state methods in the Orbis render-context
vtable are now identified. Slot 7 at `0x8E8850` restores the default pipeline
state, slot 8 at `0x8E8D20` binds a render-target set, and slot 9 at `0x8E92D0`
programs color blending.

Render-target binding resolves at most eight engine color attachments into Gnm
render targets and resolves the optional depth attachment separately. It binds
all eight hardware color slots, using null targets for unused slots, then binds
depth and derives the hardware viewport and scissor from the floating-point
rectangle stored in the binding. The right and bottom edges are computed from
the nonnegative width and height.

Attachments marked for transition cause event type 46 to be emitted once
before their preparation commands. The depth target has a separate preparation
path. If either color or depth preparation records work, the method ends the
transition sequence with the matching synchronization packet.

`orbis_render_context_set_blend_mode` maps the engine's blend-mode number to a
packed Gnm `BlendControl` and writes it to all eight color slots. Mode 11 builds
the control separately for each target; the other modes reuse one value.

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

The reset method clears the context's cached state, unbinds render targets,
selects the Source blend mode, restores raster and depth/stencil defaults,
disables stream output, and clears shader-resource bindings for all six shader
stages. This is the baseline installed before a new frame records draw work.

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
