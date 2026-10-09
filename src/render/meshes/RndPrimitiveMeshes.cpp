#include "render/meshes/RndPrimitiveMeshes.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"

namespace {

// Vertex usage flag the primitive meshes request. Name not in the reference
// map.
constexpr unsigned int kPrimitiveVertexUsage = 4;

}  // namespace

// Reconstructed from eboot.elf at 0x460640. Both meshes take the builders'
// default names, "Box" and "Cylinder": a unit box, and a cylinder of radius
// 0.5 and height 1 with 12 segments around.
RndPrimitiveMeshes::RndPrimitiveMeshes() {
    mMeshes.mData = mMeshes.mStorage;
    mMeshes.mSize = 2;
    mMeshes.mCapacity = 2;

    RndMeshUtl::CreateBoxParams box;
    box.mVertexType = kVertexColorTex;
    box.mVertexUsageFlags = kPrimitiveVertexUsage;
    box.mAxisX = {1.0F, 0.0F, 0.0F};
    box.mAxisY = {0.0F, 1.0F, 0.0F};
    box.mAxisZ = {0.0F, 0.0F, 1.0F};
    box.mNumSegmentsX = 1;
    box.mNumSegmentsY = 1;
    box.mNumSegmentsZ = 1;
    mMeshes.mData[0] = RndMeshUtl::CreateBox(box);

    RndMeshUtl::CreateCylinderParams cylinder;
    cylinder.mVertexType = kVertexColorTex;
    cylinder.mVertexUsageFlags = kPrimitiveVertexUsage;
    cylinder.mNumSegments = 12;
    cylinder.mRadius = 0.5F;
    cylinder.mHeight = 1.0F;
    cylinder.mNumHeightSegments = 1;
    mMeshes.mData[1] = RndMeshUtl::CreateCylinder(cylinder);
}

// Reconstructed from eboot.elf at 0x460880.
RndPrimitiveMeshes::~RndPrimitiveMeshes() {
    for (unsigned long index = 0; index < mMeshes.mSize; ++index) {
        if (mMeshes.mData[index] != nullptr) {
            delete mMeshes.mData[index];
            mMeshes.mData[index] = nullptr;
        }
    }
    mMeshes.mSize = 0;
}
