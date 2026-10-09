#include "render/drawing/RndTexturedQuadCom.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"

namespace {

// The quad every instance draws. Name not in the reference map.
RndMesh* gQuadMesh = nullptr;  // 0x1A750B8

}  // namespace

// Reconstructed from eboot.elf at 0x44C8B0. A unit quad in the xz plane.
void RndTexturedQuadCom::PostInit() {
    RndMeshUtl::CreateQuadParams params;
    params.mVertexType = kVertexColorTex;
    params.mAxisU = {1.0F, 0.0F, 0.0F};
    params.mAxisV = {0.0F, 0.0F, 1.0F};
    params.mNumSegmentsU = 1;
    params.mNumSegmentsV = 1;
    gQuadMesh = RndMeshUtl::CreateQuad(params);
}

// Reconstructed from eboot.elf at 0x44C930.
void RndTexturedQuadCom::Terminate() {
    delete gQuadMesh;
    gQuadMesh = nullptr;
}

// Reconstructed from eboot.elf at 0x44C960.
RndMesh* RndTexturedQuadCom::GetMesh() {
    return gQuadMesh;
}
