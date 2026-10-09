#pragma once

#include "render/core/transition_aliases.h"
#include "render/shaders/RndShaderEnums.h"

class RndFence;
class RndOcclusionQuery;
class RndParticleBuffer;
class RndShaderCBuffer;
class RndShaderProgram;

#include "render/buffers/RndComputeBuffer.h"
#include "render/meshes/RndMesh.h"
#include "render/textures/RndTexture1D.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureArray2D.h"
#include "render/textures/RndTextureArrayCube.h"
#include "render/textures/RndTextureCube.h"


// Platform object factory.
class RndFactory {
public:
    virtual ~RndFactory() {}  // slots 0-1

    // Slot 2. Not in the reference map, which predates fences.
    virtual RndFence* CreateFence() = 0;
    virtual RndMesh* CreateMesh(RndVertexType type, const char* name) = 0;
    virtual RndTexture1D* CreateTexture1D(const RndTexture1D::Description& desc) = 0;
    virtual RndTexture2D* CreateTexture2D(const RndTexture2D::Description& desc) = 0;
    virtual RndTexture3D* CreateTexture3D(const RndTexture3D::Description& desc) = 0;
    virtual RndTextureCube* CreateTextureCube(const RndTextureCube::Description& desc) = 0;
    virtual RndTextureArray1D* CreateTextureArray1D(const RndTextureArray1D::Description& desc) = 0;
    virtual RndTextureArray2D* CreateTextureArray2D(const RndTextureArray2D::Description& desc) = 0;
    virtual RndTextureArrayCube* CreateTextureArrayCube(const RndTextureArrayCube::Description& desc) = 0;
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
