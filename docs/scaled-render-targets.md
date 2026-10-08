# Scaled render targets

`render_scaled_targets_create` at `0x6B12D0` creates two generic render targets
at each of half, quarter, and eighth resolution. Shifted dimensions have a
minimum of one pixel. The target names stored in the binary are `Half-Size
Buffer`, `Quarter-Size Buffer`, and `Eighth-Size Buffer`.

The pairs occupy resource-owner offsets `0x1D0`/`0x1D8`, `0x1E0`/`0x1E8`, and
`0x1F0`/`0x1F8`. When a previous resource owner is supplied, each target uses
the matching prior slot as its reuse input. First-time creation passes no reuse
input. Every result is registered with the new owner.

Resource flag `0x100` controls creation. The target format depends on a render
system capability byte at offset `0x1A`; the exact descriptor and format
selection remain behind a narrow adapter. The matching portion of
`render_target_resources_release` at `0x6AFFE0` virtually deletes and clears
all six slots.
