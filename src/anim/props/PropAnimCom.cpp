// anim/PropAnimCom.o (0x2FBC00-0x2FCAB1). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropAnimCom.h"

#include <float.h>
#include <new>

#include "anim/props/PropAnimResource.h"
#include "anim/props/PropKeysCom.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropMetadata.h"

// The object's statics, in the order of its static initializer (0x2FCA00).
Symbol PropAnimCom::sId("PropAnim");
Symbol PropAnimCom::sClassName("PropAnim");
PropRegistry PropAnimCom::sPropRegistry;
ComMetaData PropAnimCom::sMetaData;

// Reconstructed from eboot.elf at 0x2FBC00.
PropAnimCom::PropAnimCom()
    : mFrameRangeStart(FLT_MAX), mFrameRangeEnd(-FLT_MAX), mTimeUnits(8), mLooping(false), mTargetOrder() {}

// Reconstructed from eboot.elf at 0x2FBC70.
void PropAnimCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    PropAnimCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    FloatMetadata& start = TypeSpecificMetadata(
        registry.RegisterProp("frame_range_start", offsetof(PropAnimCom, mFrameRangeStart), kPropertyFloat, 4),
        prototype.mFrameRangeStart);
    start.mDefault = prototype.mFrameRangeStart;
    FloatMetadata& end = TypeSpecificMetadata(
        registry.RegisterProp("frame_range_end", offsetof(PropAnimCom, mFrameRangeEnd), kPropertyFloat, 4),
        prototype.mFrameRangeEnd);
    end.mDefault = prototype.mFrameRangeEnd;
    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("time_units", offsetof(PropAnimCom, mTimeUnits), kPropertyInt, 4).mMetadata.Get());
    units.mDefault = prototype.mTimeUnits;
    BoolMetadata& looping = TypeSpecificMetadata(
        registry.RegisterProp("is_looping", offsetof(PropAnimCom, mLooping), kPropertyBool, 1), prototype.mLooping);
    looping.mDefault = prototype.mLooping;
    PropInfo& order = registry.RegisterPropArray(
        "target_order", offsetof(PropAnimCom, mTargetOrder), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& orderMetadata = static_cast<ArrayMetadata&>(*order.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(order, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId nullId = {0xFFFFFFFFU};
    GameObjectIdMetadata& target = TypeSpecificMetadata(orderMetadata.mItemInfo, nullId);
    static_cast<void>(Symbol("item"));
    target.mEntity._Set([](const TagArgs& args) -> Entity* {
        return args.mComponent.mObject->mEntity->mResource->mInlineOwner->mEntity;
    });
    metadata.mDescription = "Com that contains the static PropAnim keys data";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    metadata.mAliases.push_back(Symbol("PropAnimCom"));
}

// Reconstructed from eboot.elf at 0x2FC2D0.
void PropAnimCom::UpdateFrameRange() {
    mFrameRangeStart = FLT_MAX;
    mFrameRangeEnd = -FLT_MAX;
    Entity* const entity = mObject->mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr; object = entity->NextObject(object, Symbol())) {
        if (object == entity->GetRoot()) {
            continue;
        }
        const Symbol keysInterface("PropKeys");
        const GameObject::ComIndex* index = object->mComs.data();
        while (index->mBaseId != keysInterface) {
            ++index;
        }
        const PropKeysBaseCom* const keys = static_cast<const PropKeysBaseCom*>(index->mCom);
        const float first = keys->GetFirstFrame();
        if (first < mFrameRangeStart) {
            mFrameRangeStart = first;
        }
        if (first > mFrameRangeEnd) {
            mFrameRangeEnd = first;
        }
        const float last = keys->GetLastFrame();
        if (last < mFrameRangeStart) {
            mFrameRangeStart = last;
        }
        if (last > mFrameRangeEnd) {
            mFrameRangeEnd = last;
        }
    }
}

// Reconstructed from eboot.elf at 0x2FC3E0.
void PropAnimCom::SetTargetOrder(const eastl::vector<GameObjectId>& order) {
    mTargetOrder.Resize(0);
    mTargetOrder.Reserve(static_cast<unsigned int>(order.size()));
    for (const GameObjectId* id = order.mpBegin; id != order.mpEnd; ++id) {
        const GameObjectId element = *id;
        mTargetOrder._Insert(mTargetOrder.mSize, &element);
    }
}

// Reconstructed from eboot.elf at 0x2FC4F0.
void PropAnimCom::GetTargetOrder(eastl::vector<GameObjectId>& order) const {
    order.clear();
    order.reserve(mTargetOrder.mSize);
    const GameObjectId* const ids = static_cast<const GameObjectId*>(mTargetOrder.mData);
    for (unsigned int i = 0; i < mTargetOrder.mSize; ++i) {
        order.push_back(ids[i]);
    }
}

// Reconstructed from eboot.elf at 0x2FC6A0.
void PropAnimCom::_Enter() {
    if (!sRegressionTesting) {
        UpdateFrameRange();
    }
}

// Reconstructed from eboot.elf at 0x2FC6C0.
void PropAnimCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x2FC6D0.
Symbol PropAnimCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2FC6E0.
Symbol PropAnimCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2FC6F0.
int PropAnimCom::CurrentRev() const {
    return const_cast<PropAnimCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2FC710.
bool PropAnimCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2FC740.
Component* PropAnimCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2FC750. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropAnimCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropAnimCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropAnimCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2FC8C0.
PropRegistry& PropAnimCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2FC8D0.
ComMetaData& PropAnimCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2FC8E0.
void PropAnimCom::_Poll() {}

// Reconstructed from eboot.elf at 0x2B1E10. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropAnimCom::_Create() {
    return new PropAnimCom();
}

// Reconstructed from eboot.elf at 0x2A7C70. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropAnimCom::Init() {
    InitComponentClass<PropAnimCom>();
}
