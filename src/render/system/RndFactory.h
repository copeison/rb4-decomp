#pragma once

#include "render/core/transition_aliases.h"
#include "render/shaders/RndShaderEnums.h"

class RndFence;
class RndOcclusionQuery;
class RndParticleBuffer;
class RndShaderCBuffer;
class RndShaderProgram;

#include "render/buffers/RndComputeBuffer.h"

namespace rb4 {
struct RenderMesh;
struct RenderTexture1D;
struct RenderTexture1DDescriptor;
struct RenderTexture2D;
struct RenderTexture2DDescriptor;
struct RenderTexture3D;
struct RenderTexture3DDescriptor;
struct RenderTextureArray1D;
struct RenderTextureArray1DDescriptor;
struct RenderTextureArray2D;
struct RenderTextureArray2DDescriptor;
struct RenderTextureArrayCube;
struct RenderTextureArrayCubeDescriptor;
struct RenderTextureCube;
struct RenderTextureCubeDescriptor;
enum class RenderMeshFormat : unsigned int;
}  // namespace rb4

// Platform object factory. The mesh and texture slots still use the types of
// the unconverted texture and mesh code.
class RndFactory {
public:
    virtual ~RndFactory() {}  // slots 0-1

    // Slot 2. Not in the reference map, which predates fences.
    virtual RndFence* CreateFence() = 0;
    virtual rb4::RenderMesh* CreateMesh(rb4::RenderMeshFormat type, const char* name) = 0;
    virtual rb4::RenderTexture1D* CreateTexture1D(
        const rb4::RenderTexture1DDescriptor& desc) = 0;
    virtual rb4::RenderTexture2D* CreateTexture2D(
        const rb4::RenderTexture2DDescriptor& desc) = 0;
    virtual rb4::RenderTexture3D* CreateTexture3D(
        const rb4::RenderTexture3DDescriptor& desc) = 0;
    virtual rb4::RenderTextureCube* CreateTextureCube(
        const rb4::RenderTextureCubeDescriptor& desc) = 0;
    virtual rb4::RenderTextureArray1D* CreateTextureArray1D(
        const rb4::RenderTextureArray1DDescriptor& desc) = 0;
    virtual rb4::RenderTextureArray2D* CreateTextureArray2D(
        const rb4::RenderTextureArray2DDescriptor& desc) = 0;
    virtual rb4::RenderTextureArrayCube* CreateTextureArrayCube(
        const rb4::RenderTextureArrayCubeDescriptor& desc) = 0;
    virtual RndShaderCBuffer* CreateShaderCBuffer(
        const RndShaderCBufferConfig& config,
        unsigned int flags,
        unsigned long numElements) = 0;
    virtual RndShaderProgram* CreateShaderProgram(RndShaderProgramType type) = 0;
    virtual RndComputeBuffer* CreateComputeBuffer(
        const RndComputeBuffer::Description& desc) = 0;
    virtual RndParticleBuffer* CreateParticleBuffer(
        unsigned long numParticles,
        const char* name) = 0;
    virtual RndOcclusionQuery* CreateOcclusionQuery(const char* name) = 0;
};

static_assert(sizeof(RndFactory) == 8);

// The render system's installed factory. Name not in the reference map.
RndFactory* TheRndFactory();
