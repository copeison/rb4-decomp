# Built-in shader resources

Built-in graphics and compute shader ownership lives under
`src/render/resources/shaders`. The resource manager stores 35 named slots and
allocates each concrete object at the exact size observed in `eboot.elf`.
Class-specific dispatch installation and the larger class constructors remain
focused boundaries while their layouts are recovered.

Eight compact graphics resources now have source-owned constructors. FXAA and
display-shading-mode are 312-byte objects with three post-base handles;
display-sphere-map, linearize-depth, and refine-scene-mask are 296-byte objects
with one handle. Stencil-scene-mask is 312 bytes, test-pattern is 320 bytes,
and the feature-gated DOF-sprite resource is 304 bytes. Their post-base fields
preserve the original mix of `-1` invalid handles and zero-valued state.

Every constructor first initializes the shared 288-byte primary-shader base,
then installs the concrete dispatch and initializes only its verified trailing
fields. This keeps base ownership, manager-list registration, lazy preparation,
and compiled-object teardown centralized in `primary_shader_resource.cpp`.
