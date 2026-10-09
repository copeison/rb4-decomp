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

The validator's hash inputs are now source-owned. They are the backend-state
declaration hash (`0x6446A0`), the source-file hash (`0x6456C0`), and the
define matcher (`0x63EB70`). Generated text is never materialized: it is
streamed into an FNV-1a hashing text stream that sign-extends each byte.
Earlier constant-block hashing zero-extended bytes; it now shares the
corrected helper.

## Backend initialization

`RndShader::_InitShaderCollection` (`0x638430`) runs under the
recursive "hx crit sec" critical section. It is created by the static
initializer at `0x639810` and torn down at `0x12E70`, which releases any
outstanding holds. On the first call for a shader, the initializer:

1. Records the backend path (slot 3) at `+0xD0`.
2. Resolves the path into a symbol (`0x1AF950`).
3. Asks the generated-file system (`0x1AD8B0`) for the compiled cache path and
   whether it is stale.
4. Loads the cache with `render_primary_shader_load_cache` (`0x638A40`).
5. Marks the shader compiled whatever the outcome.

Archive mode (byte `0x19E4558`) trusts caches and skips validation. Otherwise
a stale, missing, or mismatched cache is retried without validation, because
retail builds cannot compile shaders.

A cache file starts with a non-zero marker and the shader variant. Four
32-bit hashes follow:

- the global shader-constant source hash (resource manager `+680`);
- the permutation layout hash;
- the constant-block and backend-state declaration hash;
- the HLSL source-file hash.

Next come the global defines the cache was built with, read as symbols under a
thread-local heap scope (`0x37AA30`/`0x37AAF0`). The compiled objects come
last. Validation compares every hash and requires each cached define to match
the resource manager's sorted define list at `+688`.

## Permutation layout hash

`RndShader::_ChecksumDefines` (`0x638D70`) streams the following into
FNV-1a, with every byte sign-extended:

1. The generated constant definitions.
2. For each of the six parameter registries, its record count, then each
   record's name and its first and last values.
3. The key of every accepted permutation, for each graphics stage enabled by
   the shader variant.

Compute permutations are not hashed. Enumeration (`0x63D100`) walks the global
registry first, then the stage registry; hull and domain share registry 2.
Enabled (global) records pack into the high 32 bits of the key.

## Permutation binding

`RndShader::_SelectShaderCollection` (`0x638920`) takes five 64-bit program keys,
indexed by variant program bit (vertex, tessellation, geometry, pixel,
compute). It first initializes the backend if needed. The context's slice mode
at `+0x10` maps through `{1, 2, 6, 1, ...}` to an `HX_NUM_RT_SLICES` value; -1
means one slice and out-of-range modes mean zero. That value goes into the
global half of every key. Single-slice draws drop the geometry program unless
slot 10 reports one.

`render_compiled_shader_objects_bind` (`0x63B500`) then updates the context's
active-stage byte at `+0x4960`. Stages that are no longer used are unbound
through context slot 28. For each used stage, it binary-searches that stage's
compiled objects by permutation key, stamps the render-system frame epoch, and
calls the program's bind slot. Any miss fails the bind, and the shader's slot-8
fallback runs. For most shaders that fallback binds the error shader
(`0x63E6C0`) with the context's shading mode at `+0x4A18`.
