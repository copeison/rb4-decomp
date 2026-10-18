// The entity rendering settings component (0x6CA2D0 to 0x6CAE7B).
#include "render/scene/RndEntityCom.h"

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropMetadata.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/scene/RndEntityResource.h"
#include "render/system/RndUtl.h"
#include "utl/files/FileUtl.h"
#include "utl/threading/PollMgr.h"

// The object's statics, in the order of its static initializer (0x6CADB0).
// The three ints it first sets (0x1AB0AD0) come from a shared header and are
// not modelled.
Symbol RndEntityCom::sId("RndEntity");
Symbol RndEntityCom::sClassName("RndEntity");
PropRegistry RndEntityCom::sPropRegistry;
ComMetaData RndEntityCom::sMetaData;

// Reconstructed from eboot.elf at 0x6CA2D0.
RndEntityCom::RndEntityCom() : mPreviewLod(0) {}

// Reconstructed from eboot.elf at 0x6CA310. The class's constructor inlines
// it.
RndEntityCom::RuntimeData::RuntimeData()
    : mPollMgr(nullptr),
      mPollGroups(nullptr),
      mSceneDrawer(nullptr),
      mOwnsSceneDrawer(false),
      mNeedsImmediateEnter(false) {}

// Reconstructed from eboot.elf at 0x6CA320. The deleting destructor is at
// 0x6CA3A0.
RndEntityCom::~RndEntityCom() {
    delete mRuntimeData.mPollGroups;
    mRuntimeData.mPollGroups = nullptr;
    delete mRuntimeData.mPollMgr;
    mRuntimeData.mPollMgr = nullptr;
    if (mRuntimeData.mOwnsSceneDrawer) {
        delete mRuntimeData.mSceneDrawer;
    }
    mRuntimeData.mSceneDrawer = nullptr;
}

// Reconstructed from eboot.elf at 0x6CA430.
PollMgr* RndEntityCom::ObtainPollMgr() {
    if (mRuntimeData.mPollMgr == nullptr) {
        mRuntimeData.mPollMgr = new PollMgr(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    }
    return mRuntimeData.mPollMgr;
}

// Reconstructed from eboot.elf at 0x6CA480.
RndSceneCom::PollGroups* RndEntityCom::ObtainPollGroups() {
    if (mRuntimeData.mPollGroups == nullptr) {
        mRuntimeData.mPollGroups =
            new RndSceneCom::PollGroups(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    }
    return mRuntimeData.mPollGroups;
}

// Reconstructed from eboot.elf at 0x6CA4D0.
void RndEntityCom::FindAncestorSceneDrawer() {
    mRuntimeData.mSceneDrawer = RndSceneDrawer::FindForEntity(mObject->mEntity);
    mRuntimeData.mOwnsSceneDrawer = false;
}

// Reconstructed from eboot.elf at 0x6CA500.
RndSceneDrawer* RndEntityCom::ObtainSceneDrawer() {
    if (mRuntimeData.mSceneDrawer == nullptr) {
        mRuntimeData.mSceneDrawer = RndSceneDrawer::FindForEntity(mObject->mEntity);
        const bool owned = mRuntimeData.mSceneDrawer == nullptr;
        if (owned) {
            mRuntimeData.mSceneDrawer = new RndSceneDrawer();
        }
        mRuntimeData.mOwnsSceneDrawer = owned;
    }
    return mRuntimeData.mSceneDrawer;
}

// Reconstructed from eboot.elf at 0x6CA560.
RndSceneDrawer* RndEntityCom::ObtainOwnedSceneDrawer() {
    if (mRuntimeData.mSceneDrawer == nullptr) {
        mRuntimeData.mSceneDrawer = new RndSceneDrawer();
        mRuntimeData.mOwnsSceneDrawer = true;
    }
    return mRuntimeData.mSceneDrawer;
}

// Reconstructed from eboot.elf at 0x6CA5A0. Each preview level is named,
// and described, by its level name.
void RndEntityCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndDrawableEntityCom::_InitAsSuperclass(registry, metadata);
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mDescription = "Entity-wide rendering settings";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mEditorRestrictions = 0;
    metadata.mAllowedResources.push_back(RndEntityResource::Id());
    metadata.mLightweight = true;
    RndEntityCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    PropInfo& lodInfo =
        registry.RegisterProp("preview_lod", offsetof(RndEntityCom, mPreviewLod), kPropertyInt, 4);
    IntMetadata<int>& lod = static_cast<IntMetadata<int>&>(*lodInfo.mMetadata.Get());
    lod.mDefault = prototype.mPreviewLod;
    static_cast<void>(Symbol("prop"));
    lod.mHelp = String("Level of detail to draw when viewing this entity on its own");
    eastl::vector<AllowedValue<int>> levels;
    levels.reserve(kNumSceneLods);
    for (unsigned int level = 0; level < kNumSceneLods; ++level) {
        const char* const name = RndUtl::GetLodName(static_cast<RndSceneLod>(level));
        levels.push_back(AllowedValue<int>{static_cast<int>(level), String(name), String(name)});
    }
    lod.mAllowedValues = static_cast<eastl::vector<AllowedValue<int>>&&>(levels);
}

// Reconstructed from eboot.elf at 0x6CAB20.
void RndEntityCom::_Exit(DestroyType type) {
    static_cast<void>(type);
    if (!mRuntimeData.mOwnsSceneDrawer) {
        mRuntimeData.mSceneDrawer = nullptr;
    }
    if (mRuntimeData.mPollGroups != nullptr) {
        mRuntimeData.mPollGroups->Reset();
        delete mRuntimeData.mPollGroups;
        mRuntimeData.mPollGroups = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x6CAB70. The same body as _Exit.
void RndEntityCom::_EditExit(DestroyType type) {
    static_cast<void>(type);
    if (!mRuntimeData.mOwnsSceneDrawer) {
        mRuntimeData.mSceneDrawer = nullptr;
    }
    if (mRuntimeData.mPollGroups != nullptr) {
        mRuntimeData.mPollGroups->Reset();
        delete mRuntimeData.mPollGroups;
        mRuntimeData.mPollGroups = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x6CABC0.
Symbol RndEntityCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6CABD0.
Symbol RndEntityCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6CABE0.
int RndEntityCom::CurrentRev() const {
    return const_cast<RndEntityCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6CAC00.
bool RndEntityCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6CAC30.
Component* RndEntityCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6CAD90.
PropRegistry& RndEntityCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6CADA0.
ComMetaData& RndEntityCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4042A0. The binary emits the factory in
// render/RndInit.o with the class's Init.
Component* RndEntityCom::_Create() {
    return new RndEntityCom();
}

// Reconstructed from eboot.elf at 0x3ED3A0. The binary emits the registration
// in render/RndInit.o.
void RndEntityCom::Init() {
    InitComponentClass<RndEntityCom>();
}
