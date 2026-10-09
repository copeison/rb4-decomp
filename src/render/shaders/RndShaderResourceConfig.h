#pragma once

#include <cstddef>

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderEnums.h"
#include "utl/containers/Vector.h"

// The textures, samplers, and buffers a shader binds, per program type, and
// the slots they are declared at.
class RndShaderResourceConfig {
public:
    // One declared resource. Textures are kind 0, buffers kind 1. Field names
    // are not in the reference map.
    struct ResourceInfo {
        unsigned int mKind;
        unsigned int mTextureType;  // An RndTextureBase::Type; -1 for buffers.
        unsigned int mNumericType;  // An RndShaderNumericType; -1 when the
                                    // buffer is custom typed.
        int mUsage;                 // RndComputeBufferUsage; -1 for textures.
        const char* mName;
        const char* mSamplerName;
        const char* mStructName;
        unsigned long mRTSliced;
        unsigned long mIndex;       // -1 hides the declaration.
        unsigned long mRegister;
    };

    // Resource lists per program type: sampled textures, unsampled
    // textures, input buffers, then writable resources.
    enum {
        kSampledTextures = 0,
        kTextures = 6,
        kInputBuffers = 12,
        kWritableResources = 18,
    };

    RndShaderResourceConfig();

    // The texture type is an RndTextureBase::Type; the map's parameter is
    // RndTextureType.
    unsigned long AddTexture(
        const char* name,
        const char* sampler,
        unsigned int textureType,
        RndShaderProgramType type,
        RndShaderNumericType numericType);  // 0x643260
    unsigned long AddTexture2DRTSliced(
        const char* name,
        const char* sampler,
        unsigned int textureType,
        RndShaderNumericType numericType);  // 0x6438D0
    // The map's signature has no program type.
    unsigned long AddTextureWritable(
        const char* name,
        unsigned int textureType,
        RndShaderProgramType type,
        RndShaderNumericType numericType);  // 0x643670
    unsigned long AddComputeBuffer(
        const char* name,
        unsigned int usage,
        RndShaderNumericType numericType,
        RndShaderProgramType type);  // 0x643C70
    unsigned long AddComputeBufferWritable(
        const char* name,
        unsigned int usage,
        RndShaderNumericType numericType,
        RndShaderProgramType type);  // 0x643EF0
    unsigned long AddComputeBufferCustomTyped(
        const char* name,
        const char* structName,
        unsigned int usage,
        RndShaderProgramType type);  // 0x644150
    unsigned long AddComputeBufferCustomTypedWritable(
        const char* name,
        const char* structName,
        unsigned int usage,
        RndShaderProgramType type);  // 0x644400
    // The map's parameter is TextStream&; see RndShaderCBufferConfig.
    void PrintCode(unsigned int& hash) const;

    void _PrintTexture(const ResourceInfo& info, bool write, bool metal, unsigned int& hash) const;  // 0x644C80
    void _PrintComputeBuffer(const ResourceInfo& info, bool write, bool metal, unsigned int& hash) const;  // 0x644E40
    void _PrintResourceVec(
        const eastl::vector<ResourceInfo>& resources,
        bool write,
        bool metal,
        unsigned int& hash) const;

    // Name not in the reference map.
    unsigned long _NumReadResources(RndShaderProgramType type) const;

    // Field names are not in the reference map.
    eastl::vector<ResourceInfo> mResources[24];
    unsigned long mNumRegisters[12];  // Textures per type, then buffers.
};

static_assert(sizeof(RndShaderResourceConfig::ResourceInfo) == 64);
static_assert(sizeof(RndShaderResourceConfig) == 864);
