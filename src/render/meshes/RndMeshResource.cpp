// render/RndMeshResource.o (0x5D1A00 to 0x5D4800). Load, Save, the mesh
// rebuild (0x5D27F0), the companion-file reader (0x5D4180) and the option
// setters are not reconstructed.
#include "render/meshes/RndMeshResource.h"

#include "render/meshes/RndMesh.h"

// The object's static, built by its static initializer.
ResourceMetaData RndMeshResource::sMetaData;

// Reconstructed from eboot.elf at 0x5D1A00.
RndMeshResource::RndMeshResource()
    : mMesh(nullptr),
      mCompressVertices(true),
      mSmoothTangents(false),
      mCalcUVScale(false),
      mCalcCurvature(false) {}

// Reconstructed from eboot.elf at 0x5D1A80.
RndMeshResource::RndMeshResource(RndMesh* mesh) : mMesh(mesh) {}

// Reconstructed from eboot.elf at 0x5D1AF0. The deleting destructor is at
// 0x5D1B30.
RndMeshResource::~RndMeshResource() {
    delete mMesh;
    mMesh = nullptr;
}

// Reconstructed from eboot.elf at 0x5D1B80.
void RndMeshResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("mesh"));
    metaData.mCategory = Symbol("Meshes");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
    metaData.mTypeFlags[5] = true;
}

// Reconstructed from eboot.elf at 0x5D1CA0.
bool RndMeshResource::Fail() const {
    return mMesh == nullptr;
}

// Reconstructed from eboot.elf at 0x5D1CB0.
bool RndMeshResource::SupportsCompanionFile() const {
    return !mInlined;
}

// Reconstructed from eboot.elf at 0x5D40B0.
bool RndMeshResource::IsKeepMeshData() const {
    return mMesh->mKeepMeshData;
}

// Reconstructed from eboot.elf at 0x5D4100.
unsigned int RndMeshResource::GetVertexUsageFlags() const {
    return mMesh->mVertexUsageFlags;
}

// Reconstructed from eboot.elf at 0x5D4140.
unsigned int RndMeshResource::GetFaceUsageFlags() const {
    return mMesh->mFaceUsageFlags;
}

// Reconstructed from eboot.elf at 0x5D46A0.
ResourceMetaData* RndMeshResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x5D46B0.
Symbol RndMeshResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x5D4750.
bool RndMeshResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}
