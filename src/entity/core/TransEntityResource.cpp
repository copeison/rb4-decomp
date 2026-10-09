#include "entity/core/TransEntityResource.h"

#include <cstring>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/InstanceCom.h"
#include "entity/core/TransCom.h"
#include "math/transform/Transform.h"
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

// Reconstructed from eboot.elf at 0x1BAC60. Revisions 12 and 13 carry the
// root data, which is skipped when it is not wanted; revisions before 12
// have no header, and their root data is the root's instance component's
// icon data.
void TransEntityResource::_LoadRoot(
    BinStream& stream,
    Entity* entity,
    eastl::vector<ResourcePath>* paths,
    eastl::vector<unsigned char>* rootData) {
    stream.ReadEndian(&mRev, sizeof(mRev));
    if (mRev < 12) {
        stream.Seek(-4, kSeekCur);
    } else if (mRev <= 13) {
        unsigned int size;
        if (rootData == nullptr) {
            stream.ReadEndian(&size, sizeof(size));
            stream.Seek(stream.Tell() + size, kSeekBegin);
        } else {
            stream.ReadEndian(&size, sizeof(size));
            rootData->resize(size);
            if (size != 0) {
                stream.Read(rootData->data(), size);
            }
        }
    }
    if (mRev >= 15) {
        stream >> mInstanceComId;
    }
    EntityResource::_LoadRoot(stream, entity, paths, rootData);
    if (mRev > 11) {
        return;
    }
    InstanceCom* const instance = entity->GetRoot()->GetCom<InstanceCom>();
    if (rootData != nullptr) {
        const unsigned int size = instance->mIconData.mSize;
        if (size != 0) {
            rootData->resize(size);
            std::memcpy(rootData->data(), instance->mIconData.mData, size);
        }
    }
}

// Reconstructed from eboot.elf at 0x1BAF10. The root's instance component
// must exist; its search is unbounded.
bool TransEntityResource::_LoadEntity(BinStream& stream, bool cached) {
    stream.ReadEndian(&mRev, sizeof(mRev));
    if (mRev < 12) {
        stream.Seek(-4, kSeekCur);
    } else if (mRev <= 13) {
        unsigned int size;
        stream.ReadEndian(&size, sizeof(size));
        mRootData.resize(size);
        if (size != 0) {
            stream.Read(mRootData.data(), size);
        }
    }
    if (mRev >= 15) {
        stream >> mInstanceComId;
    }
    if (!EntityResource::_LoadEntity(stream, cached)) {
        return false;
    }
    GameObject* const root = mEntity->GetRoot();
    const InstanceCom* const instance = root->GetExistingCom<InstanceCom>();
    if (!instance->mDrivesParent) {
        Transform identity;
        identity.m.x = {1.0F, 0.0F, 0.0F};
        identity.m.y = {0.0F, 1.0F, 0.0F};
        identity.m.z = {0.0F, 0.0F, 1.0F};
        identity.v = {0.0F, 0.0F, 0.0F};
        root->GetCom<TransCom>()->SetLocalXfm(identity);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x1BB0E0.
bool TransEntityResource::_PostLoad(BinStream& stream, bool cached) {
    static_cast<void>(stream);
    static_cast<void>(cached);
    if (mRev <= 14) {
        const GameObject* const root = mEntity->GetRoot();
        for (const GameObject::ComIndex& index : root->mComs) {
            Component* const component = index.mCom;
            if (component->_GetMetaData().mInstanceCom) {
                mInstanceComId = component->GetId();
                break;
            }
        }
    }
    return true;
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
        static_cast<TransCom*>(object->CreateComponent(TransCom::sClassName, false));
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

// Reconstructed from eboot.elf at 0x1BB660.
void TransEntityResource::_PollEntity(Entity* entity) {
    if (!mSuppressPoll || (entity->mFlags & 0x16) != 0x12) {
        EntityResource::_PollEntity(entity);
        return;
    }
    InstanceCom* const instance = entity->GetRoot()->GetCom<InstanceCom>();
    if (instance == nullptr || !instance->mPollRequested) {
        return;
    }
    ThreadPollContext& context = gEntityThreadState;
    const bool enterImmediately = context.mEnterImmediately;
    context.mEnterImmediately = false;
    EntityResource::_PollEntity(entity);
    instance->mPollRequested = false;
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
