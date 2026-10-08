# Default render materials

`render_create_default_materials` at `0x6BEC50` creates six named material
objects inside the default render scene. Every material is marked `Unique`,
which is value `2` in the `sharing_type` metadata enum.

| Object | Shader graph | Explicit blend mode |
| --- | --- | --- |
| `default_mat_unlit` | `default_unlit.sgraph` | Existing default |
| `default_mat_add` | `default_unlit.sgraph` | `Add (Linear Dodge)` |
| `default_mat_lit` | `default.sgraph` | Existing default |
| `default_text_mat` | `default_text_unlit.sgraph` | Existing default |
| `default_particle_mat` | `default_particle.sgraph` | `Add (Linear Dodge)` |
| `default_decal_mat` | `default_decal_lit.sgraph` | `Src` |

The blend names come from the material metadata table at `0x1911F20`. Its
ordered entries establish raw value `5` as `Src` and raw value `6` as
`Add (Linear Dodge)`, matching the two constants used by the initializer.

The cleaned code represents the six pointers as `DefaultMaterialSet` and uses
one helper for the repeated create, attach, set-unique, and shader assignment
sequence.
