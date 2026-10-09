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
installation and backend initialization remain narrow adapter boundaries. The
shared resource-name owner is source-owned, including its capacity-prefixed
string allocation and empty-string representation.

The 20-byte binding, 40-byte registry, and 240-byte six-registry set are shared
with the resource manager through `shader_parameter_registry.h`. This keeps
the encoded range, bit cursor, and enabled-state layout consistent wherever
shader permutation fields are registered. Insertion at `0x63C3F0` is
source-owned: ranges use an exclusive upper bound, the routine computes the
minimum bit width and shifted mask, advances the registry bit cursor, and
copies the resulting 20-byte binding to the caller. Its 40-byte record array
doubles capacity while preserving owned resource names.

## Compiled-shader cache loading

`render_compiled_shader_objects_load` at `0x63B2B0` fills the six stage-indexed
compiled-object vectors from a `BinStream`. It first releases every existing
object through its virtual deleting slot. It then requires a cache version of
at least 3 and a stage count of at least 4. Each of the six stages stores a
32-bit object count. Every record holds a discarded 32-bit field, a 64-bit owner
value, and a 32-bit binary size. The loader creates the stage's platform shader
and initializes it with the stream itself as the binary source, or with null
when the size is zero. The shader's backend name is the metadata. The original
compares the active render API with itself before loading, so the seek-skip
branch for foreign records is unreachable on this build.

The backend initializer at `0x638430` and its cache validator at `0x638A40`
remain unreconstructed. The validator opens the cache as a 592-byte
`FileStream` (`0x2443A0`). It compares the record count and the shader's slot-6
value, then the render-system version at `+0xC98` and the parameter hash from
`0x638D70`. Next it checks an FNV-1a hash (basis `0x811C9DC5`) of the generated
constant-block and backend-state source, a hash of the backend path
(`0x6456C0`), and the platform records at render-system `+0xCA0` (`0x63EB70`).
Only then does it call the loader.
