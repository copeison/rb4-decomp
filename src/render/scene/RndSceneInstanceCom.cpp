// render/RndSceneInstanceCom.o (0x4362E0 to 0x438200). The registry
// (0x436AB0, 0x437910), the masking scenes (0x436710 and its callers
// 0x436700 and 0x436A80), the poll dependencies (0x437B50) and the
// property handlers (0x437FB0-0x4380E0) are not reconstructed.
#include "render/scene/RndSceneInstanceCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityHeaderCom.h"
#include "math/vector/Vector2i.h"
#include "os/platform/PlatformMgr.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/system/RndCapabilities.h"
#include "render/system/RndDevice.h"

// The object's statics, in the order of its static initializer (0x4380F0).
// The -1, 8 and 4 it first sets (0x1A72AC8) come from a shared header and
// are not modelled.
Symbol RndSceneInstanceCom::sId("SceneInstance");
Symbol RndSceneInstanceCom::sClassName("SceneInstance");
PropRegistry RndSceneInstanceCom::sPropRegistry;
ComMetaData RndSceneInstanceCom::sMetaData;

// Reconstructed from eboot.elf at 0x4362E0. The scene always polls until
// _Enter decides otherwise.
RndSceneInstanceCom::RndSceneInstanceCom()
    : mDrawOrder(0),
      mUniqueResource(false),
      mUseThreadedPoll(false),
      mProjectionOffset{0.0F, 0.0F},
      mProjectionRect{1.0F, 1.0F},
      mMaskedByScenes(),
      mMaskedByDynamicScenes(),
      mDisableLoadTag(),
      mReenterSharedChildren(false),
      mRuntimeData{true, false, false, PropArray<GameObjectId>()} {}

// Inlined into _Imprint at 0x437CD2.
RndSceneInstanceCom::RndSceneInstanceCom(const RndSceneInstanceCom& other)
    : InstanceCom(other),
      mDrawOrder(other.mDrawOrder),
      mUniqueResource(other.mUniqueResource),
      mUseThreadedPoll(other.mUseThreadedPoll),
      mProjectionOffset(other.mProjectionOffset),
      mProjectionRect(other.mProjectionRect),
      mMaskedByScenes(),
      mMaskedByDynamicScenes(),
      mDisableLoadTag(other.mDisableLoadTag),
      mReenterSharedChildren(other.mReenterSharedChildren),
      mRuntimeData{true, false, false, PropArray<GameObjectId>()} {
    mMaskedByScenes._Copy(other.mMaskedByScenes);
    mMaskedByDynamicScenes._Copy(other.mMaskedByDynamicScenes);
}

// Reconstructed from eboot.elf at 0x436480. The deleting destructor is at
// 0x4365C0. The arrays free their storage.
RndSceneInstanceCom::~RndSceneInstanceCom() {}

// Reconstructed from eboot.elf at 0x4365E0.
bool RndSceneInstanceCom::CanUseThreadedPoll() const {
    return mUseThreadedPoll;
}

// Reconstructed from eboot.elf at 0x4365F0. The output resolution is the
// first PS4 resolution.
Hmx::Rect RndSceneInstanceCom::CalcProjectionRect(const Vector2i& size) const {
    const Vector2i& output = *TheRndDevice()->mCapabilities[kPlatformPS4].mResolutions.begin();
    const float targetAspect = static_cast<float>(size.x) / static_cast<float>(size.y);
    const float widthScale =
        static_cast<float>(output.x) / (targetAspect * static_cast<float>(output.y));
    return Hmx::Rect(
        (mProjectionRect.x * -0.5F + 0.5F) + widthScale * mProjectionOffset.x,
        (mProjectionOffset.y + 0.5F) + mProjectionRect.y * -0.5F,
        mProjectionRect.x,
        mProjectionRect.y);
}

// Reconstructed from eboot.elf at 0x436670. The scene must have a child
// whose root holds a RndSceneCom.
PollMgr* RndSceneInstanceCom::GetScenePollMgr() const {
    return mRuntime.mEntity->GetRoot()->GetExistingCom<RndSceneCom>()->mPollMgr;
}

// Reconstructed from eboot.elf at 0x4366C0.
PollGroup* RndSceneInstanceCom::GetSceneDrawerPollGroup() const {
    const Entity* const entity = mRuntime.mEntity;
    if (entity == nullptr || entity->mResource == nullptr) {
        return nullptr;
    }
    return entity->GetRoot()->GetExistingCom<RndSceneCom>()->mPollGroups->mPollGroups.find(0)->second;
}

// Reconstructed from eboot.elf at 0x4366E0.
PollGroup* RndSceneInstanceCom::GetScenePollGroup() const {
    const Entity* const entity = mRuntime.mEntity;
    if (entity == nullptr || entity->mResource == nullptr) {
        return nullptr;
    }
    return entity->GetRoot()->GetExistingCom<RndSceneCom>()->mPollGroups->mPollGroups.find(1)->second;
}

// Reconstructed from eboot.elf at 0x4379C0.
bool RndSceneInstanceCom::_OnResourcesLoaded() {
    if (!InstanceCom::_OnResourcesLoaded()) {
        return false;
    }
    _SyncMaskingScenes();
    return true;
}

// Reconstructed from eboot.elf at 0x4379F0. A scene polls only while it
// shows when its header asks for it and it does not drive its parent.
void RndSceneInstanceCom::_Enter() {
    bool alwaysPolls = true;
    const Entity* const child = mRuntime.mEntity;
    if (child != nullptr) {
        const EntityHeaderCom* const header = child->GetRoot()->GetCom<EntityHeaderCom>();
        if (header != nullptr && !header->mDrivesParent) {
            alwaysPolls = header->mInstancePollingMode == 0;
        }
    }
    mRuntimeData.mAlwaysPolls = alwaysPolls;
    InstanceCom::_Enter();
}

// Reconstructed from eboot.elf at 0x437A70. The object's draw node must
// exist.
void RndSceneInstanceCom::_Poll() {
    if (!mRuntimeData.mAlwaysPolls && mRuntime.mEntity != nullptr) {
        const RndDrawNodeCom* const drawNode = mObject->GetExistingCom<RndDrawNodeCom>();
        mRuntime.mEntity->SetActive((drawNode->mRuntime.mWorldShowHideFlags & 1) == 0);
    }
    if (mRuntimeData.mMaskingScenesDirty) {
        _SyncMaskingScenes();
        mRuntimeData.mMaskingScenesDirty = false;
    }
    InstanceCom::_Poll();
}

// Reconstructed from eboot.elf at 0x437B00.
void RndSceneInstanceCom::_EditPoll() {
    if (mRuntimeData.mReloadPending) {
        LoadResources(false);
        mRuntimeData.mReloadPending = false;
    }
    if (mRuntimeData.mMaskingScenesDirty) {
        _SyncMaskingScenes();
        mRuntimeData.mMaskingScenesDirty = false;
    }
    InstanceCom::_EditPoll();
}

// Reconstructed from eboot.elf at 0x437BC0. The DynamicComResource base's
// thunk (0x437BD0) reads the same flag.
bool RndSceneInstanceCom::_ShouldLoadUniqueResource() const {
    return mUniqueResource;
}

// Reconstructed from eboot.elf at 0x437BE0.
bool RndSceneInstanceCom::_ReenterSharedChildren() const {
    return mReenterSharedChildren;
}

// Reconstructed from eboot.elf at 0x437BF0.
bool RndSceneInstanceCom::_HasParentPollDep() const {
    return false;
}

// Reconstructed from eboot.elf at 0x437C00.
bool RndSceneInstanceCom::_CanBeDrivenByChild() const {
    return false;
}

// Reconstructed from eboot.elf at 0x437C10.
Symbol RndSceneInstanceCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x437C20.
Symbol RndSceneInstanceCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x437C30.
int RndSceneInstanceCom::CurrentRev() const {
    return const_cast<RndSceneInstanceCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x437C50.
bool RndSceneInstanceCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x437C80.
Component* RndSceneInstanceCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x437C90. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndSceneInstanceCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSceneInstanceCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSceneInstanceCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x437F60.
PropRegistry& RndSceneInstanceCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x437F70.
ComMetaData& RndSceneInstanceCom::_GetMetaData() {
    return sMetaData;
}
