// render/RndSceneInstanceCom.o (0x4362E0 to 0x438200). The masking scenes
// (0x436710 and its callers 0x436700 and 0x436A80) and the poll
// dependencies (0x437B50) are not reconstructed.
#include "render/scene/RndSceneInstanceCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityHeaderCom.h"
#include "entity/props/PropMetadata.h"
#include "math/vector/Vector2i.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndCapabilities.h"
#include "render/system/RndDevice.h"

namespace {

// The superclass registration and the property callbacks _Init installs.
// Names not in the reference map.
void InitInstanceComAsSuperclass(PropRegistry& registry, ComMetaData& metadata);
void OnUniqueResourceChanged(const PropAccessorArgs& args);
void OnMaskedByScenesChanged(const PropAccessorArgs& args);
void OnMaskedByDynamicScenesChanged(const PropAccessorArgs& args);

}  // namespace

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

// Reconstructed from eboot.elf at 0x436AB0. InstanceCom's registration
// comes first (0x437910); the file property's metadata is looked up by its
// path, once to no effect and once to load scenes. "use_threaded_poll" is
// read-only while the entity runs in the game mode. The members of
// "projection_offset" and "projection_rect" take zero as their defaults,
// not the component's.
void RndSceneInstanceCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    InitInstanceComAsSuperclass(registry, metadata);
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mDescription = "Scene instance settings";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(Symbol("UILayoutResource"));
    metadata.mInterface = InstanceCom::sClassName;
    metadata.mLightweight = false;
    RndSceneInstanceCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    static_cast<void>(registry.FindProp(PropPath(Symbol("file")), nullptr));
    const ResourcePath fileType;
    ResourcePathMetadata& file = TypeSpecificMetadata(
        *const_cast<PropInfo*>(registry.FindProp(PropPath(Symbol("file")), nullptr)), fileType);
    static_cast<void>(Symbol("prop"));
    file.mResourceId = RndSceneResource::Id();
    file.mAnimatable = false;

    IntMetadata<int>& drawOrder = TypeSpecificMetadata(
        registry.RegisterProp("draw_order", offsetof(RndSceneInstanceCom, mDrawOrder), kPropertyInt, 4),
        prototype.mDrawOrder);
    drawOrder.mDefault = prototype.mDrawOrder;
    static_cast<void>(Symbol("prop"));
    drawOrder.mDisplayName = String("old draw order");
    drawOrder.mHelp = String(
        "The draw order for this scene within a layout. Currently this works by changing poll order, and "
        "should be split!");
    drawOrder.mDebug = true;
    drawOrder.mReadOnly = true;

    BoolMetadata& uniqueResource = TypeSpecificMetadata(
        registry.RegisterProp(
            "unique_resource", offsetof(RndSceneInstanceCom, mUniqueResource), kPropertyBool, 1),
        prototype.mUniqueResource);
    uniqueResource.mDefault = prototype.mUniqueResource;
    static_cast<void>(Symbol("prop"));
    uniqueResource.mHelp = String("Should I load a completely unique copy of this scene?");
    uniqueResource.mOnChanged = OnUniqueResourceChanged;

    BoolMetadata& threadedPoll = TypeSpecificMetadata(
        registry.RegisterProp(
            "use_threaded_poll", offsetof(RndSceneInstanceCom, mUseThreadedPoll), kPropertyBool, 1),
        prototype.mUseThreadedPoll);
    threadedPoll.mDefault = prototype.mUseThreadedPoll;
    static_cast<void>(Symbol("prop"));
    threadedPoll.mHelp =
        String("If checked, the UILayout will attempt to poll this scene in parallel with other scenes");
    threadedPoll.mAnimatable = false;
    threadedPoll.mReadOnly._Set([](const TagArgs& args) {
        return (args.mComponent.mObject->mEntity->mFlags & 0x16) == 0x12;
    });

    BoolMetadata& reenter = TypeSpecificMetadata(
        registry.RegisterProp(
            "reenter_shared_children",
            offsetof(RndSceneInstanceCom, mReenterSharedChildren),
            kPropertyBool,
            1),
        prototype.mReenterSharedChildren);
    reenter.mDefault = prototype.mReenterSharedChildren;
    static_cast<void>(Symbol("prop"));
    reenter.mHelp = String("If this scene is NOT copy-on-instance, will it be exited/re-entered?");

    GroupMetadata& offset = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "projection_offset", offsetof(RndSceneInstanceCom, mProjectionOffset), kPropertyStruct, sizeof(Vector2))
        .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Vector2 offsetType = {0.0F, 0.0F};
    offset.mHelp = String("Normalized offset of the final projected image of this scene");
    FloatMetadata& offsetX = TypeSpecificMetadata(
        offset.mRegistry.RegisterProp("x", offsetof(Vector2, x), kPropertyFloat, 4), offsetType.x);
    offsetX.mDefault = offsetType.x;
    static_cast<void>(Symbol("prop"));
    offsetX.mMin = -1.0F;
    offsetX.mMax = 1.0F;
    offsetX.mSlider = true;
    FloatMetadata& offsetY = TypeSpecificMetadata(
        offset.mRegistry.RegisterProp("y", offsetof(Vector2, y), kPropertyFloat, 4), offsetType.y);
    offsetY.mDefault = offsetType.y;
    static_cast<void>(Symbol("prop"));
    offsetY.mMin = -1.0F;
    offsetY.mMax = 1.0F;
    offsetY.mSlider = true;

    PropRegistry& rect = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "projection_rect", offsetof(RndSceneInstanceCom, mProjectionRect), kPropertyStruct, sizeof(Vector2))
        .mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    const Vector2 rectType = {0.0F, 0.0F};
    FloatMetadata& width =
        TypeSpecificMetadata(rect.RegisterProp("width", offsetof(Vector2, x), kPropertyFloat, 4), rectType.x);
    width.mDefault = rectType.x;
    static_cast<void>(Symbol("prop"));
    width.mMin = 0.01F;
    width.mMax = 2.0F;
    width.mSlider = true;
    FloatMetadata& height =
        TypeSpecificMetadata(rect.RegisterProp("height", offsetof(Vector2, y), kPropertyFloat, 4), rectType.y);
    height.mDefault = rectType.y;
    static_cast<void>(Symbol("prop"));
    height.mMin = 0.01F;
    height.mMax = 2.0F;
    height.mSlider = true;

    PropInfo& maskedBy = registry.RegisterPropArray(
        "masked_by_scenes",
        offsetof(RndSceneInstanceCom, mMaskedByScenes),
        sizeof(PropArray<GameObjectId>));
    ArrayMetadata& maskedByMetadata = static_cast<ArrayMetadata&>(*maskedBy.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    maskedByMetadata.mHelp = String("Scenes that mask out portions of me (it must draw after me)");
    PropRegistry::RegisterPropArrayItem(maskedBy, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId sceneType = {static_cast<unsigned int>(-1)};
    GameObjectIdMetadata& scene = TypeSpecificMetadata(maskedByMetadata.mItemInfo, sceneType);
    static_cast<void>(Symbol("item"));
    scene.mRequiredComs = {sId};
    scene.mAllowsDuplicates = false;
    scene.mAllowsThis = false;
    scene.mOnChanged = OnMaskedByScenesChanged;

    PropInfo& maskedByDynamic = registry.RegisterPropArray(
        "masked_by_dynamic_scenes",
        offsetof(RndSceneInstanceCom, mMaskedByDynamicScenes),
        sizeof(PropArray<ResourcePath>));
    ArrayMetadata& maskedByDynamicMetadata = static_cast<ArrayMetadata&>(*maskedByDynamic.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    maskedByDynamicMetadata.mHelp = String(
        "Scene paths for dynamically-loaded scenes that mask out portions of me (it must draw after me)");
    PropRegistry::RegisterPropArrayItem(maskedByDynamic, kPropertyResourcePath, sizeof(ResourcePath));
    const ResourcePath pathType;
    ResourcePathMetadata& path = TypeSpecificMetadata(maskedByDynamicMetadata.mItemInfo, pathType);
    static_cast<void>(Symbol("item"));
    path.mResourceId = RndSceneResource::Id();
    path.mOnChanged = OnMaskedByDynamicScenesChanged;

    SymbolMetadata& disableLoadTag = TypeSpecificMetadata(
        registry.RegisterProp(
            "disable_load_tag", offsetof(RndSceneInstanceCom, mDisableLoadTag), kPropertySymbol, 8),
        prototype.mDisableLoadTag);
    disableLoadTag.mDefault = prototype.mDisableLoadTag;
    static_cast<void>(Symbol("prop"));
    disableLoadTag.mHelp = String(
        "If the tag specified is in your (ui (disable_scene_loading)) config block and not in the editor, the "
        "scene will not be loaded. This is a DEBUG property only and disabled for SHIP.");
    disableLoadTag.mDebug = true;
}

namespace {

// Reconstructed from eboot.elf at 0x437910. As
// InstanceCom::_InitAsSuperclass (0x118DC0), with the class's own file
// property "file".
void InitInstanceComAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("InstanceCom", InstanceCom::sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    InstanceCom::_Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

}  // namespace

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

namespace {

// Reconstructed from eboot.elf at 0x437FB0.
void OnUniqueResourceChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneInstanceCom*>(args.mComponent)->mRuntimeData.mReloadPending = true;
}

// Reconstructed from eboot.elf at 0x437FC0.
void OnMaskedByScenesChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneInstanceCom*>(args.mComponent)->mRuntimeData.mMaskingScenesDirty = true;
}

// Reconstructed from eboot.elf at 0x437FD0.
void OnMaskedByDynamicScenesChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneInstanceCom*>(args.mComponent)->mRuntimeData.mMaskingScenesDirty = true;
}

}  // namespace

// Reconstructed from eboot.elf at 0x404C10. The binary emits the factory in
// render/RndInit.o with the class's Init.
Component* RndSceneInstanceCom::_Create() {
    return new RndSceneInstanceCom();
}

// Reconstructed from eboot.elf at 0x3F2800. The binary emits the
// registration in render/RndInit.o.
void RndSceneInstanceCom::Init() {
    InitComponentClass<RndSceneInstanceCom>();
}
