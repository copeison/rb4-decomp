# Light-probe accumulation target

The `0x6B07DC` portion of `render_target_resources_initialize` handles resource
flag `0x10`. When volumetric scattering is disabled, it creates one full-size
`Light Probe Accum Buffer`, stores it at owner offset `0x1A8`, and registers it
with the owner. When volumetric scattering is enabled, the flag creates no
separate target.

Creation accepts the target at the same offset from a previous resource owner
for reuse. The matching portion of `render_target_resources_release` at
`0x6AFFE0` virtually deletes and clears the slot. The exact target descriptor
remains behind a narrow adapter.
