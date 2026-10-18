// render/RndMaterialRuntimeDataResource.o (0x4F9990 to 0x4F9D50).
#include "render/materials/RndMaterialRuntimeDataResource.h"

#include "render/materials/RndMaterialRuntimeData.h"

// The object's static, built by its static initializer.
ResourceMetaData RndMaterialRuntimeDataResource::sMetaData;

// Reconstructed from eboot.elf at 0x4F9990.
RndMaterialRuntimeDataResource::RndMaterialRuntimeDataResource() {}

// Reconstructed from eboot.elf at 0x4F9A00.
RndMaterialRuntimeDataResource::RndMaterialRuntimeDataResource(
    RndMaterialRuntimeData* data)
    : mData(data) {}

// Reconstructed from eboot.elf at 0x4F9A70. The deleting destructor is at
// 0x4F9AA0.
RndMaterialRuntimeDataResource::~RndMaterialRuntimeDataResource() {
    RndMaterialRuntimeData::Delete(mData);
}

// Reconstructed from eboot.elf at 0x4F9AE0.
void RndMaterialRuntimeDataResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("materialruntime"));
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
    metaData.mTypeFlags[1] = false;
    metaData.mTypeFlags[2] = false;
}

// Reconstructed from eboot.elf at 0x4F9BF0.
bool RndMaterialRuntimeDataResource::Load(BinStream&, bool) {
    return false;
}

// Reconstructed from eboot.elf at 0x4F9C00.
void RndMaterialRuntimeDataResource::Save(BinStream&, bool) {}

// Reconstructed from eboot.elf at 0x4F9C10.
bool RndMaterialRuntimeDataResource::Fail() const {
    return mData == nullptr;
}

// Reconstructed from eboot.elf at 0x4F9C20.
ResourceMetaData* RndMaterialRuntimeDataResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x4F9C30.
Symbol RndMaterialRuntimeDataResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x4F9CD0.
bool RndMaterialRuntimeDataResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}
