# Primary shader resources

Common primary-shader ownership now lives under
`src/render/resources/shaders`. The 288-byte base resource places six 32-byte
compiled-object arrays at offset `0x10`, an unowned backend-name pointer at
`0xD0`, four support-object pointers at `0xD8` through `0xF0`, the
`HX_NUM_RT_SLICES` binding at `0xF8`, and its resource-manager list link at
`0x110`.

Construction at `0x6380B0` installs the base dispatch, clears the variant and
compiled state, initializes all six object arrays, clears the support owners
and render-target-slice binding, and self-links the manager node. Lazy
preparation at `0x638270` allocates the name owner, parameter registries,
compile state, and backend state before passing them to virtual slot `0x20`.
Finalization at `0x6383D0` applies the startup-option gates for virtual shader
modes zero and one before entering the remaining backend initialization leaf.

Destruction at `0x638110` is source-owned. It destroys every 32-byte name
record, releases the six parameter registries in reverse order, frees compile
state entry storage, and walks all 24 backend-state arrays in reverse order.
It then unlinks the manager node, dynamically releases the compiled objects,
and frees the six compiled-array capacities in reverse order. Each owning
support pointer is cleared as soon as it is released. The base dispatch
installation, resource-name lifetime, and backend initialization remain
narrow adapter boundaries.

The 20-byte binding, 40-byte registry, and 240-byte six-registry set are shared
with the resource manager through `shader_parameter_registry.h`. This keeps
the encoded range, bit cursor, and enabled-state layout consistent wherever
shader permutation fields are registered. Insertion at `0x63C3F0` is
source-owned: ranges use an exclusive upper bound, the routine computes the
minimum bit width and shifted mask, advances the registry bit cursor, and
copies the resulting 20-byte binding to the caller. Its 40-byte record array
doubles capacity while preserving owned resource names.
