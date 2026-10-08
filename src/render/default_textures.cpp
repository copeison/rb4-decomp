#include "default_textures.h"

namespace rb4 {

namespace {

struct DefaultTextureSpec {
    DefaultTextureKind kind;
    const char* name;
    std::uint32_t extent;
};

constexpr std::array<DefaultTextureSpec, 7> kDefaultTextureSpecs{{
    {DefaultTextureKind::kWhite, "White", 8},
    {DefaultTextureKind::kBlack, "Black", 8},
    {DefaultTextureKind::kZero, "Zero", 8},
    {DefaultTextureKind::kFlatNormal, "Flat Normal", 8},
    {DefaultTextureKind::kError, "Error", 64},
    {DefaultTextureKind::kErrorGreyscale, "Error Greyscale", 64},
    {DefaultTextureKind::kErrorNormal, "Error Normal", 64},
}};

void create_texture_family(
    DefaultTextureFamily& family,
    const DefaultTextureSpec& spec) {
    family.texture_1d =
        render_create_default_texture_1d(spec.kind, spec.name, spec.extent);
    family.texture_2d =
        render_create_default_texture_2d(spec.kind, spec.name, spec.extent);
    family.texture_3d =
        render_create_default_texture_3d(spec.kind, spec.name, spec.extent);
    family.texture_cube =
        render_create_default_texture_cube(spec.kind, spec.name, spec.extent);
    family.texture_array_1d = render_create_default_texture_array_1d(
        spec.kind, spec.name, spec.extent);
    family.texture_array_2d = render_create_default_texture_array_2d(
        spec.kind, spec.name, spec.extent);
    family.texture_array_cube = render_create_default_texture_array_cube(
        spec.kind, spec.name, spec.extent);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6BDE60.
void render_create_default_textures(DefaultTextureSet& textures) {
    for (std::size_t index = 0; index < kDefaultTextureSpecs.size(); ++index) {
        create_texture_family(
            textures.families[index], kDefaultTextureSpecs[index]);
    }
}

}  // namespace rb4
