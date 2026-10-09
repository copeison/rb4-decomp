#include "render/core/targets/render_target_resource_factory.h"

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/system/RndFactory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resources.h"
#include "render/textures/RndTextureBase.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureArray2D.h"

namespace rb4 {

namespace {

// The description arrives freshly constructed.
void initialize_texture_descriptor(
    RndTextureBase::Description& state,
    std::int32_t descriptor_type,
    const RndPixelFormat& creation_state,
    const char* name,
    std::int32_t attachment_index,
    std::uint32_t target_flags) {
    state.mType = descriptor_type;
    state.mRequestedFormat = creation_state;
    state.mTargetFlags = target_flags;
    state.mAttachmentIndex = attachment_index;
    state.mName = name;
}

void initialize_mip_descriptor(
    RndPixelData& mip,
    RenderExtent extent,
    std::int32_t data_format) {
    mip.mSize.x = static_cast<int>(extent.width);
    mip.mSize.y = static_cast<int>(extent.height);
    mip.mSize.z = 1;
    mip.mFormat = data_format;
}

void resolve_descriptor(
    RndTextureBase::Description& state,
    std::int32_t descriptor_type) {
    state.ResolveFormat(descriptor_type, -1);
}

RndFactory& render_factory() {
    return *render_system_factory(*render_system_instance());
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B4120.
RndTextureBase* render_target_resources_create_texture_2d(
    RenderTargetResources&,
    const char* name,
    const RndPixelFormat& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RndTextureBase* reusable_texture) {
    RndTexture2D::Description descriptor;
    initialize_texture_descriptor(
        descriptor,
        1,
        creation_state,
        name,
        attachment_index,
        target_flags);
    initialize_mip_descriptor(
        descriptor.mPixels, extent, data_format);
    resolve_descriptor(descriptor, 1);

    auto* texture = render_factory().CreateTexture2D(descriptor);
    texture->SyncStatic(reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x6B41F0.
RndTextureBase* render_target_resources_create_texture_array_2d(
    RenderTargetResources&,
    const char* name,
    const RndPixelFormat& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::size_t layer_count,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RndTextureBase* reusable_texture) {
    RndTextureArray2D::Description descriptor;
    initialize_texture_descriptor(
        descriptor,
        5,
        creation_state,
        name,
        attachment_index,
        target_flags);

    auto* mip_chains = static_cast<RndPixelData*>(
        HmxAllocator::gStlAllocator.allocate(
            layer_count * sizeof(RndPixelData)));
    for (std::size_t index = 0; index < layer_count; ++index) {
        auto* layer = new (&mip_chains[index]) RndPixelData;
        initialize_mip_descriptor(*layer, extent, data_format);
    }
    descriptor.mPixels = {
        mip_chains,
        mip_chains + layer_count,
        mip_chains + layer_count,
    };
    resolve_descriptor(descriptor, 5);

    auto* texture = render_factory().CreateTextureArray2D(descriptor);
    texture->SyncStatic(reusable_texture);
    if (mip_chains != nullptr) {
        for (std::size_t index = 0; index < layer_count; ++index) {
            mip_chains[index].~RndPixelData();
        }
        HmxAllocator::gStlAllocator.deallocate(
            mip_chains,
            layer_count * sizeof(RndPixelData));
    }
    return texture;
}

}  // namespace rb4
