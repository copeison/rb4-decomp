# Orbis constant buffer

`orbis_create_constant_buffer` at `0x8D8AD0` allocates one contiguous block
named `cbuffer`. Its size is a 112-byte object header plus 16 bytes for every
requested element.

The constructor receives the original descriptor and flags together with the
element count and a pointer to the inline data region immediately after the
header. This layout keeps the constant-buffer object and its element storage in
a single allocation.

The common base at `0x639FF0` is exactly 64 bytes. It preserves the descriptor
owner, binding slot, shader-stage mask, descriptor-default element count,
selected element count, CPU data pointer, and pending-upload state. The generic
factory at `0x639F30` substitutes the descriptor's element count when its caller
passes `size_t(-1)`. Unless flag bit `0x01` requests deferred initialization,
the factory immediately invokes the platform initializer and clears the
pending-upload byte.

The platform constructor at `0x8E3800` clears 32 bytes of backend state at
object offset `+80`. Initialization at `0x8E3900` allocates a persistent GPU
copy named `CBuffer`. Its size is `16 * element_count`, with a 16-byte minimum
for an empty logical buffer, and the initial inline CPU data is copied into it.
The range update method at `0x8E3980` copies `(end - first) * 16` bytes into the
persistent allocation at `first * 16` and invalidates the cached frame-local
address.

Binding at `0x8E39C0` (`PS4ShaderCBuffer::_SelectImpl`) compares the buffer's
frame stamp with `RndDevice::mFrameCount`. On the first bind in a frame it allocates suitably sized embedded
command memory from the active `GfxContext` or `ComputeContext` (chosen by
`RndContext::mActivePipe`), copies
the persistent GPU data into that memory, builds a constant-buffer descriptor
(`Buffer::initAsConstantBuffer`), and sets `kResourceMemoryTypeRO`. Later binds in the same frame reuse the cached
embedded address.

The common descriptor supplies the binding slot at object offset `+20` and a
stage mask at `+24`. Its bits route the descriptor as follows:

| Mask | Shader stage |
| ---: | --- |
| `0x01` | Vertex |
| `0x02` | Hull and domain (the HS and LS stages) |
| `0x04` | Geometry |
| `0x08` | Pixel |
| `0x10` | Compute, using either the graphics CUE or standalone compute context |

The destructor at `0x8E3830` and release helper at `0x8E3880` defer retirement
of the persistent allocation and clear its recorded byte size. The deleting
destructor at `0x8E38C0` then frees the single contiguous object allocation.

IDA evidence is preserved in
`analysis/exports/orbis-constant-buffer-backend.asm` and
`analysis/exports/orbis-constant-buffer-backend.c`.
