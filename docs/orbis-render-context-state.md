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
