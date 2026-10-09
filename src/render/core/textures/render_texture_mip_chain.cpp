#include "render/core/textures/render_texture_mip_chain.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/core/textures/render_data_format.h"

namespace rb4 {

namespace {

struct MipChainDispatch {
    void* reserved_destruct;
    void (*release_dynamic)(RenderTextureMipChainState* mip_chain);
};

struct ChannelOrder {
    std::uint32_t count;
    std::int32_t indices[4];
};

constexpr std::int32_t kConstantOne = -1;
constexpr ChannelOrder kChannelOrders[]{
    {2, {0, 1, 0, 0}},
    {2, {1, 0, 0, 0}},
    {3, {0, 1, 2, 0}},
    {3, {2, 1, 0, 0}},
    {4, {0, 1, 2, 3}},
    {4, {0, 1, 2, kConstantOne}},
    {4, {2, 1, 0, 3}},
    {4, {2, 1, 0, kConstantOne}},
    {4, {3, 0, 1, 2}},
    {4, {kConstantOne, 0, 1, 2}},
    {1, {0, 0, 0, 0}},
};

bool channel_order(std::uint32_t layout, ChannelOrder& order) {
    if (layout >= sizeof(kChannelOrders) / sizeof(kChannelOrders[0])) {
        return false;
    }
    order = kChannelOrders[layout];
    return true;
}

float linear_to_srgb(float value) {
    if (value > 0.0031308F) {
        return std::pow(value, 1.0F / 2.4F) * 1.055F - 0.055F;
    }
    return value * 12.92F;
}

RenderFloatPixel convert_to_srgb(const RenderFloatPixel& pixel) {
    return {
        linear_to_srgb(pixel.red),
        linear_to_srgb(pixel.green),
        linear_to_srgb(pixel.blue),
        pixel.alpha,
    };
}

std::uint16_t float_to_half_truncated(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));

    const auto sign = static_cast<std::uint16_t>((bits >> 16) & 0x8000U);
    const auto exponent = (bits >> 23) & 0xFFU;
    const auto mantissa = bits & 0x7FFFFFU;
    if (exponent == 0xFFU) {
        if (mantissa == 0) {
            return static_cast<std::uint16_t>(sign | 0x7C00U);
        }
        auto payload = static_cast<std::uint16_t>(mantissa >> 13);
        if (payload == 0) {
            payload = 1;
        }
        return static_cast<std::uint16_t>(sign | 0x7C00U | payload);
    }

    const auto half_exponent = static_cast<std::int32_t>(exponent) - 112;
    if (half_exponent >= 31) {
        return static_cast<std::uint16_t>(sign | 0x7C00U);
    }
    if (half_exponent <= 0) {
        if (half_exponent <= -11) {
            return sign;
        }
        const auto significand = mantissa | 0x800000U;
        return static_cast<std::uint16_t>(
            sign | (significand >> (14 - half_exponent)));
    }
    return static_cast<std::uint16_t>(
        sign | (static_cast<std::uint32_t>(half_exponent) << 10) |
        (mantissa >> 13));
}

template <typename Value>
void write_value(std::uint8_t*& destination, Value value) {
    std::memcpy(destination, &value, sizeof(value));
    destination += sizeof(value);
}

std::uint32_t pack_unorm(float value, std::uint32_t maximum) {
    if (!(value <= 1.0F)) {
        return maximum;
    }
    if (value <= 0.0F) {
        return 0;
    }
    return static_cast<std::uint32_t>(
        value * static_cast<float>(maximum));
}

bool write_unorm_pixel(
    std::uint8_t*& destination,
    const RenderFloatPixel& pixel,
    const RenderDataFormatDescriptor& format) {
    ChannelOrder order{};
    if (!channel_order(format.channel_layout, order) ||
        format.bit_width % order.count != 0) {
        return false;
    }

    const auto component_width = format.bit_width / order.count;
    if (component_width != 8 && component_width != 16) {
        return false;
    }

    const float channels[]{pixel.red, pixel.green, pixel.blue, pixel.alpha};
    const auto maximum = component_width == 8
        ? static_cast<std::uint32_t>(
              std::numeric_limits<std::uint8_t>::max())
        : static_cast<std::uint32_t>(
              std::numeric_limits<std::uint16_t>::max());
    for (std::uint32_t index = 0; index < order.count; ++index) {
        const auto channel = order.indices[index] == kConstantOne
            ? 1.0F
            : channels[order.indices[index]];
        const auto packed = pack_unorm(channel, maximum);
        if (component_width == 8) {
            write_value(destination, static_cast<std::uint8_t>(packed));
        } else {
            write_value(destination, static_cast<std::uint16_t>(packed));
        }
    }
    return true;
}

bool write_float_pixel(
    std::uint8_t*& destination,
    const RenderFloatPixel& pixel,
    const RenderDataFormatDescriptor& format) {
    ChannelOrder order{};
    if (!channel_order(format.channel_layout, order) ||
        format.bit_width % order.count != 0) {
        return false;
    }

    const auto component_width = format.bit_width / order.count;
    if (component_width != 16 && component_width != 32) {
        return false;
    }

    const float channels[]{pixel.red, pixel.green, pixel.blue, pixel.alpha};
    for (std::uint32_t index = 0; index < order.count; ++index) {
        const auto channel = order.indices[index] == kConstantOne
            ? 1.0F
            : channels[order.indices[index]];
        if (component_width == 16) {
            write_value(destination, float_to_half_truncated(channel));
        } else {
            write_value(destination, channel);
        }
    }
    return true;
}

std::size_t mip_chain_count(const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == nullptr) {
        return 0;
    }
    return static_cast<std::size_t>(mip_chains.end - mip_chains.begin);
}

std::size_t mip_chain_capacity(const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == nullptr) {
        return 0;
    }
    return static_cast<std::size_t>(mip_chains.capacity - mip_chains.begin);
}

std::size_t mip_chain_level_count(const RenderTextureMipChainState& mip_chain) {
    std::size_t count = 0;
    auto* level = &mip_chain;
    while (level != nullptr) {
        ++count;
        level = level->fields.next_mip;
    }
    return count;
}

void release_child(RenderTextureMipChainState*& child) {
    if (child == nullptr) {
        return;
    }
    if (child->fields.implementation != nullptr) {
        auto* dispatch = static_cast<MipChainDispatch*>(
            child->fields.implementation);
        dispatch->release_dynamic(child);
    } else {
        render_texture_mip_chain_destruct(*child);
        ::operator delete(child);
    }
    child = nullptr;
}

void destroy_mip_chain_fields(RenderTextureMipChainFields& fields) {
    delete[] static_cast<std::uint8_t*>(fields.source_data);
    fields.source_data = nullptr;
    fields.source_size = 0;

    release_child(fields.next_mip);
    if (fields.auxiliary_data != nullptr) {
        MemFree(fields.auxiliary_data);
        fields.auxiliary_data = nullptr;
    }

    fields.width = 0;
    fields.height = 0;
    fields.depth = 0;
    fields.data_format = -1;
}

void copy_mip_chain_fields(
    RenderTextureMipChainFields& destination,
    const RenderTextureMipChainFields& source,
    bool has_source_data) {
    destination = {};
    destination.data_format = -1;
    destination.width = source.width;
    destination.height = source.height;
    destination.depth = source.depth;
    destination.data_format = source.data_format;
    destination.source_size = source.source_size;
    std::memcpy(destination.metadata, source.metadata, sizeof(source.metadata));

    if (source.source_data != nullptr) {
        auto* pixels = new std::uint8_t[source.source_size];
        std::memcpy(pixels, source.source_data, source.source_size);
        destination.source_data = pixels;
    }

    if (source.next_mip != nullptr) {
        destination.next_mip = new RenderTextureMipChainState;
        render_texture_mip_chain_construct(
            *destination.next_mip,
            reinterpret_cast<const RenderTextureMipChainDescriptor&>(
                *source.next_mip),
            has_source_data);
    }
}

void destroy_elements(
    RenderTextureMipChainState* begin,
    RenderTextureMipChainState* end) {
    for (auto* mip_chain = begin; mip_chain != end; ++mip_chain) {
        render_texture_mip_chain_destruct(*mip_chain);
    }
}

void release_storage(RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            mip_chains.begin,
            static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(mip_chains.capacity) -
                reinterpret_cast<std::uint8_t*>(mip_chains.begin)));
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x682930.
void render_texture_mip_chain_descriptor_construct(
    RenderTextureMipChainDescriptor& descriptor) {
    descriptor = {};
    descriptor.fields.data_format = -1;
}

// Reconstructed from eboot.elf at 0x682BC0.
void render_texture_mip_chain_descriptor_destruct(
    RenderTextureMipChainDescriptor& descriptor) {
    destroy_mip_chain_fields(descriptor.fields);
}

// Reconstructed from eboot.elf at 0x6830D0.
void render_texture_mip_chain_descriptor_initialize(
    RenderTextureMipChainDescriptor& descriptor,
    const RenderTextureExtent3D& extent,
    std::int32_t data_format) {
    destroy_mip_chain_fields(descriptor.fields);
    descriptor.fields.width = extent.width;
    descriptor.fields.height = extent.height;
    descriptor.fields.depth = extent.depth;
    descriptor.fields.data_format = data_format;
}

// Reconstructed from eboot.elf at 0x682E80.
void render_texture_mip_chain_descriptor_allocate_source(
    RenderTextureMipChainDescriptor& descriptor,
    const RenderTextureExtent3D& extent,
    std::int32_t data_format,
    const void* source_data) {
    auto& fields = descriptor.fields;
    release_child(fields.next_mip);

    fields.width = extent.width;
    fields.height = extent.height;
    fields.depth = extent.depth;
    fields.data_format = data_format;

    const auto bits_per_pixel =
        render_data_format_bits_per_pixel(data_format);
    const auto source_size =
        static_cast<std::size_t>(extent.width) * extent.height * extent.depth *
        bits_per_pixel / 8;
    if (fields.source_data == nullptr || fields.source_size != source_size) {
        delete[] static_cast<std::uint8_t*>(fields.source_data);
        fields.source_data = source_size == 0
            ? nullptr
            : static_cast<void*>(new std::uint8_t[source_size]);
    }
    fields.source_size = source_size;

    if (source_data != nullptr && source_size != 0) {
        std::memcpy(fields.source_data, source_data, source_size);
    }
}

// Reconstructed from eboot.elf at 0x684960, with conversion kernels from
// 0x6897D0 through 0x68CB1B.
bool render_texture_mip_chain_descriptor_copy_float_image(
    RenderTextureMipChainDescriptor& descriptor,
    const RenderFloatImageView& source) {
    const RenderTextureExtent3D extent{
        source.width,
        source.height,
        source.depth,
    };
    render_texture_mip_chain_descriptor_allocate_source(
        descriptor, extent, descriptor.fields.data_format, nullptr);

    const auto format = render_data_format_describe(
        descriptor.fields.data_format);
    if (format.variant != 0) {
        return false;
    }

    const auto pixel_count = static_cast<std::size_t>(source.width) *
        source.height * source.depth;
    if (pixel_count != 0 && source.pixels == nullptr) {
        return false;
    }

    auto* destination = static_cast<std::uint8_t*>(
        descriptor.fields.source_data);
    for (std::size_t index = 0; index < pixel_count; ++index) {
        const auto pixel = format.layout == 2
            ? convert_to_srgb(source.pixels[index])
            : source.pixels[index];
        const auto converted = format.numeric_type == 0
            ? write_unorm_pixel(destination, pixel, format)
            : write_float_pixel(destination, pixel, format);
        if (!converted) {
            return true;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x682960 and 0x6829A0.
void render_texture_mip_chain_construct(
    RenderTextureMipChainState& mip_chain,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data) {
    copy_mip_chain_fields(
        mip_chain.fields, descriptor.fields, has_source_data);
}

void render_texture_mip_chain_destruct(
    RenderTextureMipChainState& mip_chain) {
    destroy_mip_chain_fields(mip_chain.fields);
}

void render_texture_mip_chain_array_construct(
    RenderTextureMipChainArray& mip_chains) {
    mip_chains = {};
}

// Reconstructed from eboot.elf at 0x697470.
void render_texture_mip_chain_array_reserve(
    RenderTextureMipChainArray& mip_chains,
    std::size_t capacity) {
    if (capacity <= mip_chain_capacity(mip_chains)) {
        return;
    }

    auto* replacement = static_cast<RenderTextureMipChainState*>(
        HmxAllocator::gStlAllocator.allocate(capacity * sizeof(RenderTextureMipChainState)));
    auto* output = replacement;
    for (auto* input = mip_chains.begin;
         input != mip_chains.end;
         ++input, ++output) {
        render_texture_mip_chain_construct(
            *output,
            reinterpret_cast<const RenderTextureMipChainDescriptor&>(*input),
            true);
    }

    destroy_elements(mip_chains.begin, mip_chains.end);
    release_storage(mip_chains);
    mip_chains.begin = replacement;
    mip_chains.end = output;
    mip_chains.capacity = replacement + capacity;
}

// Reconstructed from eboot.elf at 0x697890 and its inline caller.
void render_texture_mip_chain_array_append(
    RenderTextureMipChainArray& mip_chains,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data) {
    if (mip_chains.end == mip_chains.capacity) {
        const auto count = mip_chain_count(mip_chains);
        render_texture_mip_chain_array_reserve(
            mip_chains, count == 0 ? 1 : count * 2);
    }

    render_texture_mip_chain_construct(
        *mip_chains.end, descriptor, has_source_data);
    ++mip_chains.end;
}

// Reconstructed from the validation tail at 0x696D8E.
void render_texture_mip_chain_array_validate(
    const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == mip_chains.end) {
        return;
    }

    const auto& first = *mip_chains.begin;
    if (first.fields.height != 1 || first.fields.depth != 1) {
        return;
    }
    const auto expected_levels = mip_chain_level_count(first);
    for (auto* mip_chain = mip_chains.begin + 1;
         mip_chain != mip_chains.end;
         ++mip_chain) {
        if (mip_chain->fields.height != 1 || mip_chain->fields.depth != 1 ||
            mip_chain->fields.width != first.fields.width ||
            mip_chain->fields.data_format != first.fields.data_format ||
            mip_chain_level_count(*mip_chain) != expected_levels) {
            return;
        }
    }
}

std::size_t render_texture_mip_chain_level_count(
    const RenderTextureMipChainState& mip_chain) {
    return mip_chain_level_count(mip_chain);
}

// Reconstructed from eboot.elf at 0x686340.
std::size_t render_texture_mip_chain_source_size(
    const RenderTextureMipChainState& mip_chain) {
    std::size_t size = 0;
    auto* level = &mip_chain;
    while (level != nullptr) {
        size += level->fields.source_size;
        level = level->fields.next_mip;
    }
    return size;
}

// Reconstructed from eboot.elf at 0x683260.
void render_texture_mip_chain_release_source_data(
    RenderTextureMipChainState& mip_chain) {
    auto* level = &mip_chain;
    while (level != nullptr) {
        delete[] static_cast<std::uint8_t*>(level->fields.source_data);
        level->fields.source_data = nullptr;
        level->fields.source_size = 0;
        level = level->fields.next_mip;
    }
}

std::size_t render_texture_mip_chain_array_count(
    const RenderTextureMipChainArray& mip_chains) {
    return mip_chain_count(mip_chains);
}

std::size_t render_texture_mip_chain_array_source_size(
    const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == mip_chains.end) {
        return 0;
    }
    return mip_chain_count(mip_chains) *
        render_texture_mip_chain_source_size(*mip_chains.begin);
}

void render_texture_mip_chain_array_release_source_data(
    RenderTextureMipChainArray& mip_chains) {
    for (auto* mip_chain = mip_chains.begin;
         mip_chain != mip_chains.end;
         ++mip_chain) {
        render_texture_mip_chain_release_source_data(*mip_chain);
    }
}

void render_texture_mip_chain_array_destruct(
    RenderTextureMipChainArray& mip_chains) {
    destroy_elements(mip_chains.begin, mip_chains.end);
    release_storage(mip_chains);
    mip_chains = {};
}

}  // namespace rb4
