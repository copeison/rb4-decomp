# Render data formats

The engine represents a requested texture format with a 20-byte descriptor:
total bit width, channel-layout code, numeric-type code, layout code, and an
optional platform variant. The inverse mapping at `0x68DB80` is source-owned in
`src/render/core/textures/render_data_format.cpp` for all format IDs from the
executable.

Format IDs `0` through `26` and `53` through `56` use fixed descriptor tuples.
IDs `27` through `52` use the engine's compact variants `1` through `17`, and
IDs `57` through `84` expose two layout families over variants `18` through
`31`. The recovered 31-entry variant-width table includes the sub-byte 2-bit
and 4-bit formats as well as the un-sized platform variants.

Mip source allocation uses the recovered total bit width to calculate
`width * height * depth * bits / 8`. Descriptor-to-format resolution at
`0x68E4D0` still remains a focused boundary because it also consults the
active render platform's supported-format masks and applies several fallback
transformations.
