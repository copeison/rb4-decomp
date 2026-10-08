# Orbis constant-buffer factory

`orbis_create_constant_buffer` at `0x8D8AD0` allocates one contiguous block
named `cbuffer`. Its size is a 112-byte object header plus 16 bytes for every
requested element.

The constructor receives the original descriptor and flags together with the
element count and a pointer to the inline data region immediately after the
header. This layout keeps the constant-buffer object and its element storage in
a single allocation.
