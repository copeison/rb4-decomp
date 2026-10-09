#include "render/particles/RndParticleBouncePlaneEditCom.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"

namespace {

// The box the bounce planes are drawn with. Name not in the reference map.
RndMesh* gBoxMesh = nullptr;  // 0x1AA8E48

}  // namespace

// Reconstructed from eboot.elf at 0x601880. A unit box on the axes.
void RndParticleBouncePlaneEditCom::StaticInit() {
    RndMeshUtl::CreateBoxParams params;
    params.mAxisX = {1.0F, 0.0F, 0.0F};
    params.mAxisY = {0.0F, 1.0F, 0.0F};
    params.mAxisZ = {0.0F, 0.0F, 1.0F};
    params.mNumSegmentsX = 1;
    params.mNumSegmentsY = 1;
    params.mNumSegmentsZ = 1;
    gBoxMesh = RndMeshUtl::CreateBox(params);
}

// Reconstructed from eboot.elf at 0x601910.
void RndParticleBouncePlaneEditCom::StaticTerminate() {
    delete gBoxMesh;
    gBoxMesh = nullptr;
}
