# Common render texture

`render_texture_construct` at `0x69B6E0` initializes the 168-byte base shared
by every recovered texture shape. The object starts with virtual dispatch and
a frame stamp of `-1`. Its descriptor-backed state includes sampler address and
filter modes at offsets `+96` and `+100`, flags at `+104`, a format sentinel at
`+108`, dimensions at `+112` through `+120`, an allocation name at `+152`, and
two resource indices initialized to `-1`. The texture-usage field at `+64` is
read directly by Orbis color-versus-depth initialization paths.

The constructor clears the observed descriptor, dimension, source, and runtime
state while leaving compiler padding untouched. Names for fields that have not
yet been tied to an original symbol are descriptive; offsets and defaults come
directly from the constructor. Orbis shader-binding methods independently
confirm the sampler offsets, and the Orbis texture allocation paths use the
name field directly.

The common destructor at `0x69B770` is empty. The deleting destructor at
`0x69B780` invokes that base teardown and frees the object. Its missing IDA
function boundary has been restored.

IDA evidence is preserved in `analysis/exports/render-texture.asm` and
`analysis/exports/render-texture.c`.
