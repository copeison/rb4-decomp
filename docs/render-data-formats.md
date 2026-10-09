# Render data formats

The engine represents a requested texture format with a 20-byte descriptor:
total bit width, channel-layout code, numeric-type code, layout code, and an
optional platform variant. The inverse mapping at `0x68DB80` is source-owned in
`src/render/textures/RndPixelFormat.cpp` for all format IDs from the
executable.

Format IDs `0` through `26` and `53` through `56` use fixed descriptor tuples.
IDs `27` through `52` use the engine's compact variants `1` through `17`, and
IDs `57` through `84` expose two layout families over variants `18` through
`31`. The recovered 31-entry variant-width table includes the sub-byte 2-bit
and 4-bit formats as well as the un-sized platform variants.

Exact descriptor lookup at `0x68E070` is source-owned too. It maps the fixed
bit-width/channel/numeric tuples and both compact-variant layout families back
to their format IDs, returning `-1` for unsupported tuples. The mapping is the
inverse of the recovered descriptor table across IDs `0` through `84`.

Mip source allocation uses the recovered total bit width to calculate
`width * height * depth * bits / 8`. Descriptor-to-format resolution at
`0x68E4D0` and `0x68E550` is source-owned. It checks the active render
platform's supported-format bitsets, retries the original channel-layout
fallback sequence, applies the resource-class-eight numeric conversion, and
searches the packed 24/32/40-bit family in its original preference order.

Float-image conversion at `0x684960` is also source-owned. Fixed formats use
the original channel-order matrix for R, RG/GR, RGB/BGR, RGBA/RGBX,
BGRA/BGRX, ARGB, and XRGB storage. Normalized 8- and 16-bit components clamp
and truncate exactly as the conversion kernels do; floating formats preserve
32-bit values or use the recovered truncating IEEE-754 half conversion.
Layout-two formats encode the RGB channels with the standard linear-to-sRGB
transfer curve while leaving alpha unchanged. Compact platform-variant
formats remain rejected by this CPU conversion path, matching the executable.
