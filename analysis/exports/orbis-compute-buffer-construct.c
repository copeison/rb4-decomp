// Constructs common compute-buffer state and clears Orbis backend state.
// Reconstructed from eboot.elf at 0x8E3250.
void orbis_compute_buffer_construct(
    OrbisComputeBuffer* buffer,
    const RenderComputeBufferDescriptor* descriptor) {
    compute_buffer_construct(buffer, descriptor);
    buffer->vtable = &orbis_compute_buffer_vtable;
    clear_orbis_compute_buffer_backend_state(buffer);
}
