#pragma once

#include <array>
#include <cstdint>

namespace rb4 {

struct RndTexture1D;
struct RndTexture2D;
struct RndTexture3D;
struct RndTextureArray1D;
struct RndTextureArray2D;
struct RndTextureCube;
struct RndTextureArrayCube;

enum class DefaultTextureKind : std::uint32_t {
    kWhite = 0,
    kBlack = 1,
    kZero = 2,
    kFlatNormal = 3,
    kError = 4,
    kErrorGreyscale = 5,
    kErrorNormal = 6,
};

struct DefaultTextureFamily {
    RndTexture1D* texture_1d = nullptr;
    RndTexture2D* texture_2d = nullptr;
    RndTexture3D* texture_3d = nullptr;
    RndTextureCube* texture_cube = nullptr;
    RndTextureArray1D* texture_array_1d = nullptr;
    RndTextureArray2D* texture_array_2d = nullptr;
    RndTextureArrayCube* texture_array_cube = nullptr;
};

struct DefaultTextureSet {
    std::array<DefaultTextureFamily, 7> families{};
};

RndTexture1D* render_create_default_texture_1d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTexture2D* render_create_default_texture_2d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTexture3D* render_create_default_texture_3d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureCube* render_create_default_texture_cube(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureArray1D* render_create_default_texture_array_1d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureArray2D* render_create_default_texture_array_2d(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);
RndTextureArrayCube* render_create_default_texture_array_cube(
    DefaultTextureKind kind,
    const char* name,
    std::uint32_t extent);

void render_create_default_textures(DefaultTextureSet& textures);

}  // namespace rb4
