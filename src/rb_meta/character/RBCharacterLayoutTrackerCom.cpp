// rb_meta/RBCharacterLayoutTrackerCom.o (0xE5A540 to 0xE5AEFF). The static
// initializer is at 0xE5AE30; the shared header values it sets first
// (0x1B328E8-0x1B328F0) are kept below. The class's registration and
// factory are emitted elsewhere (0xB750E0, 0xBA63C0).
#include "rb_meta/character/RBCharacterLayoutTrackerCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "ui/layout/UILayoutResource.h"
#include "ui/manager/UIMgr.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1B328E8, which the static initializer sets
// with the shared header's thread-group widths. Each object that uses it
// has its own copy. Names not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};
[[maybe_unused]] int gTrackerGroupSize2D = 8;  // 0x1B328EC
[[maybe_unused]] int gTrackerGroupSize3D = 4;  // 0x1B328F0

}  // namespace

// The object's statics, in the order of its static initializer (0xE5AE30).
Symbol RBCharacterLayoutTrackerCom::sId("RBCharacterLayoutTrackerCom");
Symbol RBCharacterLayoutTrackerCom::sClassName("RBCharacterLayoutTrackerCom");
PropRegistry RBCharacterLayoutTrackerCom::sPropRegistry;
ComMetaData RBCharacterLayoutTrackerCom::sMetaData;

// Reconstructed from eboot.elf at 0xE5A540.
RBCharacterLayoutTrackerCom::RBCharacterLayoutTrackerCom()
    : mCharacterSceneObj(nullptr),
      mSceneInstance(nullptr),
      mSceneTracker(nullptr),
      mCharacterSceneObjId(GameObjectId{0xFFFFFFFFU}) {}

// Reconstructed from eboot.elf at 0xE5A580. The deleting destructor is at
// 0xE5A590.
RBCharacterLayoutTrackerCom::~RBCharacterLayoutTrackerCom() {}

// Reconstructed from eboot.elf at 0xE5A5B0. The object must hold a
// SceneInstance.
void RBCharacterLayoutTrackerCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RBCharacterLayoutTrackerCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    metadata.mDescription =
        "Handles layout-level tracking of character entities.  Points at a scene containing characters and an "
        "RBCharacterSceneTrackerCom, and forwards requests on to it.";
    metadata.mAuthor = "Matt Ivester";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(UILayoutResource::Id());

    GameObjectIdMetadata& sceneObject = TypeSpecificMetadata(
        registry.RegisterProp(
            "character_scene_obj_id",
            offsetof(RBCharacterLayoutTrackerCom, mCharacterSceneObjId),
            kPropertyGameObjectId,
            sizeof(GameObjectId)),
        prototype.mCharacterSceneObjId);
    sceneObject.mDefault = prototype.mCharacterSceneObjId;
    static_cast<void>(Symbol("prop"));
    sceneObject.mDisplayName = String("Character Scene Object ID");
    sceneObject.mHelp =
        String("This should point at the GameObject containing the SceneInstance where the characters are located.");
    sceneObject.mRequiredComs = {RndSceneInstanceCom::sId};
}

// Reconstructed from eboot.elf at 0xE5A980. The component goes on without
// the object.
bool RBCharacterLayoutTrackerCom::_OnResourcesLoaded() {
    if (mCharacterSceneObjId.mId == gNullObjectId.mId) {
        mCharacterSceneObj = nullptr;
        static_cast<void>(MakeErrorName());
        return true;
    }
    mCharacterSceneObj = mObject->mEntity->GetObject(mCharacterSceneObjId);
    if (mCharacterSceneObj == nullptr) {
        static_cast<void>(MakeErrorName());
        return true;
    }
    mSceneInstance = mCharacterSceneObj->GetCom<RndSceneInstanceCom>();
    return true;
}

// Reconstructed from eboot.elf at 0xE5AA20. A missing scene or tracker
// leaves only the release build's error name.
void RBCharacterLayoutTrackerCom::_Enter() {
    if (mSceneInstance == nullptr || mSceneInstance->mRuntime.mEntity == nullptr) {
        static_cast<void>(MakeErrorName());
        return;
    }
    mSceneTracker = mSceneInstance->mRuntime.mEntity->GetRoot()->GetCom<RBCharacterSceneTrackerCom>();
    if (mSceneTracker == nullptr) {
        static_cast<void>(MakeErrorName());
    }
}

// Reconstructed from eboot.elf at 0xE5AAA0.
bool RBCharacterLayoutTrackerCom::AnyPlayerEntityIsLoading() const {
    return mSceneTracker != nullptr ? mSceneTracker->AnyPlayerEntityIsLoading() : false;
}

// Reconstructed from eboot.elf at 0xE5AAC0.
Entity* RBCharacterLayoutTrackerCom::GetPlayerEntityByInstrument(const MusicianType::Instrument& instrument) const {
    return mSceneTracker != nullptr ? mSceneTracker->GetPlayerEntityByInstrument(instrument) : nullptr;
}

// Reconstructed from eboot.elf at 0xE5AAE0.
Entity* RBCharacterLayoutTrackerCom::GetPlayerEntityForUser(const BandUser* user) const {
    return mSceneTracker != nullptr ? mSceneTracker->GetPlayerEntityForUser(user) : nullptr;
}

// Reconstructed from eboot.elf at 0xE5AB00. The empty path without a
// tracker.
ResourcePath RBCharacterLayoutTrackerCom::GetPlayerEntityPathByInstrument(
    const MusicianType::Instrument& instrument) const {
    if (mSceneTracker != nullptr) {
        return mSceneTracker->GetPlayerEntityPathByInstrument(instrument);
    }
    ResourcePath path;
    path = "";
    return path;
}

// Reconstructed from eboot.elf at 0xE5AB70.
ResourcePath RBCharacterLayoutTrackerCom::GetPlayerEntityPathForUser(const BandUser* user) const {
    if (mSceneTracker != nullptr) {
        return mSceneTracker->GetPlayerEntityPathForUser(user);
    }
    ResourcePath path;
    path = "";
    return path;
}

// Reconstructed from eboot.elf at 0xE5ABE0.
RBCharacterLayoutTrackerCom* RBCharacterLayoutTrackerCom::GetForCurrentLayout() {
    const UILayoutResource* const layout = theUI->GetCurrentLayout();
    if (layout == nullptr) {
        return nullptr;
    }
    return layout->mEntity->GetRoot()->GetCom<RBCharacterLayoutTrackerCom>();
}

// Reconstructed from eboot.elf at 0xE5AC50.
Symbol RBCharacterLayoutTrackerCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xE5AC60.
Symbol RBCharacterLayoutTrackerCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xE5AC70.
int RBCharacterLayoutTrackerCom::CurrentRev() const {
    return const_cast<RBCharacterLayoutTrackerCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xE5AC90.
bool RBCharacterLayoutTrackerCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xE5ACC0.
Component* RBCharacterLayoutTrackerCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xE5ACD0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBCharacterLayoutTrackerCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy =
        reinterpret_cast<RBCharacterLayoutTrackerCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBCharacterLayoutTrackerCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xE5AE10.
PropRegistry& RBCharacterLayoutTrackerCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xE5AE20.
ComMetaData& RBCharacterLayoutTrackerCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBA63C0. The binary emits the factory
// with the other rb_meta factories.
Component* RBCharacterLayoutTrackerCom::_Create() {
    return new RBCharacterLayoutTrackerCom();
}

// Reconstructed from eboot.elf at 0xB750E0. The binary emits the
// registration with the other rb_meta classes'.
void RBCharacterLayoutTrackerCom::Init() {
    InitComponentClass<RBCharacterLayoutTrackerCom>();
}
