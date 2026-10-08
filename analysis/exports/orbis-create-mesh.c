// Allocates a format-specific 472-byte Orbis mesh.
// Reconstructed from eboot.elf at 0x8D85F0.
OrbisMesh* orbis_create_mesh(RenderMeshFormat format, const char* name) {
    if (format == kMeshFormatParticle || format > kMeshFormatSkinnedCompressed)
        return 0;

    OrbisMesh* mesh = render_allocate(472);
    mesh_construct(mesh, name);
    install_orbis_mesh_format_vtables(mesh, format);
    clear_orbis_mesh_backend_state(mesh);
    return mesh;
}
