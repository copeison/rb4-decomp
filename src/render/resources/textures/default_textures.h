#pragma once

#include <array>
#include <cstdint>

#include "render/textures/RndTextureBase.h"
#include "render/textures/RndTexture1D.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureArray2D.h"
#include "render/textures/RndTextureArrayCube.h"
#include "render/textures/RndTextureCube.h"

namespace rb4 {

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

static_assert(sizeof(DefaultTextureFamily) == 56);
static_assert(sizeof(DefaultTextureSet) == 392);

void render_create_default_textures(DefaultTextureSet& textures);
RndTextureBase* render_get_default_texture(
    const DefaultTextureSet& textures,
    DefaultTextureShape shape,
    DefaultTextureKind kind);

}  // namespace rb4
