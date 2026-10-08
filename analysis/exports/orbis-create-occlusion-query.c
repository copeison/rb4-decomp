// Allocates and constructs a 72-byte Orbis occlusion query.
// Reconstructed from eboot.elf at 0x8D8C30.
OrbisOcclusionQuery* orbis_create_occlusion_query(void* owner) {
    OrbisOcclusionQuery* query = render_allocate(72);
    orbis_occlusion_query_construct(query, owner);
    return query;
}
