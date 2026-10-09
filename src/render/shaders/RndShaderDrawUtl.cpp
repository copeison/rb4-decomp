#include "render/shaders/RndShaderDrawUtl.h"

#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShader.h"
#include "render/shaders/RndShaderProgram.h"
#include "render/textures/RndTextureBase.h"

namespace {

constexpr unsigned long kSmallestCBufferElements = 16;
constexpr unsigned long kCBufferElementSize = 16;
// The per-draw constant buffers follow the global ones in RndContext.
constexpr unsigned long kFirstDrawCBuffer = 6;

}  // namespace

void RndShaderDrawUtl::SelectPixelTexture(
    RndContext& context,
    RndTextureBase* texture,
    unsigned long slot,
    unsigned int flags) {
    if (texture != nullptr) {
        texture->Select(context, kShaderProgramPixel, slot, flags, 0);
    }
}

RndShaderCBuffer& RndShaderDrawUtl::GetCBuffer(
    RndContext& context,
    unsigned long elementCount) {
    unsigned long sizeClass = 0;
    if (elementCount > kSmallestCBufferElements) {
        auto capacity = kSmallestCBufferElements;
        do {
            capacity *= 2;
            ++sizeClass;
        } while (capacity < elementCount);
    }
    return *context.mCBuffers[kFirstDrawCBuffer + sizeClass];
}

void* RndShaderDrawUtl::GetCBufferMember(
    RndShaderCBuffer& buffer,
    unsigned long memberOffset) {
    return static_cast<unsigned char*>(buffer.mData) +
        memberOffset * kCBufferElementSize;
}

void RndShaderDrawUtl::CommitCBuffer(
    RndShaderCBuffer& buffer,
    RndContext& context,
    unsigned long elementCount) {
    buffer.mSyncPending = true;
    buffer._SyncImpl(context, 0, elementCount);
    buffer.mSyncPending = false;
    buffer._SelectImpl(context);
}

void RndShaderDrawUtl::SelectWithPixelTexture(
    RndShader& shader,
    RndContext& context,
    RndTextureBase& texture,
    unsigned long slot) {
    SelectPixelTexture(context, &texture, slot);
    RndShaderKeyGroup keys{};
    shader._SelectShaderCollection(context, keys);
}
