#include "render/meshes/RndPrimitiveMeshes.h"

#include "render/meshes/RndMesh.h"

// The mesh builders are not identified yet; the map's
// RndMeshUtl::CreateBox and CreateCylinder take parameter blocks. Names not
// in the reference map.
RndMesh* RndCreateDefaultBoxMesh();
RndMesh* RndCreateDefaultCylinderMesh();

// Reconstructed from eboot.elf at 0x460640.
RndPrimitiveMeshes::RndPrimitiveMeshes() {
    mMeshes.mData = mMeshes.mStorage;
    mMeshes.mSize = 2;
    mMeshes.mCapacity = 2;
    mMeshes.mStorage[0] = RndCreateDefaultBoxMesh();
    mMeshes.mStorage[1] = RndCreateDefaultCylinderMesh();
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
