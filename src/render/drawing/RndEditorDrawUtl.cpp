#include "render/drawing/RndEditorDrawUtl.h"

#include "entity/resources/Resource.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndVertex.h"
#include "render/textures/RndTexture2DResource.h"

namespace {

// The texture drawn for selected control points. Name not in the reference
// map. The binary registers no exit destructor for it, where this
// reconstruction does; Terminate releases it, so that destructor finds it
// empty.
ResourcePtr<RndTexture2DResource> gControlPointTexture;  // 0x1A86298
// The arrow drawn by the editor. Name not in the reference map.
RndMesh* gArrowMesh = nullptr;  // 0x1A862A0

// The arrow's triangles over its seven vertices. Name not in the reference
// map.
const RndMesh::Face kArrowFaces[] = {
    {{0, 1, 2}},
    {{0, 2, 6}},
    {{2, 4, 6}},
    {{2, 3, 4}},
    {{4, 5, 6}},
};

}  // namespace

// Reconstructed from eboot.elf at 0x461D10. The arrow's vertex positions
// are left for its users to fill.
void RndEditorDrawUtl::Init() {
    gControlPointTexture = Resource::GetOrLoad<RndTexture2DResource>(
        ResourcePath("../../system/data/editordraw/control_point_selected_pma.png"),
        false);

    gArrowMesh = RndMesh::New(kVertexPosOnly, "Arrow Mesh");
    gArrowMesh->_SetNumVerticesImpl(7);
    gArrowMesh->SetVertexUsageFlags(1);
    constexpr unsigned long kNumFaces = sizeof(kArrowFaces) / sizeof(kArrowFaces[0]);
    gArrowMesh->mFaces.resize(kNumFaces);
    for (unsigned long face = 0; face < kNumFaces; ++face) {
        gArrowMesh->mFaces[face] = kArrowFaces[face];
    }
    gArrowMesh->SyncStatic();
}

// Reconstructed from eboot.elf at 0x461F50.
void RndEditorDrawUtl::Terminate() {
    gControlPointTexture = nullptr;
    delete gArrowMesh;
    gArrowMesh = nullptr;
}
