# Orbis shader factory

`orbis_create_shader` at `0x8D8B30` is virtual slot 31 of the Orbis render
system. It accepts the six-stage renderer enum and constructs the four shader
forms supported as independent Orbis objects:

| Value | Stage | Object size | Constructor | Embedded allocation name |
| ---: | --- | ---: | --- | --- |
| 0 | Vertex | 104 | `0x8E46E0` | `VShader` |
| 3 | Geometry | 72 | `0x8E40A0` | `GShader` |
| 4 | Pixel | 64 | `0x8E43D0` | `PShader` |
| 5 | Compute | 64 | `0x8E3D20` | `CShader` |

Hull and domain values 1 and 2 return null. Every platform constructor first
invokes the common shader constructor at `0x642270`, installs its stage-specific
vtable, and initializes its backend allocation fields. Shader loading at
`0x63B2B0` keeps six stage-indexed vectors and calls this factory with the
vector index, independently confirming the enum values.
