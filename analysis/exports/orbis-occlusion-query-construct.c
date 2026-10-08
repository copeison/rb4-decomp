// Constructs common occlusion-query state and clears its backend address.
// Reconstructed from eboot.elf at 0x8E28C0.
void orbis_occlusion_query_construct(
    OrbisOcclusionQuery* query,
    void* owner) {
    occlusion_query_construct(query, owner);
    query->vtable = &orbis_occlusion_query_vtable;
    query->backend_address = 0;
}
