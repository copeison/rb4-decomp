# Volumetric-scattering textures

`render_volumetric_scattering_textures_create` at `0x6B3730` runs when the
volumetric-scattering setting is enabled and resource flag `0x1000` selects the
per-scene resource group. It divides the output width and height by
`volumetric_scattering_tile_size`, rounds up, then aligns both tile dimensions
to a multiple of eight.

Each 216-byte per-scene block receives `VScat Inscattering` and `VScat Accum
Scattering` 3D textures at depths 128, 256, and 512. The slots are block
offsets `0x90`-`0xA0` and `0xC0`-`0xD0`. Accumulated-scattering source data is
an alternating 64-bit half-float pattern. Even voxels contain `(0, 1, 1, 1)`;
odd voxels contain `(1, 0.5, 0, 1)`.

When the resource owner's mode field is three, the routine also creates
`VScat Inscattering (Stereo)` textures at the same three depths in block
offsets `0xA8`-`0xB8`. The mode's broader meaning remains unresolved, so its
exact test stays behind a narrow boolean adapter.

With a previous block, all textures reuse their matching old slots. During
first-time creation, the depth-256 and depth-128 mono textures reuse their
family's newly created depth-512 texture. Stereo textures receive no reuse
input. The matching release loop virtually deletes and clears all nine slots.
