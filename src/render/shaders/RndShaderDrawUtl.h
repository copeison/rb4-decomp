#pragma once

class RndContext;
class RndShader;
class RndShaderCBuffer;
class RndTextureBase;

// The patterns that the built-in shaders' Select methods inline: binding a
// pixel texture and filling one of the context's per-draw constant buffers.
// The binary has no out-of-line copy, so the map names neither the class nor
// its methods. Name not in the reference map.
class RndShaderDrawUtl {
public:
    // Selects the texture for the pixel stage at the slot; null textures are
    // skipped.
    static void SelectPixelTexture(
        RndContext& context,
        RndTextureBase* texture,
        unsigned long slot,
        unsigned int flags = 0);

    // The context's smallest per-draw constant buffer that holds the given
    // number of 16-byte elements.
    static RndShaderCBuffer& GetCBuffer(
        RndContext& context,
        unsigned long elementCount);

    // Address of a constant-block member inside a constant buffer's staging
    // data.
    static void* GetCBufferMember(
        RndShaderCBuffer& buffer,
        unsigned long memberOffset);

    // Uploads the first elementCount elements and selects the buffer.
    static void CommitCBuffer(
        RndShaderCBuffer& buffer,
        RndContext& context,
        unsigned long elementCount);

    // Draw used by single-texture shaders: selects the texture for the pixel
    // stage at the slot and the shader with default keys.
    static void SelectWithPixelTexture(
        RndShader& shader,
        RndContext& context,
        RndTextureBase& texture,
        unsigned long slot);
};
