#include "render/resources/textures/default_textures.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/core/textures/render_data_format.h"
#include "math/color/Color.h"
#include "math/vector/Vector3i.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelDataCube.h"

namespace rb4 {

namespace {

struct DefaultTextureSpec {
    const char* name;
    std::uint32_t extent;
    std::uint32_t creation_mode;
    Hmx::Color primary;
    Hmx::Color secondary;
    bool checkerboard;
};

constexpr Hmx::Color kTransparentBlack{0.0F, 0.0F, 0.0F, 0.0F};

constexpr std::array<DefaultTextureSpec, 7> kDefaultTextureSpecs{{
    {"White", 8, 0, {1.0F, 1.0F, 1.0F, 1.0F}, kTransparentBlack, false},
    {"Black", 8, 0, {0.0F, 0.0F, 0.0F, 1.0F}, kTransparentBlack, false},
    {"Zero", 8, 0, kTransparentBlack, kTransparentBlack, false},
    {"Flat Normal", 8, 3, {0.5F, 0.5F, 1.0F, 1.0F}, kTransparentBlack, false},
    {"Error", 64, 0, {1.0F, 0.5F, 0.0F, 1.0F}, {0.0F, 1.0F, 1.0F, 1.0F}, true},
    {"Error Greyscale", 64, 0, {0.25F, 0.25F, 0.25F, 1.0F}, {0.75F, 0.75F, 0.75F, 1.0F}, true},
    {"Error Normal", 64, 3, {1.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 1.0F, 0.0F, 1.0F}, true},
}};

constexpr RenderDataFormatDescriptor kDefaultTextureFormat{
    32,
    4,
    0,
    2,
    -1,
};

class DefaultPixelBuffer {
public:
    explicit DefaultPixelBuffer(const DefaultTextureSpec& spec)
        : spec_(spec) {
    }

    ~DefaultPixelBuffer() {
        if (pixels_ != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(pixels_, byte_count_);
        }
    }

    DefaultPixelBuffer(const DefaultPixelBuffer&) = delete;
    DefaultPixelBuffer& operator=(const DefaultPixelBuffer&) = delete;

    RndPixelCanvas reshape(
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t depth) {
        const auto pixel_count =
            static_cast<std::size_t>(width) * height * depth;
        const auto byte_count = pixel_count * sizeof(Hmx::Color);
        if (byte_count != byte_count_) {
            if (pixels_ != nullptr) {
                HmxAllocator::gStlAllocator.deallocate(pixels_, byte_count_);
            }
            pixels_ = static_cast<Hmx::Color*>(
                HmxAllocator::gStlAllocator.allocate(byte_count));
            byte_count_ = byte_count;
        }

        for (std::uint32_t z = 0; z < depth; ++z) {
            for (std::uint32_t y = 0; y < height; ++y) {
                for (std::uint32_t x = 0; x < width; ++x) {
                    const auto checker_index =
                        (x / 8U) ^ (y / 8U) ^ (z / 8U);
                    const auto use_primary =
                        !spec_.checkerboard || (checker_index & 1U) != 0;
                    const auto pixel_index =
                        x + width * (y + height * z);
                    pixels_[pixel_index] = use_primary
                        ? spec_.primary
                        : spec_.secondary;
                }
            }
        }

        return {
            nullptr,
            static_cast<int>(width),
            static_cast<int>(height),
            static_cast<int>(depth),
            0,
            pixels_,
            nullptr,
        };
    }

private:
    const DefaultTextureSpec& spec_;
    Hmx::Color* pixels_ = nullptr;
    std::size_t byte_count_ = 0;
};

void configure_texture_state(
    RndTextureBase::Description& state,
    const DefaultTextureSpec& spec) {
    state.mRequestedFormat.mUsage = static_cast<int>(spec.creation_mode);
    state.mRequestedFormat.mWrapMode = 2;
    state.mRequestedFormat.mFilterMode = 1;
    state.mName = spec.name;
}

void populate_mip(
    RndPixelData& mip,
    const RndPixelCanvas& image,
    std::int32_t data_format) {
    const Vector3i extent{image.mWidth, image.mHeight, image.mDepth};
    mip.Create(extent, data_format, nullptr);
    mip.ConvertFrom(image);
}

void populate_cube_mips(
    RndPixelDataCube& cube,
    const RndPixelCanvas& image,
    std::int32_t data_format) {
    for (auto& face : cube.mFaces) {
        populate_mip(face, image, data_format);
    }
}

void create_texture_family(
    DefaultTextureFamily& family,
    const DefaultTextureSpec& spec,
    std::int32_t data_format) {
    DefaultPixelBuffer pixels(spec);

    auto image = pixels.reshape(spec.extent, 1, 1);
    RndTexture1D::Description texture_1d;
    configure_texture_state(texture_1d, spec);
    populate_mip(texture_1d.mPixels, image, data_format);
    family.texture_1d = RndTexture1D::New(texture_1d, nullptr);

    RndTextureArray1D::Description array_1d;
    configure_texture_state(array_1d, spec);
    array_1d.mPixels = {
        &texture_1d.mPixels,
        &texture_1d.mPixels + 1,
        &texture_1d.mPixels + 1,
    };
    family.texture_array_1d =
        RndTextureArray1D::New(array_1d, nullptr);

    image = pixels.reshape(spec.extent, spec.extent, 1);
    RndTexture2D::Description texture_2d;
    configure_texture_state(texture_2d, spec);
    populate_mip(texture_2d.mPixels, image, data_format);
    family.texture_2d = RndTexture2D::New(texture_2d);

    RndTextureArray2D::Description array_2d;
    configure_texture_state(array_2d, spec);
    array_2d.mPixels = {
        &texture_2d.mPixels,
        &texture_2d.mPixels + 1,
        &texture_2d.mPixels + 1,
    };
    family.texture_array_2d = RndTextureArray2D::New(array_2d);

    RndTextureCube::Description cube;
    configure_texture_state(cube, spec);
    populate_cube_mips(cube.mCube, image, data_format);
    family.texture_cube = RndTextureCube::New(cube, nullptr);

    RndPixelDataCube array_cube_state;
    populate_cube_mips(array_cube_state, image, data_format);
    RndTextureArrayCube::Description array_cube;
    configure_texture_state(array_cube, spec);
    array_cube.mCubes = {
        &array_cube_state,
        &array_cube_state + 1,
        &array_cube_state + 1,
    };
    family.texture_array_cube =
        RndTextureArrayCube::New(array_cube, nullptr);

    image = pixels.reshape(spec.extent, spec.extent, spec.extent);
    RndTexture3D::Description texture_3d;
    configure_texture_state(texture_3d, spec);
    populate_mip(texture_3d.mPixels, image, data_format);
    family.texture_3d = RndTexture3D::New(texture_3d, nullptr);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6BDE60.
void render_create_default_textures(DefaultTextureSet& textures) {
    const auto data_format =
        render_data_format_resolve(kDefaultTextureFormat, 7);
    for (std::size_t index = 0; index < kDefaultTextureSpecs.size(); ++index) {
        create_texture_family(
            textures.families[index],
            kDefaultTextureSpecs[index],
            data_format);
    }
}

// Reconstructed from eboot.elf at 0x6C00F0.
RndTextureBase* render_get_default_texture(
    const DefaultTextureSet& textures,
    DefaultTextureShape shape,
    DefaultTextureKind kind) {
    const auto index = static_cast<std::size_t>(kind);
    if (index >= textures.families.size()) {
        return nullptr;
    }

    const auto& family = textures.families[index];
    switch (shape) {
    case DefaultTextureShape::kTexture1D:
        return family.texture_1d;
    case DefaultTextureShape::kTexture2D:
        return family.texture_2d;
    case DefaultTextureShape::kTexture3D:
        return family.texture_3d;
    case DefaultTextureShape::kTextureCube:
        return family.texture_cube;
    case DefaultTextureShape::kTextureArray1D:
        return family.texture_array_1d;
    case DefaultTextureShape::kTextureArray2D:
        return family.texture_array_2d;
    case DefaultTextureShape::kTextureArrayCube:
        return family.texture_array_cube;
    }
    return nullptr;
}

}  // namespace rb4
