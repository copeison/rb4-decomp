# Render resource manager

The render system embeds a 712-byte resource-manager block at offset `0x9F0`.
Its constructor at `0x63F180` begins with four fixed 20-byte registries. Each
registry contains two cleared words and a disabled byte at offset `0x10`.

The following handle-state region uses `-1` as its invalid sentinel, with the
observed counters and optional handles initialized to zero. The 336-byte
runtime region begins cleared. It now exposes three sized-array owners, a
32-byte-record name-array owner, a specialized state, the function-table
texture, and 35 dynamic resource slots. The constructor then allocates a 32-byte empty
pointer-array owner and two 16-byte intrusive-list sentinels; each list node is
self-linked when empty.

The destructor at `0x63F350` unlinks and releases the two sentinel nodes, frees
the pointer array's backing storage by its capacity, and releases the array
owner. Runtime initialization remains a typed adapter boundary while its
larger registry algorithm is reconstructed.

Finalization at `0x641370` marks the second manager phase, finalizes every
primary shader resource, and builds the `function_table` texture directly. The
texture is a four-layer 1D array with 128 RGBA32-float samples per layer. Each
layer samples one engine transfer function over the inclusive `[0, 1]` range,
copies it into an owned mip descriptor, and is then consumed by the common
texture-array factory. The four source-owned functions are inverse linear,
inverse-square falloff, normalized negative exponential, and normalized
descending sigmoid curves. Primary-shader finalization remains the focused
boundary.

Shader reload at `0x641F30` walks both intrusive lists directly. Primary
resources place their manager link at offset `0x110`; each owns six 32-byte
arrays beginning at offset `0x10`. Reload dynamically releases every compiled
object, resets each array's end pointer to its beginning, and clears the byte at
resource offset `0x0C`. When partial-framerate scenes are disabled, the second
list also receives its observed dirty-byte update 395 bytes before each link.
That secondary resource layout remains unnamed beyond this verified relative
offset.

Shutdown at `0x641740` is source-owned. It releases eight sized-array owners in
the handle region and three in the runtime region, destroys every name record
before freeing its array owner, tears down the specialized state containing
six 40-byte arrays in reverse order, and invokes
the dynamic release slot for the function-table texture and all 35 resource
slots. Every owning slot is cleared immediately after release. Only the
record-specific name destructor remains a focused adapter boundary because it
installs an unidentified dispatch table and releases its string allocation.
