#include "renderps4/system/PS4Factory.h"

#include "render/platform/orbis/meshes/orbis_mesh.h"
#include "render/platform/orbis/textures/orbis_texture_1d.h"
#include "render/platform/orbis/textures/orbis_texture_2d.h"
#include "render/platform/orbis/textures/orbis_texture_3d.h"
#include "render/platform/orbis/textures/orbis_texture_array_1d.h"
#include "render/platform/orbis/textures/orbis_texture_array_2d.h"
#include "render/platform/orbis/textures/orbis_texture_array_cube.h"
#include "render/platform/orbis/textures/orbis_texture_cube.h"
#include "renderps4/buffers/PS4ComputeBuffer.h"
#include "renderps4/buffers/PS4ParticleBuffer.h"
#include "renderps4/buffers/PS4ShaderCBuffer.h"
#include "renderps4/queries/PS4OcclusionQuery.h"
#include "renderps4/shaders/PS4ShaderProgramCompute.h"
#include "renderps4/shaders/PS4ShaderProgramGeometry.h"
#include "renderps4/shaders/PS4ShaderProgramPixel.h"
#include "renderps4/shaders/PS4ShaderProgramVertex.h"
#include "renderps4/system/PS4Fence.h"

// Reconstructed from eboot.elf at 0x8D85C0.
RndFence* PS4Factory::CreateFence() {
    return new PS4Fence;
}

rb4::RenderMesh* PS4Factory::CreateMesh(
    rb4::RenderMeshFormat type,
    const char* name) {
    return rb4::orbis_create_mesh(type, name);
}

rb4::RenderTexture1D* PS4Factory::CreateTexture1D(
    const rb4::RenderTexture1DDescriptor& desc) {
    return rb4::orbis_create_texture_1d(desc);
}

rb4::RenderTexture2D* PS4Factory::CreateTexture2D(
    const rb4::RenderTexture2DDescriptor& desc) {
    return rb4::orbis_create_texture_2d(desc);
}

rb4::RenderTexture3D* PS4Factory::CreateTexture3D(
    const rb4::RenderTexture3DDescriptor& desc) {
    return rb4::orbis_create_texture_3d(desc);
}

rb4::RenderTextureCube* PS4Factory::CreateTextureCube(
    const rb4::RenderTextureCubeDescriptor& desc) {
    return rb4::orbis_create_texture_cube(desc);
}

rb4::RenderTextureArray1D* PS4Factory::CreateTextureArray1D(
    const rb4::RenderTextureArray1DDescriptor& desc) {
    return rb4::orbis_create_texture_array_1d(desc);
}

rb4::RenderTextureArray2D* PS4Factory::CreateTextureArray2D(
    const rb4::RenderTextureArray2DDescriptor& desc) {
    return rb4::orbis_create_texture_array_2d(desc);
}

rb4::RenderTextureArrayCube* PS4Factory::CreateTextureArrayCube(
    const rb4::RenderTextureArrayCubeDescriptor& desc) {
    return rb4::orbis_create_texture_array_cube(desc);
}

// Reconstructed from eboot.elf at 0x8D8AD0. The buffer's element data is
// allocated inline after the object.
RndShaderCBuffer* PS4Factory::CreateShaderCBuffer(
    const RndShaderCBufferConfig& config,
    unsigned int flags,
    unsigned long numElements) {
    constexpr unsigned long kElementSize = 16;
    auto* storage = MemAlloc(
        sizeof(PS4ShaderCBuffer) + kElementSize * numElements, "cbuffer", 0);
    return new (storage) PS4ShaderCBuffer(
        config,
        flags,
        numElements,
        static_cast<unsigned char*>(storage) + sizeof(PS4ShaderCBuffer));
}

// Reconstructed from eboot.elf at 0x8D8B30. The hull and domain stages have
// no PS4 program.
RndShaderProgram* PS4Factory::CreateShaderProgram(RndShaderProgramType type) {
    switch (type) {
    case kShaderProgramVertex:
        return new PS4ShaderProgramVertex;
    case kShaderProgramGeometry:
        return new PS4ShaderProgramGeometry;
    case kShaderProgramPixel:
        return new PS4ShaderProgramPixel;
    case kShaderProgramCompute:
        return new PS4ShaderProgramCompute;
    default:
        return nullptr;
    }
}

// Reconstructed from eboot.elf at 0x8D8BC0.
RndComputeBuffer* PS4Factory::CreateComputeBuffer(
    const RndComputeBuffer::Description& desc) {
    return new PS4ComputeBuffer(desc);
}

// Reconstructed from eboot.elf at 0x8D8BF0.
RndParticleBuffer* PS4Factory::CreateParticleBuffer(
    unsigned long numParticles,
    const char* name) {
    return new PS4ParticleBuffer(numParticles, name);
}

// Reconstructed from eboot.elf at 0x8D8C30.
RndOcclusionQuery* PS4Factory::CreateOcclusionQuery(const char* name) {
    return new PS4OcclusionQuery(name);
}
