#include "entity/core/TransEntityResource.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "utl/streams/BinStream.h"
#include "utl/threading/PollMgr.h"

namespace {

// When set, every new object's TransCom is parented to the root. Nothing in
// this build writes the word at 0x19E4978, so it stays zero. Name not in
// the reference map.
int gParentObjectsToRoot;

// The revision Save writes. Name not in the reference map.
constexpr int kTransEntityRev = 15;

}  // namespace

// The class's metadata at 0x19E4980.
ResourceMetaData TransEntityResource::sMetaData;

// Reconstructed from eboot.elf at 0x1BAAA0.
TransEntityResource::TransEntityResource()
    : mSuppressPoll(false), mRev(kTransEntityRev), mInstanceComId() {}

// Reconstructed from eboot.elf at 0x1BAB40.
void TransEntityResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("transentity"));
    metaData.mTypeFlags[0] = false;
    metaData.mCategory = Symbol("Trans Entities");
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x1BB4A0.
void TransEntityResource::Save(BinStream& stream, bool cached) {
    const int rev = kTransEntityRev;
    stream.WriteEndian(&rev, sizeof(rev));
    stream << mInstanceComId;
    EntityResource::Save(stream, cached);
}

// Reconstructed from eboot.elf at 0x1BB520.
void TransEntityResource::InitObject(GameObject* object) {
    EntityResource::InitObject(object);
    Entity* const entity = object->mEntity;
    TransCom* const trans =
        reinterpret_cast<TransCom*>(object->CreateComponent(TransCom::sClassName, false));
    if (gParentObjectsToRoot != 0) {
        GameObject* const root = entity->GetRoot();
        if (root != nullptr && root != object) {
            trans->SetTransParent(root->mId, false);
        }
    }
}

// Reconstructed from eboot.elf at 0x1BB580. A suppressed entity enters with
// the thread's immediate flag cleared, so its objects are not entered at
// once; flag bit 0 overrides this.
void TransEntityResource::_EnterEntity(Entity* entity, unsigned int flags) {
    if ((flags & 1) != 0 || !mSuppressPoll) {
        EntityResource::_EnterEntity(entity, flags);
        return;
    }
    ThreadPollContext& context = gEntityThreadState;
    const bool enterImmediately = context.mEnterImmediately;
    context.mEnterImmediately = false;
    EntityResource::_EnterEntity(entity, flags);
    context.mEnterImmediately = enterImmediately;
}

// Reconstructed from eboot.elf at 0x1BB7B0.
void TransEntityResource::_EnterImmediately(Entity* entity) {
    if (!mSuppressPoll || (entity->mFlags & 0x16) != 0x12) {
        EntityResource::_EnterImmediately(entity);
    }
}

// Reconstructed from eboot.elf at 0x1BB7D0.
void TransEntityResource::_PollImmediately(Entity* entity) {
    if (!mSuppressPoll) {
        EntityResource::_PollImmediately(entity);
    }
}

// Reconstructed from eboot.elf at 0x1BB7E0.
ResourceMetaData* TransEntityResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x1BB7F0.
Symbol TransEntityResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x1BB890.
bool TransEntityResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x1BB8C0. The property registries and the
// icon data are released before the entity resource.
TransEntityResource::~TransEntityResource() {}
