# Render debug modes

Each render target owned by a frame owner stores two debug selections. Offset
`0x10` is the 32-value material draw mode, and offset `0x14` is the 74-value
buffer debug view. The owner helpers read the first target and apply setters to
every target so split or multi-output rendering remains consistent.

The material draw-mode table at `0x192F7C0` includes lighting breakdowns,
overdraw and batching views, vertex attributes, UV channels, and individual
material properties. Its parser at `0x645E80` uses case-sensitive comparison.

The buffer debug-view table at `0x1936E20` includes GBuffer channels, depth and
stencil views, lighting accumulation, tiled-light diagnostics, sky buffers,
shadow stages, downsampled buffers, masks, ambient occlusion, and the function
table. Its parser at `0x6B5510` is case-insensitive.

The complete ordered names are preserved in `src/render/core/render_debug_mode.cpp`.
This ordering matters because the renderer stores and passes the numeric table
index, including when the screenshot path mirrors the active view.
