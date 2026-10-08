// Allocates and constructs a 136-byte Orbis compute buffer.
// Reconstructed from eboot.elf at 0x8D8BC0.
OrbisComputeBuffer* orbis_create_compute_buffer(
    const RenderComputeBufferDescriptor* descriptor) {
    OrbisComputeBuffer* buffer = render_allocate(136);
    orbis_compute_buffer_construct(buffer, descriptor);
    return buffer;
}
