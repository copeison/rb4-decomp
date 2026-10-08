#include "render/resources/textures/default_textures.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_data_format_adapters.h"
#include "render/core/textures/render_texture_mip_chain_adapters.h"

namespace rb4 {

namespace {

struct DefaultTextureSpec {
    const char* name;
    std::uint32_t extent;
    std::uint32_t creation_mode;
    RenderFloatPixel primary;
    RenderFloatPixel secondary;
    bool checkerboard;
};

constexpr RenderFloatPixel kTransparentBlack{0.0F, 0.0F, 0.0F, 0.0F};

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
            engine_deallocate_sized(pixels_, byte_count_);
        }
    }

    DefaultPixelBuffer(const DefaultPixelBuffer&) = delete;
    DefaultPixelBuffer& operator=(const DefaultPixelBuffer&) = delete;

    RenderFloatImageView reshape(
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t depth) {
        const auto pixel_count =
            static_cast<std::size_t>(width) * height * depth;
        const auto byte_count = pixel_count * sizeof(RenderFloatPixel);
        if (byte_count != byte_count_) {
            if (pixels_ != nullptr) {
                engine_deallocate_sized(pixels_, byte_count_);
            }
            pixels_ = static_cast<RenderFloatPixel*>(
                engine_allocate_sized(byte_count));
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
            width,
            height,
            depth,
            0,
            pixels_,
            nullptr,
        };
    }

private:
    const DefaultTextureSpec& spec_;
    RenderFloatPixel* pixels_ = nullptr;
    std::size_t byte_count_ = 0;
};

void initialize_texture_state(
    RenderTextureDescriptorState& state,
    std::int32_t descriptor_type,
    const DefaultTextureSpec& spec) {
    render_texture_descriptor_construct(state);
    state.descriptor_type = descriptor_type;
    state.creation_state.values[0] = spec.creation_mode;
    state.creation_state.values[8] = 2;
    state.creation_state.values[9] = 1;
    state.name = spec.name;
}

void initialize_mip(
    RenderTextureMipChainDescriptor& mip,
    const RenderFloatImageView& image,
    std::int32_t data_format) {
    render_texture_mip_chain_descriptor_construct(mip);
    const RenderTextureExtent3D extent{
        image.width,
        image.height,
        image.depth,
    };
    render_texture_mip_chain_descriptor_allocate_source(
        mip, extent, data_format, nullptr);
    render_texture_mip_chain_descriptor_copy_float_image(mip, image);
}

void destruct_mip(RenderTextureMipChainDescriptor& mip) {
    render_texture_mip_chain_descriptor_destruct(mip);
}

void initialize_cube_mips(
    RenderTextureCubeDescriptorState& cube,
    const RenderFloatImageView& image,
    std::int32_t data_format) {
    for (auto& face : cube.faces) {
        initialize_mip(face, image, data_format);
    }
}

void destruct_cube_mips(RenderTextureCubeDescriptorState& cube) {
    for (auto& face : cube.faces) {
        destruct_mip(face);
    }
}

void create_texture_family(
    DefaultTextureFamily& family,
    const DefaultTextureSpec& spec,
    std::int32_t data_format) {
    DefaultPixelBuffer pixels(spec);

    auto image = pixels.reshape(spec.extent, 1, 1);
    RenderTextureMipChainDescriptor mip_1d;
    initialize_mip(mip_1d, image, data_format);

    RenderTexture1DDescriptor texture_1d{};
    initialize_texture_state(texture_1d.texture_state, 0, spec);
    texture_1d.mip_chain = mip_1d;
    family.texture_1d = render_create_texture_1d(texture_1d, nullptr);

    RenderTextureArray1DDescriptor array_1d{};
    initialize_texture_state(array_1d.texture_state, 4, spec);
    array_1d.mip_chains = {&mip_1d, &mip_1d + 1, &mip_1d + 1};
    family.texture_array_1d =
        render_create_texture_array_1d(array_1d, nullptr);

    image = pixels.reshape(spec.extent, spec.extent, 1);
    RenderTextureMipChainDescriptor mip_2d;
    initialize_mip(mip_2d, image, data_format);

    RenderTexture2DDescriptor texture_2d{};
    initialize_texture_state(texture_2d.texture_state, 1, spec);
    texture_2d.mip_chain = mip_2d;
    family.texture_2d = render_create_texture_2d(texture_2d);

    RenderTextureArray2DDescriptor array_2d{};
    initialize_texture_state(array_2d.texture_state, 5, spec);
    array_2d.mip_chains = {&mip_2d, &mip_2d + 1, &mip_2d + 1};
    family.texture_array_2d = render_create_texture_array_2d(array_2d);

    RenderTextureCubeDescriptor cube{};
    initialize_texture_state(cube.texture_state, 3, spec);
    initialize_cube_mips(cube.cube, image, data_format);
    family.texture_cube = render_create_texture_cube(cube, nullptr);

    RenderTextureCubeDescriptorState array_cube_state{};
    initialize_cube_mips(array_cube_state, image, data_format);
    RenderTextureArrayCubeDescriptor array_cube{};
    initialize_texture_state(array_cube.texture_state, 7, spec);
    array_cube.cubes = {
        &array_cube_state,
        &array_cube_state + 1,
        &array_cube_state + 1,
    };
    family.texture_array_cube =
        render_create_texture_array_cube(array_cube, nullptr);

    image = pixels.reshape(spec.extent, spec.extent, spec.extent);
    RenderTextureMipChainDescriptor mip_3d;
    initialize_mip(mip_3d, image, data_format);
    RenderTexture3DDescriptor texture_3d{};
    initialize_texture_state(texture_3d.texture_state, 2, spec);
    texture_3d.mip_chain = mip_3d;
    family.texture_3d = render_create_texture_3d(texture_3d, nullptr);

    destruct_mip(mip_3d);
    destruct_cube_mips(array_cube_state);
    destruct_cube_mips(cube.cube);
    destruct_mip(mip_2d);
    destruct_mip(mip_1d);
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
RenderTexture* render_get_default_texture(
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
