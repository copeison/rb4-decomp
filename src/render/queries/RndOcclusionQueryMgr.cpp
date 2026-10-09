#include "render/queries/RndOcclusionQueryMgr.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/queries/RndShaderDisplayOcclusionQueryCoverage.h"
#include "render/queries/RndShaderOcclusionQuery.h"

namespace {

// Names not in the reference map.
RndShaderDisplayOcclusionQueryCoverage* gDisplayCoverageShader = nullptr;  // 0x1AA7C20
RndMesh* gOcclusionSphere = nullptr;                                       // 0x1AA7C28
RndShaderOcclusionQuery* gOcclusionQueryShader = nullptr;                  // 0x1AA7C30

// Creates a shader and registers it with the shader manager. Name not in
// the reference map.
template <class T>
T* CreateShader() {
    auto* shader = new T;
    shader->_Register();
    return shader;
}

// Name not in the reference map.
template <class T>
void SafeDelete(T*& object) {
    delete object;
    object = nullptr;
}

}  // namespace

// Reconstructed from eboot.elf at 0x5F7ED0.
void RndOcclusionQueryMgr::Init() {
    RndMeshUtl::CreateSphereParams params;
    params.mName = "Occlusion Query Sphere";
    params.mVertexType = kVertexPosOnly;
    params.mNumSegments = 8;
    params.mRadius = 1.0F;
    params.mNumRings = 4;
    gOcclusionSphere = RndMeshUtl::CreateSphere(params);
    gOcclusionQueryShader = CreateShader<RndShaderOcclusionQuery>();
    gDisplayCoverageShader = CreateShader<RndShaderDisplayOcclusionQueryCoverage>();
}

// Reconstructed from eboot.elf at 0x5F7FB0.
void RndOcclusionQueryMgr::Terminate() {
    SafeDelete(gOcclusionSphere);
    SafeDelete(gOcclusionQueryShader);
    SafeDelete(gDisplayCoverageShader);
}

// Reconstructed from eboot.elf at 0x5F7EC0.
RndShaderDisplayOcclusionQueryCoverage* RndOcclusionQueryMgr::GetDisplayCoverageShader() {
    return gDisplayCoverageShader;
}
