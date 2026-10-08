# Built-in shader resources

Built-in graphics and compute shader ownership lives under
`src/render/resources/shaders`. The resource manager stores 35 named slots and
allocates each concrete object at the exact size observed in `eboot.elf`.
Class-specific dispatch installation and seven larger compute constructors
remain focused boundaries while their layouts are recovered.

All built-in graphics resources now have source-owned constructors. The error,
basic, Bink conversion, bloom, blur, display-texture-cube, downsample, output
conversion, and render-test classes use the shared 20-byte shader-parameter
binding layout in their trailing state. FXAA and display-shading-mode are
312-byte objects with three post-base handles; display-sphere-map,
linearize-depth, and refine-scene-mask are 296-byte objects with one handle.
Stencil-scene-mask is 312 bytes, test-pattern is 320 bytes, and the
feature-gated DOF-sprite resource is 304 bytes. Every verified post-base field
preserves the original mix of `-1` invalid handles and zero-valued state.

Ten compact compute resources are source-owned as well: blur classification,
depth-range calculation, DOF disc blur, SSAO, all four CMAA stages, signed
distance generation, and signed-distance classification. The compute-derived
classes pass through the shared compute-shader dispatch before installing
their concrete dispatch. Their verified trailing regions preserve contiguous
invalid-handle runs, explicit zero state, and the untouched eight-byte SSAO
gap at object offset `0x148`.

Every constructor first initializes the shared 288-byte primary-shader base,
then installs the concrete dispatch and initializes only its verified trailing
fields. This keeps base ownership, manager-list registration, lazy preparation,
and compiled-object teardown centralized in `primary_shader_resource.cpp`.
