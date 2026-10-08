# Render resource manager

The render system embeds a 712-byte resource-manager block at offset `0x9F0`.
Its constructor at `0x63F180` begins with four fixed 20-byte registries. Each
registry contains two cleared words and a disabled byte at offset `0x10`.

The following handle-state region uses `-1` as its invalid sentinel, with the
observed counters and optional handles initialized to zero. The 336-byte
runtime region begins cleared. The constructor then allocates a 32-byte empty
pointer-array owner and two 16-byte intrusive-list sentinels; each list node is
self-linked when empty.

The destructor at `0x63F350` unlinks and releases the two sentinel nodes, frees
the pointer array's backing storage by its capacity, and releases the array
owner. Runtime initialization, finalization, and shutdown remain typed adapter
boundaries while their larger registry algorithms are reconstructed.

Shader reload at `0x641F30` walks both intrusive lists directly. Primary
resources place their manager link at offset `0x110`; each owns six 32-byte
arrays beginning at offset `0x10`. Reload dynamically releases every compiled
object, resets each array's end pointer to its beginning, and clears the byte at
resource offset `0x0C`. When partial-framerate scenes are disabled, the second
list also receives its observed dirty-byte update 395 bytes before each link.
That secondary resource layout remains unnamed beyond this verified relative
offset.
