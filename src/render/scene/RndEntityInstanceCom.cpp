// render/RndEntityInstanceCom.o (0x6CAE80 to 0x6CBF3F). The poll (0x6CB580)
// is not reconstructed.
#include "render/scene/RndEntityInstanceCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityHeaderCom.h"
#include "entity/core/InstanceCom.h"
#include "entity/core/TransCom.h"
#include "render/context/RndCameraCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"

// The object's statics, in the order of its static initializer (0x6CBE70).
// The -1, 8 and 4 it first sets (0x1AB0D38) come from a shared header and
// are not modelled.
Symbol RndEntityInstanceCom::sId("RndEntityInstance");
Symbol RndEntityInstanceCom::sClassName("RndEntityInstance");
PropRegistry RndEntityInstanceCom::sPropRegistry;
ComMetaData RndEntityInstanceCom::sMetaData;

// Reconstructed from eboot.elf at 0x6CAE80.
RndEntityInstanceCom::RndEntityInstanceCom() : mRuntime() {}

// Reconstructed from eboot.elf at 0x6CAEB0. The instance component and the
// draw node are left unset until _Enter.
RndEntityInstanceCom::RuntimeData::RuntimeData() : mPollOnlyWhenShowing(false), mPollOnlyOnScreen(false) {}

// Reconstructed from eboot.elf at 0x6CAEC0.
void RndEntityInstanceCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    static_cast<void>(registry);
    metadata.mDescription = "Manages renderer-specific aspects of InstanceCom behavior.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mRequiredComponents.push_back(InstanceCom::sClassName);
    metadata.mEditorRestrictions = 0;
    metadata.mLightweight = true;
}

// Reconstructed from eboot.elf at 0x6CB100.
void RndEntityInstanceCom::_PreDestroy(DestroyType type) {
    if (type != kDestroyComponent) {
        return;
    }
    RndDrawNodeCom* const drawNode = mObject->GetCom<RndDrawNodeCom>();
    if (drawNode == nullptr) {
        return;
    }
    const TransCom* const trans = mObject->GetCom<TransCom>();
    if (trans == nullptr) {
        return;
    }
    Sphere sphere;
    sphere.center = trans->mWorldXfm.v;
    sphere.radius = 0.0F;
    drawNode->SetLocalSphere(sphere);
}

// Reconstructed from eboot.elf at 0x6CB1E0. A static instanced entity always
// polls. An entity that drives its parent takes the enclosing instance's
// mode; one inside a scene instance that does not always poll polls only
// while it shows.
void RndEntityInstanceCom::_Enter() {
    InstanceCom* const instance = mObject->GetBaseCom<InstanceCom>();
    mRuntime.mInstance = instance;
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    bool whenShowing = false;
    bool onScreen = false;
    Entity* const child = instance->mRuntime.mEntity;
    if (child != nullptr) {
        if (IsStaticEntity(child)) {
            mRuntime.mPollOnlyWhenShowing = false;
            mRuntime.mPollOnlyOnScreen = false;
            return;
        }
        const EntityHeaderCom* const header = child->GetRoot()->GetCom<EntityHeaderCom>();
        if (header != nullptr && !header->mDrivesParent) {
            whenShowing = header->mInstancePollingMode != 0;
            onScreen = header->mInstancePollingMode == 2;
        }
    }
    mRuntime.mPollOnlyWhenShowing = whenShowing;
    mRuntime.mPollOnlyOnScreen = onScreen;
    const GameObject* const parent = mObject->mEntity->mParentObject;
    if (parent == nullptr) {
        return;
    }
    const RndEntityInstanceCom* const outer = parent->GetCom<RndEntityInstanceCom>();
    if (outer != nullptr) {
        mRuntime.mPollOnlyWhenShowing = whenShowing || outer->mRuntime.mPollOnlyWhenShowing;
        mRuntime.mPollOnlyOnScreen = onScreen || outer->mRuntime.mPollOnlyOnScreen;
        return;
    }
    const RndSceneInstanceCom* const scene = parent->GetCom<RndSceneInstanceCom>();
    if (scene != nullptr) {
        mRuntime.mPollOnlyWhenShowing = whenShowing || !scene->mRuntimeData.mAlwaysPolls;
    }
}

// Reconstructed from eboot.elf at 0x6CB410.
void RndEntityInstanceCom::_GetComponentOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    follows.push_back(RndDrawNodeCom::sClassName);
    precedes.push_back(InstanceCom::sClassName);
}

// Reconstructed from eboot.elf at 0x6CB9F0.
void RndEntityInstanceCom::_GetPollDeps(eastl::vector<GameObjectId>& before, eastl::vector<GameObjectId>& after) {
    static_cast<void>(after);
    RndSceneCom* const scene = mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    if (scene == nullptr || !_HasCameraDependency()) {
        return;
    }
    const RndCameraCom* const camera = scene->GetCamera(0);
    if (camera != nullptr) {
        before.push_back(camera->mObject->mId);
    }
}

// Reconstructed from eboot.elf at 0x6CBB40. The search is recursive through
// the instanced entity's own entity instances.
bool RndEntityInstanceCom::_HasCameraDependency() const {
    const Entity* const child = mObject->GetExistingBaseCom<InstanceCom>()->mRuntime.mEntity;
    if (child == nullptr) {
        return false;
    }
    const EntityHeaderCom* const header = child->GetRoot()->GetExistingCom<EntityHeaderCom>();
    if (!header->mDrivesParent && header->mInstancePollingMode == 2) {
        return true;
    }
    for (const GameObject* object = child->NextObject(nullptr, sId); object != nullptr;
         object = child->NextObject(object, sId)) {
        if (object->GetCom<RndEntityInstanceCom>()->_HasCameraDependency()) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6CBC70, which jumps to Component's. The
// deleting destructor is at 0x6CBC80.
RndEntityInstanceCom::~RndEntityInstanceCom() {}

// Reconstructed from eboot.elf at 0x6CBCA0.
Symbol RndEntityInstanceCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6CBCB0.
Symbol RndEntityInstanceCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6CBCC0.
int RndEntityInstanceCom::CurrentRev() const {
    return const_cast<RndEntityInstanceCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6CBCE0.
bool RndEntityInstanceCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6CBD10.
Component* RndEntityInstanceCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6CBD20. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndEntityInstanceCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndEntityInstanceCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndEntityInstanceCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6CBE50.
PropRegistry& RndEntityInstanceCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6CBE60.
ComMetaData& RndEntityInstanceCom::_GetMetaData() {
    return sMetaData;
}
