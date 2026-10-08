#pragma once

#include <array>
#include <cstdint>

#include "render/core/textures/render_texture.h"
#include "render/core/textures/render_texture_1d.h"
#include "render/core/textures/render_texture_2d.h"
#include "render/core/textures/render_texture_3d.h"
#include "render/core/textures/render_texture_array_1d.h"
#include "render/core/textures/render_texture_array_2d.h"
#include "render/core/textures/render_texture_array_cube.h"
#include "render/core/textures/render_texture_cube.h"

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
    RenderTexture1D* texture_1d = nullptr;
    RenderTexture2D* texture_2d = nullptr;
    RenderTexture3D* texture_3d = nullptr;
    RenderTextureCube* texture_cube = nullptr;
    RenderTextureArray1D* texture_array_1d = nullptr;
    RenderTextureArray2D* texture_array_2d = nullptr;
    RenderTextureArrayCube* texture_array_cube = nullptr;
};

struct DefaultTextureSet {
    std::array<DefaultTextureFamily, 7> families{};
};

static_assert(sizeof(DefaultTextureFamily) == 56);
static_assert(sizeof(DefaultTextureSet) == 392);

void render_create_default_textures(DefaultTextureSet& textures);
RenderTexture* render_get_default_texture(
    const DefaultTextureSet& textures,
    DefaultTextureShape shape,
    DefaultTextureKind kind);

}  // namespace rb4
