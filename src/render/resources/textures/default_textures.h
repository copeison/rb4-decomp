#pragma once

#include <array>
#include <cstdint>

namespace rb4 {

struct RndTextureResource;

enum class DefaultTextureKind : std::uint32_t {
    kWhite = 0,
    kBlack = 1,
    kZero = 2,
    kFlatNormal = 3,
    kError = 4,
    kErrorGreyscale = 5,
    kErrorNormal = 6,
};

enum class DefaultTextureShape : std::uint32_t {
    kTexture1D = 0,
    kTexture2D = 1,
    kTexture3D = 2,
    kTextureCube = 3,
    kTextureArray1D = 4,
    kTextureArray2D = 5,
    kTextureArrayCube = 7,
};

struct DefaultTextureFamily {
    RndTextureResource* texture_1d = nullptr;
    RndTextureResource* texture_2d = nullptr;
    RndTextureResource* texture_3d = nullptr;
    RndTextureResource* texture_cube = nullptr;
    RndTextureResource* texture_array_1d = nullptr;
    RndTextureResource* texture_array_2d = nullptr;
    RndTextureResource* texture_array_cube = nullptr;
};

struct DefaultTextureSet {
    std::array<DefaultTextureFamily, 7> families{};
};

RndTextureResource* render_create_default_texture_1d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureResource* render_create_default_texture_2d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureResource* render_create_default_texture_3d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureResource* render_create_default_texture_cube(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureResource* render_create_default_texture_array_1d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureResource* render_create_default_texture_array_2d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureResource* render_create_default_texture_array_cube(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
void rnd_texture_resource_release(RndTextureResource* texture);

void render_create_default_textures(DefaultTextureSet& textures);
RndTextureResource* render_get_default_texture(
    const DefaultTextureSet& textures,
    DefaultTextureShape shape,
    DefaultTextureKind kind);

}  // namespace rb4
