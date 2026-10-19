// anim/CharBoneMaskResource.o (0x2E4C10 to 0x2E4ECF). The class's
// registration and factory are emitted in anim/AnimInit.o.
#include "anim/char/CharBoneMaskResource.h"

#include "anim/char/CharBoneMaskCom.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"

// The object's statics, in the order of its static initializer (0x2E4E90).
ResourceMetaData CharBoneMaskResource::sMetaData;

// Reconstructed from eboot.elf at 0x2E4C10.
void CharBoneMaskResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("bonemask"));
    metaData.mCategory = Symbol("Animations");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x2E4D30.
void CharBoneMaskResource::Save(BinStream& stream, bool cached) {
    EntityResource::Save(stream, cached);
}

// Reconstructed from eboot.elf at 0x2E4D40.
Entity* CharBoneMaskResource::CreateEntity() {
    Entity* const entity = EntityResource::CreateEntity();
    entity->GetRoot()->CreateComponent(CharBoneMaskCom::sClassName, false);
    return entity;
}

// Reconstructed from eboot.elf at 0x2E4D80.
ResourceMetaData* CharBoneMaskResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2E4D90.
Symbol CharBoneMaskResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2E4E30.
bool CharBoneMaskResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2B5E90. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* CharBoneMaskResource::_Create() {
    return new CharBoneMaskResource();
}

// Reconstructed from eboot.elf at 0x2AE160. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharBoneMaskResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("EntityResource"), true);
}
