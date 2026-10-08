# Render resource manager

The render system embeds a 712-byte resource-manager block at offset `0x9F0`.
Its constructor at `0x63F180` begins with four fixed 20-byte shader-parameter
bindings. Each binding contains its encoded value range and bit placement plus
an enabled byte at offset `0x10`; construction clears all four.

The following 304-byte shader-constant state begins with an eight-byte phase
word, followed by eight named constant-block owners, their uniform handles,
three transient constant blocks, and a constant-name registry. The original
constructor initializes the first 34 words with the observed `-1` sentinels,
then clears the phase word, block-owner slots, transient blocks, and registry.
The 304-byte runtime region begins at manager offset `0x180`; it contains the
shader-parameter registry set, function-table texture, and 35 dynamic resource
slots. The constructor then allocates a 32-byte empty pointer-array owner and
two 16-byte intrusive-list sentinels; each list node is self-linked when empty.

The destructor at `0x63F350` unlinks and releases the two sentinel nodes, frees
the pointer array's backing storage by its capacity, and releases the array
owner. The top-level initializer at `0x63F400` is source-owned. It advances the
first manager phase, initializes the constant registry, parameter registries,
and constant blocks, then allocates and registers every built-in shader in its
original order.

The formerly anonymous 35-pointer runtime array now has named slots. Fifteen
graphics shaders are always created, followed by the always-on render-test
shader. When platform slot seven exposes feature bit `0x10`, the manager also
creates the DOF sprite shader and 17 compute resources for blur, depth range,
buffer clearing/copying, DOF, volumetric scattering, SSAO, CMAA, depth
linearization, signed distance, and compute render testing. Slot 16 remains
unused as observed. Each resource retains its exact allocation size from the
executable. Compact built-in shader constructors are tracked separately in
`docs/render-builtin-shaders.md`.

Shader-constant setup at `0x640D60` is source-owned. It creates the `Scene`,
`RenderTarget`, `Camera`, `ClipPlanes`,
`Skeleton`, `MiscDrawState`, `OcclusionQuery`, and `Debug` constant blocks plus
three `Transient` variants containing 16, 32, and 64 vectors. The manager now
exposes typed offsets for every registered constant, including the six-target
`gCameraRTSlicedData` array and the 256-element `gSkeletonBoneXfms` array.

The shared 72-byte constant-block implementation is kept under
`src/render/resources/shaders`. Its member records preserve the HLSL type,
array count, render-target slicing flag, generated register offset, and name.
Scalar, array, and sliced-array insertion at `0x63A1B0`, `0x63A370`, and
`0x63A700` are source-owned, including the original doubling growth policy and
type-width table. Source emission at `0x63A8C0` reproduces both the Metal and
HLSL constant-buffer declarations, including Metal padding fields and HLSL
`packoffset` clauses. The manager hashes the emitted blocks and constant
registry in the original order with 32-bit FNV-1a and stores the result at
manager offset `0x2A8`.

Shader-parameter setup at `0x640BF0` is source-owned. It allocates six 40-byte
registries, enables the first, and registers four manager bindings:
`HX_BT709_TO_BT2020` over `[0, 2)`, `HX_NUM_RT_SLICES` over `[0, 7)`,
`HX_SHADING_MODE` over `[0, 19)`, and `HX_GEO_TYPE` over `[0, 2)`. Geometry
type uses the second registry; the other fields share the first registry's bit
cursor. The binding and registry layouts are shared with primary-shader lazy
preparation under `src/render/resources/shaders`.

Finalization at `0x641370` marks the second manager phase, finalizes every
primary shader resource, and builds the `function_table` texture directly. The
texture is a four-layer 1D array with 128 RGBA32-float samples per layer. Each
layer samples one engine transfer function over the inclusive `[0, 1]` range,
copies it into an owned mip descriptor, and is then consumed by the common
texture-array factory. The four source-owned functions are inverse linear,
inverse-square falloff, normalized negative exponential, and normalized
descending sigmoid curves. Primary-shader finalization is source-owned as
well: it performs the common prepare step, reads shader mode
from virtual slot `0x28`, gates modes zero and one with render-system startup
options, and initializes the backend only when that mode is enabled. The
common prepare path at `0x638270` is source-owned. It lazily allocates the
shader's 32-byte name owner, six 40-byte parameter registries, 72-byte compile
state, and 864-byte backend state. The first parameter registry is enabled,
the others begin disabled, and the compile state records resource kind eight,
the virtual shader variant, and the virtual source identifier. Preparation
then registers the three-bit `HX_NUM_RT_SLICES` field and passes all four
support objects to virtual slot `0x20`. The backend-initialize operation
remains a focused boundary.

The shader constant registry at `0x63F920` is source-owned. It preserves all
13 comment groups and 116 definitions used by generated shader source,
including program and shading modes, debug modes, cube and frustum indices,
lighting paths, and shared structure sizes. Comment records own the engine's
16-byte resource-name type; that common layout now lives under
`src/render/resources/names` and is reused by shader and GPU-stat resources.
Registry growth doubles capacity from one record, matching the original
32-byte record array.

The primary-shader layout and ownership implementation are kept in the
dedicated `src/render/resources/shaders` domain. Registration at `0x638A20`
now inserts each shader into the manager's primary intrusive list directly and
immediately finalizes late registrations after manager phase two. The manager
converts links back to typed shader resources when finalizing or clearing
compiled objects.

Shader reload at `0x641F30` walks both intrusive lists directly. Primary
resources place their manager link at offset `0x110`; each owns six 32-byte
arrays beginning at offset `0x10`. Reload dynamically releases every compiled
object, resets each array's end pointer to its beginning, and clears the byte at
resource offset `0x0C`. When partial-framerate scenes are disabled, the second
list also receives its observed dirty-byte update 395 bytes before each link.
That secondary resource layout remains unnamed beyond this verified relative
offset.

Shutdown at `0x641740` is source-owned. It releases the eight named
shader-constant blocks and three transient blocks, destroys every constant-name
record before freeing its registry, tears down the shader-parameter registry set
containing six 40-byte arrays in reverse order, and invokes
the dynamic release slot for the function-table texture and all 35 resource
slots. Every owning slot is cleared immediately after release. Only the
record-specific name destructor remains a focused adapter boundary because it
installs an unidentified dispatch table and releases its string allocation.
