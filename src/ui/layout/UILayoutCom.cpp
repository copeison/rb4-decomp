// The "UILayout" component's object (0x8A9340 to 0x8AAA3F). The
// registration and the factory are emitted in ui/UI.o.
#include "ui/layout/UILayoutCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "ui/layout/UILayoutResource.h"
#include "utl/files/FileUtl.h"
#include "utl/text/Str.h"
#include "utl/threading/PollMgr.h"

namespace {

// Removes the first element equal to the id, as the binary's inlined
// PropArray erase does: destroys it and moves the later elements down
// through the array's vtable. Name not in the reference map.
void EraseId(PropArray<GameObjectId>& array, GameObjectId id) {
    for (unsigned long i = 0; i < array.mSize; ++i) {
        if (array.data()[i].mId != id.mId) {
            continue;
        }
        array.Destruct(1, array.ElementAt(i));
        if (static_cast<unsigned int>(i) < array.mSize - 1) {
            array.Move(array.mSize - 1 - i, array.ElementAt(i), array.ElementAt(i + 1));
        }
        --array.mSize;
        return;
    }
}

// The change callback of "poll_order"'s scenes (0x8AA850): the entity
// rebuilds its poll order. Name not in the reference map.
void OnPollOrderChanged(const PropAccessorArgs& args) {
    args.mComponent->mObject->mEntity->mFlags |= 0x400;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8AA970).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol UILayoutCom::sId("UILayout");
Symbol UILayoutCom::sClassName("UILayout");
PropRegistry UILayoutCom::sPropRegistry;
ComMetaData UILayoutCom::sMetaData;

// Reconstructed from eboot.elf at 0x8A9400.
UILayoutCom::UILayoutCom()
    : mPollOrder(),
      mDrawOrder(),
      mHmdMirrorOverlays(),
      mPollMgr(nullptr),
      mPollGroup(nullptr),
      mScenePollGroups(nullptr),
      mThreadedScenes(),
      mStartPollGroups(),
      mReadyScenes(),
      mPollRangeReleased(false) {}

// Reconstructed from eboot.elf at 0x8A9600. _DestroyPollGroups is inlined.
// The deleting destructor is at 0x8A98B0.
UILayoutCom::~UILayoutCom() {
    _DestroyPollGroups();
    delete mPollMgr;
    mPollMgr = nullptr;
}

// Reconstructed from eboot.elf at 0x8A9830.
void UILayoutCom::_DestroyPollGroups() {
    if (mScenePollGroups != nullptr) {
        mScenePollGroups->Reset();
    }
    if (mPollGroup != nullptr) {
        mPollGroup->Reset();
    }
    delete mScenePollGroups;
    mScenePollGroups = nullptr;
    delete mPollGroup;
    mPollGroup = nullptr;
}

// Reconstructed from eboot.elf at 0x8A98D0.
void UILayoutCom::_ObtainPollMgr() {
    const String name(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    if (mPollMgr == nullptr) {
        mPollMgr = new PollMgr(name.c_str());
    }
}

// Reconstructed from eboot.elf at 0x8A9960.
void UILayoutCom::_CreatePollGroups() {
    const String name(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    mScenePollGroups = new RndSceneCom::PollGroups(name.c_str());
    mPollGroup = new PollGroup(name.c_str());
}

// Reconstructed from eboot.elf at 0x8A9B00.
void UILayoutCom::RemoveSceneObject(GameObjectId id) {
    EraseId(mPollOrder, id);
    EraseId(mDrawOrder, id);
    EraseId(mHmdMirrorOverlays, id);
}

// Reconstructed from eboot.elf at 0x8A9E30. Each order lists scene
// instances; the HMD mirror overlays are hidden.
void UILayoutCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndDrawableEntityCom::_InitAsSuperclass(registry, metadata);
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mDescription = "Layout-wide rendering settings";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mEditorRestrictions = 0;
    metadata.mAllowedResources.push_back(UILayoutResource::Id());
    UILayoutCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    const GameObjectId sceneType = {0xFFFFFFFFU};
    PropInfo& pollOrder = registry.RegisterPropArray(
        "poll_order", offsetof(UILayoutCom, mPollOrder), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& pollOrderList = static_cast<ArrayMetadata&>(*pollOrder.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    pollOrderList.mHelp = String(
        "The order in which my scenes are polled (also the reverse-order in which they handle input)");
    PropRegistry::RegisterPropArrayItem(pollOrder, kPropertyGameObjectId, sizeof(GameObjectId));
    GameObjectIdMetadata& pollScene = TypeSpecificMetadata(pollOrderList.mItemInfo, sceneType);
    static_cast<void>(Symbol("item"));
    pollScene.mRequiredComs = {RndSceneInstanceCom::sId};
    pollScene.mOnChanged = OnPollOrderChanged;

    PropInfo& drawOrder = registry.RegisterPropArray(
        "draw_order", offsetof(UILayoutCom, mDrawOrder), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& drawOrderList = static_cast<ArrayMetadata&>(*drawOrder.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    drawOrderList.mDisplayName = String("Draw Order");
    drawOrderList.mHelp = String("The order in which my scenes are drawn");
    PropRegistry::RegisterPropArrayItem(drawOrder, kPropertyGameObjectId, sizeof(GameObjectId));
    GameObjectIdMetadata& drawScene = TypeSpecificMetadata(drawOrderList.mItemInfo, sceneType);
    static_cast<void>(Symbol("item"));
    drawScene.mRequiredComs = {RndSceneInstanceCom::sId};

    PropInfo& overlays = registry.RegisterPropArray(
        "hmd_mirror_overlays", offsetof(UILayoutCom, mHmdMirrorOverlays), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& overlayList = static_cast<ArrayMetadata&>(*overlays.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    overlayList.mDisplayName = String("HMD Mirror Overlays");
    overlayList.mHelp = String(
        "Scenes which will only draw as overlays on top of the HMD mirror (in the order we draw them)");
    PropRegistry::RegisterPropArrayItem(overlays, kPropertyGameObjectId, sizeof(GameObjectId));
    GameObjectIdMetadata& overlayScene = TypeSpecificMetadata(overlayList.mItemInfo, sceneType);
    static_cast<void>(Symbol("item"));
    overlayScene.mRequiredComs = {RndSceneInstanceCom::sId};
    overlayList.mIsHidden._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return true;
    });
}

// Reconstructed from eboot.elf at 0x8AA510.
Symbol UILayoutCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8AA520.
Symbol UILayoutCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8AA530.
int UILayoutCom::CurrentRev() const {
    return const_cast<UILayoutCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8AA550.
bool UILayoutCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8AA580.
Component* UILayoutCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8AA590. The copy goes at the buffer's
// next 8-byte boundary.
char* UILayoutCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UILayoutCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UILayoutCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8AA830.
PropRegistry& UILayoutCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8AA840.
ComMetaData& UILayoutCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x895070. The binary emits the factory
// in ui/UI.o.
Component* UILayoutCom::_Create() {
    return new UILayoutCom();
}

// Reconstructed from eboot.elf at 0x892360. The binary emits the
// registration in ui/UI.o.
void UILayoutCom::Init() {
    InitComponentClass<UILayoutCom>();
}
