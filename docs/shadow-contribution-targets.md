# Shadow-contribution targets

`RndBufferCollection::_AllocShadowBlurBuffers` at `0x6B1970` returns immediately
when `max_shadow_contrib_buffers` is zero. Otherwise it creates a `Shadow
Contrib TexArray` whose layer count comes from that setting.

At resolutions above 1,920 by 1,080, the shadow-contribution extent is halved
and the routine also creates `Shadow Contrib Stencil` and a second `Shadow
Contrib Scratch` target. At smaller resolutions it uses the full extent and
creates only the primary scratch target. Two `Shadow Soften Tiles` targets are
always created at the contribution extent divided by
`shadow_soften_tile_size`, rounding each dimension up.

The texture array, stencil, scratch pair, and soften-tile pair occupy owner
offsets `0x240`, `0x248`, `0x250`/`0x258`, and `0x260`/`0x268`. Each resource
uses the matching slot from a previous owner as its reuse input and is
registered with the new owner. Resource flag `0x400` controls this group. The
matching portion of `RndBufferCollection::Destroy` at `0x6AFFE0` virtually
deletes and clears all six slots.

The exact target descriptors, including the texture-array and stencil factory
differences, remain behind a narrow adapter.
