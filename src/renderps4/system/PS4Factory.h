#pragma once

#include "render/system/RndFactory.h"

// PS4 object factory. The mesh slot forwards to the unconverted Orbis mesh
// creation function.
class PS4Factory : public RndFactory {
public:
    ~PS4Factory() override {}

    RndFence* CreateFence() override;                                        // 0x8D85C0
    rb4::RenderMesh* CreateMesh(rb4::RenderMeshFormat type, const char* name) override;
    RndTexture1D* CreateTexture1D(const RndTexture1D::Description& desc) override;
    RndTexture2D* CreateTexture2D(const RndTexture2D::Description& desc) override;
    RndTexture3D* CreateTexture3D(const RndTexture3D::Description& desc) override;
    RndTextureCube* CreateTextureCube(const RndTextureCube::Description& desc) override;
    RndTextureArray1D* CreateTextureArray1D(const RndTextureArray1D::Description& desc) override;
    RndTextureArray2D* CreateTextureArray2D(const RndTextureArray2D::Description& desc) override;
    RndTextureArrayCube* CreateTextureArrayCube(const RndTextureArrayCube::Description& desc) override;
    RndShaderCBuffer* CreateShaderCBuffer(
        const RndShaderCBufferConfig& config,
        unsigned int flags,
        unsigned long numElements) override;                                 // 0x8D8AD0
    RndShaderProgram* CreateShaderProgram(RndShaderProgramType type) override;  // 0x8D8B30
    RndComputeBuffer* CreateComputeBuffer(
        const RndComputeBuffer::Description& desc) override;                 // 0x8D8BC0
    RndParticleBuffer* CreateParticleBuffer(
        unsigned long numParticles,
        const char* name) override;                                          // 0x8D8BF0
    RndOcclusionQuery* CreateOcclusionQuery(const char* name) override;      // 0x8D8C30
};

static_assert(sizeof(PS4Factory) == 8);
