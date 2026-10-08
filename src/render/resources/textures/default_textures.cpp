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

void configure_texture_state(
    RenderTextureDescriptorState& state,
    const DefaultTextureSpec& spec) {
    state.creation_state.values[0] = spec.creation_mode;
    state.creation_state.values[8] = 2;
    state.creation_state.values[9] = 1;
    state.name = spec.name;
}

void populate_mip(
    RenderTextureMipChainDescriptor& mip,
    const RenderFloatImageView& image,
    std::int32_t data_format) {
    const RenderTextureExtent3D extent{
        image.width,
        image.height,
        image.depth,
    };
    render_texture_mip_chain_descriptor_allocate_source(
        mip, extent, data_format, nullptr);
    render_texture_mip_chain_descriptor_copy_float_image(mip, image);
}

void initialize_mip(
    RenderTextureMipChainDescriptor& mip,
    const RenderFloatImageView& image,
    std::int32_t data_format) {
    render_texture_mip_chain_descriptor_construct(mip);
    populate_mip(mip, image, data_format);
}

void destruct_mip(RenderTextureMipChainDescriptor& mip) {
    render_texture_mip_chain_descriptor_destruct(mip);
}

void populate_cube_mips(
    RenderTextureCubeDescriptorState& cube,
    const RenderFloatImageView& image,
    std::int32_t data_format) {
    for (auto& face : cube.faces) {
        populate_mip(face, image, data_format);
    }
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
    RenderTexture1DDescriptor texture_1d;
    render_texture_1d_descriptor_construct(texture_1d);
    configure_texture_state(texture_1d.texture_state, spec);
    populate_mip(texture_1d.mip_chain, image, data_format);
    family.texture_1d = render_create_texture_1d(texture_1d, nullptr);

    RenderTextureArray1DDescriptor array_1d;
    render_texture_array_1d_descriptor_construct(array_1d);
    configure_texture_state(array_1d.texture_state, spec);
    array_1d.mip_chains = {
        &texture_1d.mip_chain,
        &texture_1d.mip_chain + 1,
        &texture_1d.mip_chain + 1,
    };
    family.texture_array_1d =
        render_create_texture_array_1d(array_1d, nullptr);

    image = pixels.reshape(spec.extent, spec.extent, 1);
    RenderTexture2DDescriptor texture_2d;
    render_texture_2d_descriptor_construct(texture_2d);
    configure_texture_state(texture_2d.texture_state, spec);
    populate_mip(texture_2d.mip_chain, image, data_format);
    family.texture_2d = render_create_texture_2d(texture_2d);

    RenderTextureArray2DDescriptor array_2d;
    render_texture_array_2d_descriptor_construct(array_2d);
    configure_texture_state(array_2d.texture_state, spec);
    array_2d.mip_chains = {
        &texture_2d.mip_chain,
        &texture_2d.mip_chain + 1,
        &texture_2d.mip_chain + 1,
    };
    family.texture_array_2d = render_create_texture_array_2d(array_2d);

    RenderTextureCubeDescriptor cube;
    render_texture_cube_descriptor_construct(cube);
    configure_texture_state(cube.texture_state, spec);
    populate_cube_mips(cube.cube, image, data_format);
    family.texture_cube = render_create_texture_cube(cube, nullptr);

    RenderTextureCubeDescriptorState array_cube_state{};
    initialize_cube_mips(array_cube_state, image, data_format);
    RenderTextureArrayCubeDescriptor array_cube;
    render_texture_array_cube_descriptor_construct(array_cube);
    configure_texture_state(array_cube.texture_state, spec);
    array_cube.cubes = {
        &array_cube_state,
        &array_cube_state + 1,
        &array_cube_state + 1,
    };
    family.texture_array_cube =
        render_create_texture_array_cube(array_cube, nullptr);

    image = pixels.reshape(spec.extent, spec.extent, spec.extent);
    RenderTexture3DDescriptor texture_3d;
    render_texture_3d_descriptor_construct(texture_3d);
    configure_texture_state(texture_3d.texture_state, spec);
    populate_mip(texture_3d.mip_chain, image, data_format);
    family.texture_3d = render_create_texture_3d(texture_3d, nullptr);

    destruct_mip(texture_3d.mip_chain);
    destruct_cube_mips(array_cube_state);
    destruct_cube_mips(cube.cube);
    destruct_mip(texture_2d.mip_chain);
    destruct_mip(texture_1d.mip_chain);
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
