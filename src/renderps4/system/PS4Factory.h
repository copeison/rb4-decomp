#pragma once

#include "render/system/RndFactory.h"

// PS4 object factory. The mesh and texture slots forward to the unconverted
// Orbis creation functions.
class PS4Factory : public RndFactory {
public:
    ~PS4Factory() override {}

    RndFence* CreateFence() override;                                        // 0x8D85C0
    rb4::RenderMesh* CreateMesh(rb4::RenderMeshFormat type, const char* name) override;
    rb4::RenderTexture1D* CreateTexture1D(
        const rb4::RenderTexture1DDescriptor& desc) override;
    rb4::RenderTexture2D* CreateTexture2D(
        const rb4::RenderTexture2DDescriptor& desc) override;
    rb4::RenderTexture3D* CreateTexture3D(
        const rb4::RenderTexture3DDescriptor& desc) override;
    rb4::RenderTextureCube* CreateTextureCube(
        const rb4::RenderTextureCubeDescriptor& desc) override;
    rb4::RenderTextureArray1D* CreateTextureArray1D(
        const rb4::RenderTextureArray1DDescriptor& desc) override;
    rb4::RenderTextureArray2D* CreateTextureArray2D(
        const rb4::RenderTextureArray2DDescriptor& desc) override;
    rb4::RenderTextureArrayCube* CreateTextureArrayCube(
        const rb4::RenderTextureArrayCubeDescriptor& desc) override;
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
