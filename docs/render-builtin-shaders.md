# Built-in shader resources

Built-in graphics and compute shader ownership lives under
`src/render/resources/shaders`. The resource manager stores 35 named slots and
allocates each concrete object at the exact size observed in `eboot.elf`.
Every class-specific dispatch is source-owned beside its constructor, in the
domain that uses the shader, and the shared adapter boundary has been retired.

All built-in graphics resources now have source-owned constructors. The error,
basic, Bink conversion, bloom, blur, display-texture-cube, downsample, output
conversion, and render-test classes use the shared 20-byte shader-parameter
binding layout in their trailing state. FXAA and display-shading-mode are
312-byte objects with three post-base handles; display-sphere-map,
linearize-depth, and refine-scene-mask are 296-byte objects with one handle.
Stencil-scene-mask is 312 bytes, test-pattern is 320 bytes, and the
feature-gated DOF-sprite resource is 304 bytes. Every verified post-base field
preserves the original mix of `-1` invalid handles and zero-valued state.

All seventeen compute resources are source-owned as well. The ten compact
constructors cover blur classification,
depth-range calculation, DOF disc blur, SSAO, all four CMAA stages, signed
distance generation, and signed-distance classification. The remaining seven
constructors cover clear buffer, copy buffer, the three volumetric-scattering
passes, compute linear-depth conversion, and render testing. Their typed
20-byte permutation bindings and all verified invalid-handle and zero-state
fields now live in source. The compute-derived classes pass through the shared
compute-shader dispatch before installing their concrete dispatch. Their
verified trailing regions preserve intentional gaps, including the eight-byte
SSAO gap at object offset `0x148` and the deferred-volumetric gaps after its
binding and first three handles.

Every constructor first initializes the shared 288-byte primary-shader base,
then installs the concrete dispatch and initializes only its verified trailing
fields. This keeps base ownership, manager-list registration, lazy preparation,
and compiled-object teardown centralized in `primary_shader_resource.cpp`.

## Dispatch slots 7-10

Primary-shader dispatch tables have 11 slots, and every source dispatch now
models all of them. The later slots are consumed by permutation enumeration:

| Slot | Default | Meaning |
| ---: | ---: | --- |
| 7 | `0x6388F0` | Validate a permutation key for a stage; default accepts |
| 8 | `0x638900` | Bind a fallback program; default binds the error shader |
| 9 | `0x450870` | Supports six-slice render targets; default false |
| 10 | `0x450880` | Has a geometry program for single-slice draws; default false |

Overrides:

| Shader | Slot | Address | Behavior |
| --- | ---: | ---: | --- |
| Basic | 7 | `0x639E40` | Pixel stage: shading modes 0, 16, 17 only; 17 excludes alpha cut; red-as-alpha needs a texture |
| Basic | 9 | `0x639EF0` | True |
| Error | 8 | `0x63E820` | No fallback |
| Error | 9 | `0x63E810` | True |
| Downsample | 9 | `0x6364B0` | True |
| Blur | 7 | `0x635F00` | Pixel stage: power-of-two sample count >= 2; classification only with depth-aware blur |
| Output conversion | 7 | `0x636BF0` | Rejects every HMD-mask permutation |
| Clear/copy buffer | 7 | `0x637B60`, `0x6F3D50` | Compute stage: uint or float4 numeric type; invalid, 1D, or 2D texture type |
| DOF sprite | 10 | `0x6F3520` | True |
| Bink conversion | 9 | `0x5F4E80` | False, matching the default |

The error-shader permutation bind used by the default fallback (`0x63E6C0`)
remains an adapter boundary.
