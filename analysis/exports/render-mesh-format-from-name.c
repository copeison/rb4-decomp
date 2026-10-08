// Reconstructed from eboot.elf at 0x442930.
RenderMeshFormat render_mesh_format_from_name(const char* name) {
    if (strcmp(name, "Color") == 0) return kMeshFormatColor;
    if (strcmp(name, "ColorTex") == 0) return kMeshFormatColorTexture;
    if (strcmp(name, "Unskinned") == 0) return kMeshFormatUnskinned;
    if (strcmp(name, "Skinned") == 0) return kMeshFormatSkinned;
    if (strcmp(name, "PosOnly") == 0) return kMeshFormatPositionOnly;
    if (strcmp(name, "Particle") == 0) return kMeshFormatParticle;
    if (strcmp(name, "UnskinnedCompressed") == 0)
        return kMeshFormatUnskinnedCompressed;
    if (strcmp(name, "SkinnedCompressed") == 0)
        return kMeshFormatSkinnedCompressed;
    return kMeshFormatInvalid;
}
