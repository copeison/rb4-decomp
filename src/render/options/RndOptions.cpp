// render/RndOptions.o (0x468000 to 0x46A830). The nine "*Options" classes'
// Init registrations and factories that the object also emits are not
// reconstructed.
#include "render/options/RndOptions.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "render/options/RndOptionsCom.h"

Entity* RndOptions::gEntity;
bool RndOptions::gSuppressed;
ResourceMetaData RndOptionsResource::sMetaData;

// Reconstructed from eboot.elf at 0x469CA0.
bool RndOptions::IsSuppressed() {
    return gSuppressed;
}

// Reconstructed from eboot.elf at 0x469CB0.
void RndOptions::SetSuppressed(bool suppressed) {
    if (suppressed == gSuppressed) {
        return;
    }
    gSuppressed = suppressed;
    GameObject* const root = gEntity->GetRoot();
    for (unsigned int i = 0; i < root->mComs.size(); ++i) {
        Component* const com = root->mComs[i].mCom;
        if (com != nullptr && com->IsA(RndOptionsCom::sClassName)) {
            static_cast<RndOptionsCom*>(com)->mRuntimeData.mSuppressed = suppressed;
        }
    }
}

// Reconstructed from eboot.elf at 0x469D50.
GameObject* RndOptions::GetOwner() {
    return gEntity->GetRoot();
}

// Reconstructed from eboot.elf at 0x469D70.
Symbol RndOptions::GetEntityTypeId() {
    return RndOptionsResource::Id();
}

// Reconstructed from eboot.elf at 0x469E10.
void RndOptionsResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("rndopt"));
    metaData.mCategory = Symbol("Renderer Options");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x469F30.
void RndOptionsResource::Save(BinStream& stream, bool cached) {
    EntityResource::Save(stream, cached);
}

// Reconstructed from eboot.elf at 0x469F40.
Entity* RndOptionsResource::CreateEntity() {
    Entity* const entity = _NewEntity();
    GameObject* const root = entity->CreateObject(0, 0);
    root->SetName(Symbol("options"));
    return entity;
}

// Reconstructed from eboot.elf at 0x469FB0.
ResourceMetaData* RndOptionsResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x469FC0.
Symbol RndOptionsResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x46A060.
bool RndOptionsResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x46A090. The deleting destructor is at
// 0x46A0A0.
RndOptionsResource::~RndOptionsResource() {}
