# Depth/stencil target

`render_depth_stencil_target_create` at `0x6B2A80` creates the
`Depth/Stencil Buffer` in slot `0x10` of each 216-byte per-scene resource
block. Its descriptor switches between the standard and 40-bit format layouts
through `RenderSettings::use_40_bit_depth_stencil`.

Partial-frame blocks request an unassigned attachment index and can reuse the
corresponding target from an earlier block. The primary block starts at the
owner's next depth attachment. It reuses a prior target only when that target
also has an unassigned attachment; otherwise it allocates a new target at the
current owner attachment. After creation, the owner advances past the target's
assigned attachment range. Only the primary target is registered in the
owner's resource list.

The matching part of `render_target_resources_release` at `0x6AFFE0` invokes
the target's virtual deleting destructor and clears the block slot. Descriptor
assembly and the derived target's attachment-range fields remain behind narrow
adapters.
