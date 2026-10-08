# Default render lighting

The render-system initializer at `0x6BDCA0` creates the default scene resource,
two compute buffers, the camera, six fallback materials, and the lighting used
when a loaded scene does not provide its own setup. The cleaned reconstruction
currently covers the authored default-lighting scene and its mode switch.

## State layout

The function group consistently accesses these fields relative to its owning
render-state subobject:

| Offset | Meaning |
| --- | --- |
| `0x1D8` | Retained `RndSceneResource` for default lighting. |
| `0x1E0` | Global scene-settings component obtained from the scene root. |
| `0x1E8` | `RndLightProbeCom` from `default_probe`. |
| `0x1F0` | Directional-light object-ID vector. |
| `0x210` | Shadowed-spot object-ID vector. |
| `0x230` | Active light set: `0` for directional, `1` for shadowed spot. |
| `0x234` | Scale used for falloff distances and the spot transform position; defaults to `100.0`. |

The source uses `DefaultLightingState` instead of padding a partial structure
out to those offsets. The table preserves the binary evidence while keeping the
reconstructed control flow readable.

The default-resource constructor at `0x6BDB30` zeroes the resource pointers,
texture table, compute buffers, camera, materials, and light vectors. Its final
eight-byte store initializes the lighting mode to directional (`0`) and the
scale to the IEEE-754 value `100.0f` (`0x42C80000`).

## Scene load

`render_load_default_lighting` at `0x6BEF40` loads
`../../system/data/render/default_lighting.scene` as an `RndSceneResource`. It
requires the scene's global-settings component; a missing component causes the
resource to be released and the caller to build fallback lighting.

After loading, the function disables every authored `RndLightCom` and
`RndLightProbeCom`, then locates three named objects:

- `default_directional` contributes its object ID to the directional list.
- `default_spot_with_shadows` contributes its object ID to the shadowed-spot
  list after its falloff and transform are scaled.
- `default_probe` is enabled immediately and retained as the default probe.

The directional and spot lists hold four-byte scene object IDs rather than raw
pointers. `render_apply_default_lighting_mode` resolves each ID through the
scene and enables only the selected list.

## Scale behavior

The component metadata confirms the setter fields at offsets `0x18` and
`0x1C` in `RndLightProbeCom` are `falloff_start` and `falloff_end`. The default
probe uses `scale * 2` and `scale * 3` respectively.

The corresponding `RndLightSpotCom` fields are at offsets `0xEC` and `0xF0`.
The shadowed spot uses `scale` and `scale * 2`, then resets its transform from
the engine default and multiplies only the position by the same scale.

The accessor at `0x6BFEA0` resolves the first shadowed-spot ID and returns its
`shadow_offset` property at component offset `0x140`; it returns zero when the
spot list is empty.

## Fallback path

`render_create_fallback_default_lighting` at `0x6BF4F0` creates an object named
`default_directional_light`, attaches a directional-light component, sets its
intensity field to `2.0`, orients its transform from engine defaults, appends
its object ID to the directional list, and applies the active lighting mode.
Its transform construction remains in IDA until the involved math types and
global basis vectors are recovered. The high-level fallback path is now in the
cleaned source; a narrow transform adapter contains that remaining math detail.
