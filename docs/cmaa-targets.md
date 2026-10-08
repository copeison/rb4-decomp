# CMAA targets

`render_cmaa_targets_create` at `0x6B1E60` runs when the active renderer
reports CMAA support. It creates two full-resolution `CMAA Edge Buffer`
targets and a `CMAA Compressed Edge Buffer` whose width and height are halved
with a minimum of one pixel. A `CMAA Color Buffer` is recreated only when the
previous resource owner supplies one; its descriptor follows the
`use_64_bit_light_accum` setting.

The owner's state slot at `0x218` is reset after allocation. The color, two
edge, and compressed-edge targets occupy offsets `0x220`, `0x228`, `0x230`,
and `0x238`. Every created target is registered with the owner. The matching
section of `render_target_resources_release` at `0x6AFFE0` invokes their
virtual deleting destructors and clears all four slots.

The platform capability test and exact target descriptors remain behind
narrow adapters.
