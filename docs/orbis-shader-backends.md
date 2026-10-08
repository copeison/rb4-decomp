# Orbis shader backends

Each Orbis shader object uses the common shader lifecycle and supplies virtual
methods for binary initialization, context binding, allocation release, and
stage identification. The common destructor checks the initialized byte at
offset 16, calls the class-specific release method when set, and then clears
that byte.

## Compute shader

Initialization at `0x8E3DC0` parses the shader binary through the Gnmx compute
shader loader. It allocates the code at 256-byte alignment and the copied
compute header at four-byte alignment under the `CShader` allocation name.
The copied header receives the GPU code address split into its 256-byte base
and high address fields. Object offsets 40, 48, and 56 hold the active header,
owned header allocation, and code allocation.

Binding at `0x8E3F40` follows the render context's command mode. Graphics mode
updates the graphics CUE and binds the compute shader through the graphics
context. Standalone compute mode updates the compute CUE, binds through the
compute context, and completes the context transition. Other modes do not
bind. Release at `0x8E4030` defers both owned allocations and clears the active
header pointer.

## Pixel shader

Initialization at `0x8E4480` uses the Gnmx shader parser and creates aligned
`PShader` header and code allocations. It copies both regions, patches the
code address in the header, and stores the active header at offset 40, the
owned header at 48, and code at 56.

Binding at `0x8E4600` refreshes the pixel input-resource table when the header
changes, binds the shader to the active graphics context, and enters pixel
shader mode. Release at `0x8E4670` defers the header and code allocations and
clears the active header pointer.
