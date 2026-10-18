// render/RndSceneCom.o (0x407F30 to 0x419170).
#include "render/scene/RndSceneCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropUtl.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/profiling/PerfMgr.h"
#include "render/atmosphere/RndAtmosphereCom.h"
#include "render/atmosphere/RndSkyCom.h"
#include "render/context/RndCameraCom.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/volumetric/RndVolumetricScatteringCom.h"
#include "render/postprocessing/antialiasing/RndCMAACom.h"
#include "render/postprocessing/chain/RndPostProcCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/system/RndUtl.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureUtl.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/threading/PollMgr.h"
#include "utl/time/TimeMgr.h"
#include "utl/time/TimelinesCom.h"

namespace {

// The id of no object. The binary reads it from a shared header's global
// (0x1A72288), which the object's static initializer sets to -1. Name not
// in the reference map.
constexpr unsigned int kNoObject = 0xFFFFFFFFU;

// Whether the renderer draws scenes at partial frame rates: the settings
// allow some, enable them, and use tiled lighting. Inlined into every
// user. Name not in the reference map.
bool PartialFramerateActive() {
    const RndConfig& settings = *TheRndDevice()->mSettings;
    return settings.mMaxPartialFramerateScenes != 0 && settings.mPartialFramerateEnabled &&
        settings.mUseTiledLighting;
}

GameObjectId ObjectIdOf(const Component* component) {
    return component != nullptr ? component->mObject->mId : GameObjectId{kNoObject};
}

// A path of up to four member names, as the revision handler builds them
// with PropPath's node constructor (0x17E8B0). Name not in the reference
// map.
PropPath MemberPath(const char* name0, const char* name1, const char* name2, const char* name3 = nullptr) {
    PropPath::Node node0;
    PropPath::Node node1;
    PropPath::Node node2;
    PropPath::Node node3;
    node0 = Symbol(name0);
    node1 = Symbol(name1);
    node2 = Symbol(name2);
    if (name3 != nullptr) {
        node3 = Symbol(name3);
    }
    return PropPath(node0, node1, node2, node3);
}

// Reads a property of the saved properties the revision handler receives
// into `value` as the type; false when it is missing or cannot be read.
// Inlined into the revision handler. Name not in the reference map.
bool ReadSavedProp(const Component& saved, const PropPath& path, void* value, PropertyType type) {
    void* storage = const_cast<Component*>(&saved);
    const PropInfo* const info = const_cast<Component&>(saved)._GetPropRegistry().FindProp(path, &storage);
    return info != nullptr && _GetProp(saved, path, *info, storage, value, type);
}

// ReadSavedProp, with the warnings of the release build: the error name and
// the path's text are formatted for a notification that was compiled out.
// Name not in the reference map.
void ReadSavedPropOrWarn(const Component& saved, const PropPath& path, void* value, PropertyType type) {
    void* storage = const_cast<Component*>(&saved);
    const PropInfo* const info = const_cast<Component&>(saved)._GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr) {
        static_cast<void>(saved.MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    } else if (!_GetProp(saved, path, *info, storage, value, type)) {
        static_cast<void>(saved.MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
}

// The property callbacks _Init installs (0x412A00 to 0x412C80). Names not
// in the reference map.
bool GetFramerateDeprecated(const PropAccessorArgs& args);
void OnDefaultValueXChanged(const PropAccessorArgs& args);
void OnDefaultValueYChanged(const PropAccessorArgs& args);
void OnDefaultValueZChanged(const PropAccessorArgs& args);
void OnDefaultValueWChanged(const PropAccessorArgs& args);
void OnProp0Changed(const PropAccessorArgs& args);
void OnProp1Changed(const PropAccessorArgs& args);
void OnProp2Changed(const PropAccessorArgs& args);
void OnProp3Changed(const PropAccessorArgs& args);
void InsertPropHysteresisEntry(const PropAccessorArgs& args);
void RemovePropHysteresisEntry(const PropAccessorArgs& args);
void OnTextureSizeChanged(const PropAccessorArgs& args);
void OnWrapModeChanged(const PropAccessorArgs& args);
void OnFilterModeChanged(const PropAccessorArgs& args);
void OnMediumLodDistanceChanged(const PropAccessorArgs& args);
void OnLowLodDistanceChanged(const PropAccessorArgs& args);

// The saved properties a revision handler reads: a component of the old
// layout, the third context pointer of the arguments (weak evidence). Name
// not in the reference map.
const Component& SavedProps(const PropRegistry::RevisionArgs& args) {
    return *args.mSavedProps;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x4190A0).
// The three ints it first sets (0x1A72288) come from a shared header and are
// not modelled.
Symbol RndSceneCom::sId("Scene");
Symbol RndSceneCom::sClassName("Scene");
PropRegistry RndSceneCom::sPropRegistry;
ComMetaData RndSceneCom::sMetaData;

// Reconstructed from eboot.elf at 0x408600.
void RndSceneCom::ClearPollSplitJobs() {
    for (PollSplitJobEntry& entry : mPollSplitJobs) {
        PollGroup* const group = entry.mPollGroup;
        group->Reset();
        delete group;
        delete entry.mJob;
    }
    mPollSplitJobs.mpEnd = mPollSplitJobs.mpBegin;
}

// Reconstructed from eboot.elf at 0x4080E0.
RndSceneCom::RndSceneCom()
    : mDrawOrder(0),
      mClearType(0),
      mClearColor(Hmx::Color::GetBlack()),
      mCameras{{kNoObject}, {kNoObject}},
      mLightMgr{kNoObject},
      mSky{kNoObject},
      mAtmosphere{kNoObject},
      mPostProcs{{kNoObject}, {kNoObject}},
      mAntialiasing{kNoObject},
      mMaskBuffer{false, true, 6},
      mShaderGraphTrans{{kNoObject}, {kNoObject}, {kNoObject}, {kNoObject}},
      mHighLodDistance(0.0F),
      mMediumLodDistance(200.0F),
      mLowLodDistance(1000.0F),
      mEnableLod(false),
      mNumDrawEntries(0),
      mNeverCull(false),
      mPollMgr(nullptr),
      mPollGroups(nullptr),
      mSceneDrawer(nullptr),
      mFrameInterval(static_cast<unsigned long>(-1)),
      mPartialFramerateScene(0),
      mIsolateLod(-1),
      mIntervalStartFrame(-1),
      mPropHysteresisTexture(nullptr),
      mPropHysteresisPosition(0.0F),
      mEntered(false),
      mTimelines(nullptr) {
    mClearDepthAfterPostProc[0] = false;
    mClearDepthAfterPostProc[1] = false;
}

// Reconstructed from eboot.elf at 0x4083D0. The deleting destructor is at
// 0x4087F0.
RndSceneCom::~RndSceneCom() {
    ClearPollSplitJobs();
    if (mPollGroups != nullptr) {
        mPollGroups->Reset();
        delete mPollGroups;
    }
    mPollGroups = nullptr;
    delete mPollMgr;
    mPollMgr = nullptr;
    delete mSceneDrawer;
    mSceneDrawer = nullptr;
    delete mPropHysteresisTexture;
    mPropHysteresisTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x408810.
void RndSceneCom::AddPollSplitJob(
    const char* name,
    int pollWindowStart,
    int pollWindowEnd,
    unsigned int weight) {
    Entity* entity = mObject->mEntity;
    const int index = static_cast<int>(mPollSplitJobs.size());
    PollSplitJob* job =
        new PollSplitJob(entity->mResource, entity, index, pollWindowStart, pollWindowEnd);
    const char* timerName =
        (FormatString("poll-split-job: %d") << static_cast<unsigned int>(index)).Str();
    job->mTimerIndex = thePerfMgr.GetTimerIndex(Symbol(timerName));
    PollGroup* const group = new PollGroup((FormatString("%s PG") << name).Str());
    group->mShareParentGroup = true;
    job->SetPollWeight(weight, false);
    mPollSplitJobs.push_back(PollSplitJobEntry{job, group});
}

// Reconstructed from eboot.elf at 0x408A80.
void RndSceneCom::LinkPollSplitJobs() {
    for (unsigned long i = 0; i < mPollSplitJobs.size(); ++i) {
        mPollSplitJobs[i].mPollGroup->AddJob(mPollSplitJobs[i].mJob);
        if (i != 0) {
            mPollSplitJobs[i].mPollGroup->PollAfter(mPollSplitJobs[i - 1].mPollGroup);
        }
    }
}

// Reconstructed from eboot.elf at 0x408B10.
void RndSceneCom::CreatePollGroups(Entity* entity, bool root) {
    mPollGroups = new PollGroups(FileGetName(entity->mResource->mPath.Str()));
    if (root) {
        return;
    }
    mPollGroups->mPollGroups.find(1)->second->RemovePollAfter(mPollGroups->mPollGroups.find(0)->second);
}

// Reconstructed from eboot.elf at 0x408C10.
int RndSceneCom::GetFramerateType() const {
    if (!PartialFramerateActive()) {
        return 0;
    }
    const TimeMgr::Clock* clock =
        mTimelines != nullptr ? &mTimelines->mClock : TheTimeMgr->GetClock(mObject->mEntity);
    return clock->mDeltaMode;
}

// Reconstructed from eboot.elf at 0x408CD0. A frame-rate type past half
// rate has no interval; the division is by zero then, as in the binary.
unsigned long RndSceneCom::StepFrameInterval() {
    unsigned long frames = 1;
    if (PartialFramerateActive()) {
        const int type = GetFramerateType();
        frames = type == 0 ? 1 : (type == 1 ? 2 : 0);
    }
    const unsigned long next = mFrameInterval + 1;
    mFrameInterval = next % frames;
    return next / frames;
}

// Reconstructed from eboot.elf at 0x408D70.
void RndSceneCom::SetCamera(int index, RndCameraCom* camera) {
    mCameras[index] = ObjectIdOf(camera);
}

namespace {

// The component of class T on the scene entity's object with the id, or
// null. Inlined into each getter. Name not in the reference map.
template <typename T>
T* FindSceneCom(const Component* scene, GameObjectId id) {
    if (id.mId == kNoObject) {
        return nullptr;
    }
    const GameObject* object = scene->mObject->mEntity->GetObject(id);
    return object != nullptr ? object->GetCom<T>() : nullptr;
}

}  // namespace

// Reconstructed from eboot.elf at 0x408D90.
RndCameraCom* RndSceneCom::GetCamera(int index) const {
    return FindSceneCom<RndCameraCom>(this, mCameras[index]);
}

// Reconstructed from eboot.elf at 0x408E20.
RndCameraCom* RndSceneCom::GetCamera(int index) {
    return FindSceneCom<RndCameraCom>(this, mCameras[index]);
}

// Reconstructed from eboot.elf at 0x408EB0.
void RndSceneCom::SetLightMgr(RndLightMgrCom* lightMgr) {
    mLightMgr = ObjectIdOf(lightMgr);
}

// Reconstructed from eboot.elf at 0x408ED0.
RndLightMgrCom* RndSceneCom::GetLightMgr() const {
    return FindSceneCom<RndLightMgrCom>(this, mLightMgr);
}

// Reconstructed from eboot.elf at 0x408F60.
RndLightMgrCom* RndSceneCom::GetLightMgr() {
    return FindSceneCom<RndLightMgrCom>(this, mLightMgr);
}

// Reconstructed from eboot.elf at 0x408FF0.
void RndSceneCom::SetSky(RndSkyCom* sky) {
    mSky = ObjectIdOf(sky);
}

// Reconstructed from eboot.elf at 0x409010.
RndSkyCom* RndSceneCom::GetSky() const {
    return FindSceneCom<RndSkyCom>(this, mSky);
}

// Reconstructed from eboot.elf at 0x4090A0.
RndSkyCom* RndSceneCom::GetSky() {
    return FindSceneCom<RndSkyCom>(this, mSky);
}

// Reconstructed from eboot.elf at 0x409130.
void RndSceneCom::SetAtmosphere(RndAtmosphereCom* atmosphere) {
    mAtmosphere = ObjectIdOf(atmosphere);
}

// Reconstructed from eboot.elf at 0x4091F0.
RndAtmosphereCom* RndSceneCom::GetAtmosphere() {
    if (mAtmosphere.mId == kNoObject) {
        return nullptr;
    }
    const GameObject* object = mObject->mEntity->GetObject(mAtmosphere);
    return object != nullptr ? object->GetBaseCom<RndAtmosphereCom>() : nullptr;
}

// Reconstructed from eboot.elf at 0x4093B0.
RndVolumetricScatteringCom* RndSceneCom::GetVolumetricScattering() {
    return FindSceneCom<RndVolumetricScatteringCom>(this, mAtmosphere);
}

// Reconstructed from eboot.elf at 0x4094D0.
RndPostProcCom* RndSceneCom::GetPostProc(unsigned long index) const {
    return FindSceneCom<RndPostProcCom>(this, mPostProcs[index]);
}

// Reconstructed from eboot.elf at 0x409560.
RndPostProcCom* RndSceneCom::GetPostProc(unsigned long index) {
    return FindSceneCom<RndPostProcCom>(this, mPostProcs[index]);
}

// Reconstructed from eboot.elf at 0x4095F0.
RndCMAACom* RndSceneCom::GetAntialiasing() const {
    return FindSceneCom<RndCMAACom>(this, mAntialiasing);
}

// Reconstructed from eboot.elf at 0x409680.
RndCMAACom* RndSceneCom::GetAntialiasing() {
    return FindSceneCom<RndCMAACom>(this, mAntialiasing);
}

// Reconstructed from eboot.elf at 0x40B490. The groups ("clear_depth",
// "globals", "shadergraph_data", "prop_hysteresis", "lod", "utilities")
// have no storage of their own, and their members' offsets are the
// component's. The globals' entries describe themselves by their index;
// the hysteresis settings are read-only without entries and recreate the
// texture when they change; the LOD distances keep medium below low. The
// revision handler upgrades data saved before revision 5: the single
// "post_proc" (0), LOD (1), the hysteresis properties of a single entry
// (2), the renamed "atmosphere" and "volumetric_scattering" (3) and the
// framerate that moved to TimelinesCom (4). Every registration builds a
// "prop", "struct", "array", "struct_item", "action" or "driven_prop"
// symbol it does not use.
void RndSceneCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<GlobalEntry<float>>(Symbol("GlobalEntry<float>"));
    RegisterPropStruct<GlobalEntry<Hmx::Color>>(Symbol("GlobalEntry<Hmx::Color>"));
    RegisterPropStruct<PropHysteresisEntry>(Symbol("PropHysteresisEntry"));
    RndDrawableEntityCom::_InitAsSuperclass(registry, metadata);
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mDescription = "Global scene settings";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mEditorRestrictions = 0;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    RndSceneCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    IntMetadata<int>& drawOrder = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "draw_order", offsetof(RndSceneCom, mDrawOrder), kPropertyInt, 4).mMetadata.Get());
    drawOrder.mDefault = prototype.mDrawOrder;
    static_cast<void>(Symbol("prop"));
    drawOrder.mHelp = String("How is draw order for this scene determined?");
    drawOrder.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("Automatic"), String("Draw order is automatic")},
        AllowedValue<int>{
            1,
            String("Manual"),
            String("Draw order is manual; the following rendering features will not be available: deferred "
                   "lighting, decals")}};

    IntMetadata<int>& clearType = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "clear_type", offsetof(RndSceneCom, mClearType), kPropertyInt, 4).mMetadata.Get());
    clearType.mDefault = prototype.mClearType;
    static_cast<void>(Symbol("prop"));
    clearType.mHelp = String("How is the color buffer cleared before drawing this scene?");
    clearType.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            0,
            String("Transparent"),
            String("Only clears the color buffer if this is the first scene drawn, using the default clear "
                   "color.")},
        AllowedValue<int>{1, String("Color"), String("Always clear the color buffer, using my clear color.")},
        AllowedValue<int>{
            2,
            String("Opaque"),
            String("Never clear the color buffer (use when scene geometry fully occludes the background, eg. "
                   "when you have a skybox.")}};

    ColorMetadata& clearColor = TypeSpecificMetadata(
        registry.RegisterProp("clear_color", offsetof(RndSceneCom, mClearColor), kPropertyColor, 16),
        prototype.mClearColor);
    clearColor.mDefault = prototype.mClearColor;
    static_cast<void>(Symbol("prop"));
    clearColor.mHelp = String("Color to use when clearing the color buffer before drawing this scene.");
    clearColor.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const RndSceneCom&>(args.mComponent).mClearType != 1;
    });

    PropRegistry& clearDepth = static_cast<GroupMetadata&>(
        *registry.RegisterProp("clear_depth", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& afterPostProc0 = TypeSpecificMetadata(
        clearDepth.RegisterProp(
            "after_post_proc_0", offsetof(RndSceneCom, mClearDepthAfterPostProc), kPropertyBool, 1),
        prototype.mClearDepthAfterPostProc[0]);
    afterPostProc0.mDefault = prototype.mClearDepthAfterPostProc[0];
    static_cast<void>(Symbol("prop"));
    afterPostProc0.mHelp = String(
        "Reclear the depth buffer after drawing postProc 0 (so anything in the After PostProc 0 material "
        "bucket won't depth-test with the previously drawn geometry in the scene)");
    BoolMetadata& afterPostProc1 = TypeSpecificMetadata(
        clearDepth.RegisterProp(
            "after_post_proc_1", offsetof(RndSceneCom, mClearDepthAfterPostProc) + 1, kPropertyBool, 1),
        prototype.mClearDepthAfterPostProc[1]);
    afterPostProc1.mDefault = prototype.mClearDepthAfterPostProc[1];
    static_cast<void>(Symbol("prop"));
    afterPostProc1.mHelp = String(
        "Reclear the depth buffer after drawing postProc 1 (so anything in the After PostProc 1 material "
        "bucket won't depth-test with the previously drawn geometry in the scene)");

    const bool framerateType = false;
    BoolMetadata& framerate = TypeSpecificMetadata(
        registry.RegisterProp("framerate_deprecated", -1, kPropertyBool, 0), framerateType);
    static_cast<void>(Symbol("prop"));
    framerate.mHelp = String(
        "Looking for the 'framerate' property?  Framerate controls have moved to TimelinesCom!  Create a "
        "TimelinesCom on the root of the scene to control framerate.  By default your scene will run at full "
        "framerate without one.");
    framerate.mIsHidden._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return gRndDevice->mSettings->mMaxPartialFramerateScenes == 0;
    });
    framerate.mGetter = GetFramerateDeprecated;
    framerate.mSaves = false;
    framerate.mReadOnly = true;

    GameObjectIdMetadata& camera = TypeSpecificMetadata(
        registry.RegisterProp("camera", offsetof(RndSceneCom, mCameras), kPropertyGameObjectId, 4),
        prototype.mCameras[0]);
    camera.mDefault = prototype.mCameras[0];
    static_cast<void>(Symbol("prop"));
    camera.mAllowsNull = false;
    camera.mRequiredComs = {RndCameraCom::sId};

    GameObjectIdMetadata& auxCamera = TypeSpecificMetadata(
        registry.RegisterProp(
            "aux_camera", offsetof(RndSceneCom, mCameras) + sizeof(GameObjectId), kPropertyGameObjectId, 4),
        prototype.mCameras[1]);
    auxCamera.mDefault = prototype.mCameras[1];
    static_cast<void>(Symbol("prop"));
    auxCamera.mAllowsNull = true;
    auxCamera.mRequiredComs = {RndCameraCom::sId};

    GameObjectIdMetadata& lightMgr = TypeSpecificMetadata(
        registry.RegisterProp("light_manager", offsetof(RndSceneCom, mLightMgr), kPropertyGameObjectId, 4),
        prototype.mLightMgr);
    lightMgr.mDefault = prototype.mLightMgr;
    static_cast<void>(Symbol("prop"));
    lightMgr.mAllowsNull = false;
    lightMgr.mRequiredComs = {RndLightMgrCom::sId};

    GameObjectIdMetadata& sky = TypeSpecificMetadata(
        registry.RegisterProp("sky", offsetof(RndSceneCom, mSky), kPropertyGameObjectId, 4), prototype.mSky);
    sky.mDefault = prototype.mSky;
    static_cast<void>(Symbol("prop"));
    sky.mRequiredComs = {RndSkyCom::sId};

    GameObjectIdMetadata& atmosphere = TypeSpecificMetadata(
        registry.RegisterProp("atmosphere", offsetof(RndSceneCom, mAtmosphere), kPropertyGameObjectId, 4),
        prototype.mAtmosphere);
    atmosphere.mDefault = prototype.mAtmosphere;
    static_cast<void>(Symbol("prop"));
    atmosphere.mRequiredInterfaces = {RndAtmosphereCom::sClassName};

    GameObjectIdMetadata& postProc0 = TypeSpecificMetadata(
        registry.RegisterProp("post_proc_0", offsetof(RndSceneCom, mPostProcs), kPropertyGameObjectId, 4),
        prototype.mPostProcs[0]);
    postProc0.mDefault = prototype.mPostProcs[0];
    static_cast<void>(Symbol("prop"));
    postProc0.mRequiredComs = {RndPostProcCom::sId};

    GameObjectIdMetadata& postProc1 = TypeSpecificMetadata(
        registry.RegisterProp(
            "post_proc_1", offsetof(RndSceneCom, mPostProcs) + sizeof(GameObjectId), kPropertyGameObjectId, 4),
        prototype.mPostProcs[1]);
    postProc1.mDefault = prototype.mPostProcs[1];
    static_cast<void>(Symbol("prop"));
    postProc1.mRequiredComs = {RndPostProcCom::sId};

    GameObjectIdMetadata& antialiasing = TypeSpecificMetadata(
        registry.RegisterProp("antialiasing", offsetof(RndSceneCom, mAntialiasing), kPropertyGameObjectId, 4),
        prototype.mAntialiasing);
    antialiasing.mDefault = prototype.mAntialiasing;
    static_cast<void>(Symbol("prop"));
    antialiasing.mRequiredComs = {RndCMAACom::sId};

    GroupMetadata& maskBuffer = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "mask_buffer", offsetof(RndSceneCom, mMaskBuffer), kPropertyStruct, sizeof(MaskBuffer))
        .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const MaskBuffer maskType = {false, true, 6};
    maskBuffer.mHelp =
        String("Settings governing the rendering of the mask buffer (eg. useful for object outlining)");
    PropRegistry& mask = maskBuffer.mRegistry;
    BoolMetadata& maskEnabled = TypeSpecificMetadata(
        mask.RegisterProp("enabled", offsetof(MaskBuffer, mEnabled), kPropertyBool, 1), maskType.mEnabled);
    maskEnabled.mDefault = maskType.mEnabled;
    static_cast<void>(Symbol("prop"));
    BoolMetadata& signedDistance = TypeSpecificMetadata(
        mask.RegisterProp(
            "calc_signed_distance", offsetof(MaskBuffer, mCalcSignedDistance), kPropertyBool, 1),
        maskType.mCalcSignedDistance);
    signedDistance.mDefault = maskType.mCalcSignedDistance;
    static_cast<void>(Symbol("prop"));
    signedDistance.mHelp = String("Should we compute a signed distance field for the mask buffer?");
    signedDistance.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSceneCom&>(args.mComponent).mMaskBuffer.mEnabled;
    });
    IntMetadata<int>& signedDistanceWidth = TypeSpecificMetadata(
        mask.RegisterProp(
            "signed_distance_width", offsetof(MaskBuffer, mSignedDistanceWidth), kPropertyInt, 4),
        maskType.mSignedDistanceWidth);
    signedDistanceWidth.mDefault = maskType.mSignedDistanceWidth;
    static_cast<void>(Symbol("prop"));
    signedDistanceWidth.mHelp = String(
        "Width (in pixels, as viewed at 1080p) of the signed distance field generated from the raw mask "
        "buffer; wider is more expensive, so use with care (0 disables it)");
    signedDistanceWidth.mUnits = Symbol("pixels");
    signedDistanceWidth.mMin = 0;
    signedDistanceWidth.mMax = 16;
    signedDistanceWidth.mSlider = true;
    signedDistanceWidth.mReadOnly._Set([](const TagArgs& args) {
        const MaskBuffer& settings = static_cast<const RndSceneCom&>(args.mComponent).mMaskBuffer;
        return !settings.mEnabled || !settings.mCalcSignedDistance;
    });

    // The entity the globals' driving properties live in: the scene's.
    const auto sceneEntity = [](const TagArgs& args) { return args.mComponent.mObject->mEntity; };

    PropRegistry& globals = static_cast<GroupMetadata&>(
        *registry.RegisterProp("globals", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));

    PropInfo& floats =
        globals.RegisterPropArray("floats", offsetof(RndSceneCom, mFloats), sizeof(PropArray<GlobalEntry<float>>));
    ArrayMetadata& floatsMetadata = static_cast<ArrayMetadata&>(*floats.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    floatsMetadata.mFixedSize = true;
    PropRegistry::RegisterPropArrayItem(floats, kPropertyStruct, sizeof(GlobalEntry<float>));
    GroupMetadata& floatEntry = static_cast<GroupMetadata&>(*floatsMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    GlobalEntry<float> floatType;
    floatEntry.mDisplayName._Set([](const TagArgs& args) {
        return String((FormatString("float %d") << args.mPath[-1].mValue).Str());
    });
    floatEntry.mSummary._Set([](const TagArgs& args) {
        const RndSceneCom& scene = static_cast<const RndSceneCom&>(args.mComponent);
        return scene.mFloats[args.mPath[-1].mValue].GetSummary(args.mComponent, args.mPath);
    });
    floatEntry.mHelp._Set([](const TagArgs& args) {
        return String((FormatString("Drives global float #%d (access via SceneGlobalsInfo in entities within "
                                    "this scene, and via SceneGlobalFloat in shader graphs within this scene)")
                       << args.mPath[-1].mValue)
                          .Str());
    });
    SymbolMetadata& floatComment = TypeSpecificMetadata(
        floatEntry.mRegistry.RegisterProp("comment", offsetof(GlobalEntry<float>, mComment), kPropertySymbol, 8),
        floatType.mComment);
    floatComment.mDefault = floatType.mComment;
    static_cast<void>(Symbol("prop"));
    floatComment.mHelp._Set([](const TagArgs& args) {
        return String((FormatString("Comment for float %d") << args.mPath[-2].mValue).Str());
    });
    GroupMetadata& floatValue = floatType.mValue.Register(
        floatEntry.mRegistry,
        Symbol("value"),
        offsetof(GlobalEntry<float>, mValue),
        std::function<Entity*(const TagArgs&)>(sceneEntity));
    static_cast<void>(Symbol("driven_prop"));
    floatValue.mHelp._Set([](const TagArgs& args) {
        return String((FormatString("Drives global float #%d (access via SceneGlobalsInfo in entities within "
                                    "this scene, and via SceneGlobalFloat in shader graphs within this scene)")
                       << args.mPath[-2].mValue)
                          .Str());
    });

    PropInfo& colors = globals.RegisterPropArray(
        "colors", offsetof(RndSceneCom, mColors), sizeof(PropArray<GlobalEntry<Hmx::Color>>));
    ArrayMetadata& colorsMetadata = static_cast<ArrayMetadata&>(*colors.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    colorsMetadata.mFixedSize = true;
    PropRegistry::RegisterPropArrayItem(colors, kPropertyStruct, sizeof(GlobalEntry<Hmx::Color>));
    GroupMetadata& colorEntry = static_cast<GroupMetadata&>(*colorsMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    // A new colour entry is white.
    GlobalEntry<Hmx::Color> colorType;
    colorType.mValue.mValue = Hmx::Color::GetWhite();
    colorEntry.mDisplayName._Set([](const TagArgs& args) {
        return String((FormatString("color %d") << args.mPath[-1].mValue).Str());
    });
    colorEntry.mSummary._Set([](const TagArgs& args) {
        const RndSceneCom& scene = static_cast<const RndSceneCom&>(args.mComponent);
        return scene.mColors[args.mPath[-1].mValue].GetSummary(args.mComponent, args.mPath);
    });
    colorEntry.mHelp._Set([](const TagArgs& args) {
        return String(
            (FormatString("Color which drives SceneGlobalColor #%d in shader graphs within this scene")
             << args.mPath[-1].mValue)
                .Str());
    });
    SymbolMetadata& colorComment = TypeSpecificMetadata(
        colorEntry.mRegistry.RegisterProp(
            "comment", offsetof(GlobalEntry<Hmx::Color>, mComment), kPropertySymbol, 8),
        colorType.mComment);
    colorComment.mDefault = colorType.mComment;
    static_cast<void>(Symbol("prop"));
    colorComment.mHelp._Set([](const TagArgs& args) {
        return String((FormatString("Comment for color %d") << args.mPath[-2].mValue).Str());
    });
    colorType.mValue.Register(
        colorEntry.mRegistry,
        Symbol("value"),
        offsetof(GlobalEntry<Hmx::Color>, mValue),
        std::function<Entity*(const TagArgs&)>(sceneEntity));
    static_cast<void>(Symbol("driven_prop"));

    PropRegistry& shaderGraph = static_cast<GroupMetadata&>(
        *registry.RegisterProp("shadergraph_data", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    GameObjectIdMetadata& trans0 = TypeSpecificMetadata(
        shaderGraph.RegisterProp("trans_0", offsetof(RndSceneCom, mShaderGraphTrans), kPropertyGameObjectId, 4),
        prototype.mShaderGraphTrans[0]);
    trans0.mDefault = prototype.mShaderGraphTrans[0];
    static_cast<void>(Symbol("prop"));
    trans0.mHelp = String("Transform which provides TransInfo #0 to the shader graph for this scene");
    GameObjectIdMetadata& trans1 = TypeSpecificMetadata(
        shaderGraph.RegisterProp("trans_1", offsetof(RndSceneCom, mShaderGraphTrans) + 1 * sizeof(GameObjectId), kPropertyGameObjectId, 4),
        prototype.mShaderGraphTrans[1]);
    trans1.mDefault = prototype.mShaderGraphTrans[1];
    static_cast<void>(Symbol("prop"));
    trans1.mHelp = String("Transform which provides TransInfo #1 to the shader graph for this scene");
    GameObjectIdMetadata& trans2 = TypeSpecificMetadata(
        shaderGraph.RegisterProp("trans_2", offsetof(RndSceneCom, mShaderGraphTrans) + 2 * sizeof(GameObjectId), kPropertyGameObjectId, 4),
        prototype.mShaderGraphTrans[2]);
    trans2.mDefault = prototype.mShaderGraphTrans[2];
    static_cast<void>(Symbol("prop"));
    trans2.mHelp = String("Transform which provides TransInfo #2 to the shader graph for this scene");
    GameObjectIdMetadata& trans3 = TypeSpecificMetadata(
        shaderGraph.RegisterProp("trans_3", offsetof(RndSceneCom, mShaderGraphTrans) + 3 * sizeof(GameObjectId), kPropertyGameObjectId, 4),
        prototype.mShaderGraphTrans[3]);
    trans3.mDefault = prototype.mShaderGraphTrans[3];
    static_cast<void>(Symbol("prop"));
    trans3.mHelp = String("Transform which provides TransInfo #3 to the shader graph for this scene");

    PropRegistry& hysteresis = static_cast<GroupMetadata&>(
        *shaderGraph.RegisterProp("prop_hysteresis", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    // The types a hysteresis row can read: the numbers and bool.
    eastl::vector<PropertyType> numberTypes;
    {
        unsigned int temp;
        MemPushTemp(temp, true, true);
        numberTypes.push_back(kPropertyFloat);
        numberTypes.push_back(kPropertyInt8);
        numberTypes.push_back(kPropertyInt16);
        numberTypes.push_back(kPropertyInt);
        numberTypes.push_back(kPropertyInt64);
        numberTypes.push_back(kPropertyUInt8);
        numberTypes.push_back(kPropertyUInt16);
        numberTypes.push_back(kPropertyUInt);
        numberTypes.push_back(kPropertyUInt64);
        numberTypes.push_back(kPropertyBool);
        MemPopTemp(temp);
    }

    PropInfo& entries = hysteresis.RegisterPropArray(
        "entries",
        offsetof(RndSceneCom, mPropHysteresis) + offsetof(PropHysteresis, mEntries),
        sizeof(PropArray<PropHysteresisEntry>));
    ArrayMetadata& entriesMetadata = static_cast<ArrayMetadata&>(*entries.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(entries, kPropertyStruct, sizeof(PropHysteresisEntry));
    PropRegistry& entry = static_cast<GroupMetadata&>(*entriesMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const PropHysteresisEntry entryType;

    GroupMetadata& defaultValues = static_cast<GroupMetadata&>(*entry.RegisterProp(
        "default_values", offsetof(PropHysteresisEntry, mDefaultValues), kPropertyStruct, sizeof(Vector4))
        .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Vector4 valuesType = {0.0F, 0.0F, 0.0F, 0.0F};
    defaultValues.mHelp = String("Default values for clearing the hysteresis texture");
    // The struct registrations set this word; its reader is not identified.
    defaultValues.mKind = 1;
    FloatMetadata& valueX = TypeSpecificMetadata(
        defaultValues.mRegistry.RegisterProp("x", offsetof(Vector4, x), kPropertyFloat, 4), valuesType.x);
    valueX.mDefault = valuesType.x;
    static_cast<void>(Symbol("prop"));
    valueX.mOnChanged = OnDefaultValueXChanged;
    FloatMetadata& valueY = TypeSpecificMetadata(
        defaultValues.mRegistry.RegisterProp("y", offsetof(Vector4, y), kPropertyFloat, 4), valuesType.y);
    valueY.mDefault = valuesType.y;
    static_cast<void>(Symbol("prop"));
    valueY.mOnChanged = OnDefaultValueYChanged;
    FloatMetadata& valueZ = TypeSpecificMetadata(
        defaultValues.mRegistry.RegisterProp("z", offsetof(Vector4, z), kPropertyFloat, 4), valuesType.z);
    valueZ.mDefault = valuesType.z;
    static_cast<void>(Symbol("prop"));
    valueZ.mOnChanged = OnDefaultValueZChanged;
    FloatMetadata& valueW = TypeSpecificMetadata(
        defaultValues.mRegistry.RegisterProp("w", offsetof(Vector4, w), kPropertyFloat, 4), valuesType.w);
    valueW.mDefault = valuesType.w;
    static_cast<void>(Symbol("prop"));
    valueW.mOnChanged = OnDefaultValueWChanged;
    defaultValues.mAnimatable = false;

    PropRefMetadata& prop0 = TypeSpecificMetadata(
        entry.RegisterProp("prop_0", offsetof(PropHysteresisEntry, mProps), kPropertyPropRef, sizeof(PropRef)),
        entryType.mProps[0]);
    prop0.mRefDefault = entryType.mProps[0];
    static_cast<void>(Symbol("prop"));
    prop0.mHelp = String("Property which drives the red channel of the hysteresis texture");
    prop0.mAllowedTypes = numberTypes;
    prop0.mRefOnChanged = OnProp0Changed;
    PropRefMetadata& prop1 = TypeSpecificMetadata(
        entry.RegisterProp("prop_1", offsetof(PropHysteresisEntry, mProps) + 1 * sizeof(PropRef), kPropertyPropRef, sizeof(PropRef)),
        entryType.mProps[1]);
    prop1.mRefDefault = entryType.mProps[1];
    static_cast<void>(Symbol("prop"));
    prop1.mHelp = String("Property which drives the green channel of the hysteresis texture");
    prop1.mAllowedTypes = numberTypes;
    prop1.mRefOnChanged = OnProp1Changed;
    PropRefMetadata& prop2 = TypeSpecificMetadata(
        entry.RegisterProp("prop_2", offsetof(PropHysteresisEntry, mProps) + 2 * sizeof(PropRef), kPropertyPropRef, sizeof(PropRef)),
        entryType.mProps[2]);
    prop2.mRefDefault = entryType.mProps[2];
    static_cast<void>(Symbol("prop"));
    prop2.mHelp = String("Property which drives the blue channel of the hysteresis texture");
    prop2.mAllowedTypes = numberTypes;
    prop2.mRefOnChanged = OnProp2Changed;
    PropRefMetadata& prop3 = TypeSpecificMetadata(
        entry.RegisterProp("prop_3", offsetof(PropHysteresisEntry, mProps) + 3 * sizeof(PropRef), kPropertyPropRef, sizeof(PropRef)),
        entryType.mProps[3]);
    prop3.mRefDefault = entryType.mProps[3];
    static_cast<void>(Symbol("prop"));
    prop3.mHelp = String("Property which drives the alpha channel of the hysteresis texture");
    prop3.mAllowedTypes = numberTypes;
    prop3.mRefOnChanged = OnProp3Changed;
    entriesMetadata.mInsert = InsertPropHysteresisEntry;
    // The hook that replaces the default removal of an element
    // (_RemoveProp, 0x199300).
    entriesMetadata.mHook2 = reinterpret_cast<void*>(RemovePropHysteresisEntry);

    IntMetadata<int>& timeline = static_cast<IntMetadata<int>&>(*hysteresis.RegisterProp(
        "timeline",
        offsetof(RndSceneCom, mPropHysteresis) + offsetof(PropHysteresis, mTimeline),
        kPropertyInt,
        4).mMetadata.Get());
    timeline.mDefault = prototype.mPropHysteresis.mTimeline;
    static_cast<void>(Symbol("prop"));
    timeline.mHelp = String("Timeline for updating the hysteresis texture");
    timeline.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("Seconds"), String("Seconds")},
        AllowedValue<int>{2, String("UI Seconds"), String("UI Seconds")}};
    timeline.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndSceneCom&>(args.mComponent).mPropHysteresis.mEntries.size() == 0;
    });
    timeline.mAnimatable = false;

    IntMetadata<unsigned int>& textureSize = TypeSpecificMetadata(
        hysteresis.RegisterProp(
            "texture_size",
            offsetof(RndSceneCom, mPropHysteresis) + offsetof(PropHysteresis, mTextureSize),
            kPropertyUInt,
            4),
        prototype.mPropHysteresis.mTextureSize);
    textureSize.mDefault = prototype.mPropHysteresis.mTextureSize;
    static_cast<void>(Symbol("prop"));
    textureSize.mHelp =
        String("Size of the hysteresis texture (ie. the number of frames of hysteresis we retain)");
    textureSize.mMin = 1U;
    textureSize.mMax = 256U;
    textureSize.mSlider = true;
    textureSize.mSliderLogResolution = 1.0F;
    textureSize.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndSceneCom&>(args.mComponent).mPropHysteresis.mEntries.size() == 0;
    });
    textureSize.mAnimatable = false;
    textureSize.mOnChanged = OnTextureSizeChanged;

    IntMetadata<int>& wrapMode = static_cast<IntMetadata<int>&>(*hysteresis.RegisterProp(
        "wrap_mode",
        offsetof(RndSceneCom, mPropHysteresis) + offsetof(PropHysteresis, mWrapMode),
        kPropertyInt,
        4).mMetadata.Get());
    wrapMode.mDefault = prototype.mPropHysteresis.mWrapMode;
    static_cast<void>(Symbol("prop"));
    wrapMode.mHelp = String("Wrap mode of the hysteresis texture");
    {
        eastl::vector<AllowedValue<int>> modes;
        modes.reserve(6);
        for (int mode = 0; mode != 6; ++mode) {
            const RndTexWrapMode wrap = static_cast<RndTexWrapMode>(mode);
            modes.push_back(AllowedValue<int>{
                mode, String(RndTextureUtl::GetWrapModeName(wrap)), String(RndTextureUtl::GetWrapModeHelp(wrap))});
        }
        wrapMode.mAllowedValues = modes;
    }
    wrapMode.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndSceneCom&>(args.mComponent).mPropHysteresis.mEntries.size() == 0;
    });
    wrapMode.mAnimatable = false;
    wrapMode.mOnChanged = OnWrapModeChanged;

    IntMetadata<int>& filterMode = static_cast<IntMetadata<int>&>(*hysteresis.RegisterProp(
        "filter_mode",
        offsetof(RndSceneCom, mPropHysteresis) + offsetof(PropHysteresis, mFilterMode),
        kPropertyInt,
        4).mMetadata.Get());
    filterMode.mDefault = prototype.mPropHysteresis.mFilterMode;
    static_cast<void>(Symbol("prop"));
    filterMode.mHelp = String("Filter mode of the hysteresis texture");
    {
        // Point and bilinear filtering only.
        eastl::vector<AllowedValue<int>> modes;
        modes.reserve(4);
        for (int mode = 0; mode != 6; ++mode) {
            if (static_cast<unsigned int>(mode - 1) <= 1) {
                const RndTexFilterMode filter = static_cast<RndTexFilterMode>(mode);
                modes.push_back(AllowedValue<int>{
                    mode,
                    String(RndTextureUtl::GetFilterModeName(filter)),
                    String(RndTextureUtl::GetFilterModeHelp(filter))});
            }
        }
        filterMode.mAllowedValues = modes;
    }
    filterMode.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndSceneCom&>(args.mComponent).mPropHysteresis.mEntries.size() == 0;
    });
    filterMode.mAnimatable = false;
    filterMode.mOnChanged = OnFilterModeChanged;

    ActionMetadata& reset = static_cast<ActionMetadata&>(*hysteresis.RegisterAction("reset").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    reset.mAction = [](const TagArgs& args) {
        const_cast<RndSceneCom&>(static_cast<const RndSceneCom&>(args.mComponent))._ClearPropHysteresisTexture();
    };
    reset.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndSceneCom&>(args.mComponent).mPropHysteresis.mEntries.size() == 0;
    });
    reset.mHistory = false;

    PropRegistry& lod =
        static_cast<GroupMetadata&>(*registry.RegisterProp("lod", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& enableLod = TypeSpecificMetadata(
        lod.RegisterProp("enable_lod", offsetof(RndSceneCom, mEnableLod), kPropertyBool, 1), prototype.mEnableLod);
    enableLod.mDefault = prototype.mEnableLod;
    static_cast<void>(Symbol("prop"));
    enableLod.mHelp = String("Does this scene use LOD?");

    FloatMetadata& mediumLod = TypeSpecificMetadata(
        lod.RegisterProp("medium_lod_distance", offsetof(RndSceneCom, mMediumLodDistance), kPropertyFloat, 4),
        prototype.mMediumLodDistance);
    mediumLod.mDefault = prototype.mMediumLodDistance;
    static_cast<void>(Symbol("prop"));
    mediumLod.mHelp = String("Distance at which we start drawing medium LOD objects");
    mediumLod.mMin = 0.0F;
    mediumLod.mMax = 10000.0F;
    mediumLod.mSlider = true;
    mediumLod.mSliderLogResolution = 2.0F;
    mediumLod.mOnChanged = OnMediumLodDistanceChanged;
    mediumLod.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSceneCom&>(args.mComponent).mEnableLod;
    });

    FloatMetadata& lowLod = TypeSpecificMetadata(
        lod.RegisterProp("low_lod_distance", offsetof(RndSceneCom, mLowLodDistance), kPropertyFloat, 4),
        prototype.mLowLodDistance);
    lowLod.mDefault = prototype.mLowLodDistance;
    static_cast<void>(Symbol("prop"));
    lowLod.mHelp = String("Distance at which we start drawing low LOD objects");
    lowLod.mMin = 0.0F;
    lowLod.mMax = 10000.0F;
    lowLod.mSlider = true;
    lowLod.mSliderLogResolution = 2.0F;
    lowLod.mOnChanged = OnLowLodDistanceChanged;
    lowLod.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSceneCom&>(args.mComponent).mEnableLod;
    });

    IntMetadata<int>& isolateLod = static_cast<IntMetadata<int>&>(*lod.RegisterProp(
        "isolate_lod", offsetof(RndSceneCom, mIsolateLod), kPropertyInt, 4).mMetadata.Get());
    isolateLod.mDefault = prototype.mIsolateLod;
    static_cast<void>(Symbol("prop"));
    isolateLod.mHelp = String("Set to restrict to only drawing a single LOD (for debugging)");
    isolateLod.mSaves = false;
    {
        eastl::vector<AllowedValue<int>> levels;
        levels.reserve(4);
        levels.push_back(AllowedValue<int>{-1, String("<None>"), String("<None>")});
        for (int level = 0; level < 3; ++level) {
            const char* const name = RndUtl::GetLodName(static_cast<RndSceneLod>(level));
            levels.push_back(AllowedValue<int>{level, String(name), String(name)});
        }
        isolateLod.mAllowedValues = levels;
    }
    isolateLod.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSceneCom&>(args.mComponent).mEnableLod;
    });

    BoolMetadata& neverCull = TypeSpecificMetadata(
        registry.RegisterProp("never_cull", offsetof(RndSceneCom, mNeverCull), kPropertyBool, 1),
        prototype.mNeverCull);
    neverCull.mDefault = prototype.mNeverCull;
    static_cast<void>(Symbol("prop"));
    neverCull.mHelp = String(
        "Should this scene never be culled? Set this for scenes which will always be fully on camera, and "
        "therefore do not require culling.");

    IntMetadata<unsigned long>& numDrawEntries = TypeSpecificMetadata(
        registry.RegisterProp("num_draw_entries", offsetof(RndSceneCom, mNumDrawEntries), kPropertyUInt64, 8),
        prototype.mNumDrawEntries);
    numDrawEntries.mDefault = prototype.mNumDrawEntries;
    static_cast<void>(Symbol("prop"));
    numDrawEntries.mReadOnly = true;
    numDrawEntries.mDebug = true;

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            RndSceneCom* const scene = static_cast<RndSceneCom*>(args.mComponent);
            const int rev = args.mSavedRev;
            if (rev < 1) {
                GameObjectId postProc = {kNoObject};
                ReadSavedPropOrWarn(
                    SavedProps(args),
                    PropPath(Symbol("post_proc")),
                    &postProc,
                    kPropertyGameObjectId);
                if (postProc.mId != kNoObject) {
                    scene->mPostProcs[0] = postProc;
                }
            }
            if (rev < 2) {
                scene->mEnableLod = true;
            }
            if (rev < 3) {
                // The single row the older layout kept in shadergraph_data.
                PropHysteresisEntry row;
                const bool found0 = ReadSavedProp(
                    SavedProps(args), MemberPath("shadergraph_data", "prop_hysteresis", "prop_0"), &row.mProps[0],
                    kPropertyPropRef);
                const bool found1 = ReadSavedProp(
                    SavedProps(args),
                    MemberPath("shadergraph_data", "prop_hysteresis", "prop_1"), &row.mProps[1], kPropertyPropRef);
                const bool found2 = ReadSavedProp(
                    SavedProps(args),
                    MemberPath("shadergraph_data", "prop_hysteresis", "prop_2"), &row.mProps[2], kPropertyPropRef);
                const bool found3 = ReadSavedProp(
                    SavedProps(args),
                    MemberPath("shadergraph_data", "prop_hysteresis", "prop_3"), &row.mProps[3], kPropertyPropRef);
                if (found3 || found0 || found1 || found2) {
                    float x = 0.0F;
                    ReadSavedPropOrWarn(
                        SavedProps(args),
                        MemberPath("shadergraph_data", "prop_hysteresis", "default_values", "x"), &x,
                        kPropertyFloat);
                    row.mDefaultValues.x = x;
                    float y = 0.0F;
                    ReadSavedPropOrWarn(
                        SavedProps(args),
                        MemberPath("shadergraph_data", "prop_hysteresis", "default_values", "y"), &y,
                        kPropertyFloat);
                    row.mDefaultValues.y = y;
                    float z = 0.0F;
                    ReadSavedPropOrWarn(
                        SavedProps(args),
                        MemberPath("shadergraph_data", "prop_hysteresis", "default_values", "z"), &z,
                        kPropertyFloat);
                    row.mDefaultValues.z = z;
                    float w = 0.0F;
                    ReadSavedPropOrWarn(
                        SavedProps(args),
                        MemberPath("shadergraph_data", "prop_hysteresis", "default_values", "w"), &w,
                        kPropertyFloat);
                    row.mDefaultValues.w = w;
                    scene->mPropHysteresis.mEntries._Insert(scene->mPropHysteresis.mEntries.mSize, &row);
                }
            }
            if (rev < 4) {
                // The old "atmosphere" is the sky, and the old
                // "volumetric_scattering" the atmosphere.
                GameObjectId sky = {kNoObject};
                if (ReadSavedProp(
                        SavedProps(args), PropPath(Symbol("atmosphere")), &sky,
                        kPropertyGameObjectId)) {
                    scene->mSky = sky;
                }
                GameObjectId atmosphere = {kNoObject};
                if (ReadSavedProp(
                        SavedProps(args),
                        PropPath(Symbol("volumetric_scattering")), &atmosphere, kPropertyGameObjectId)) {
                    scene->mAtmosphere = atmosphere;
                }
            }
            if (rev < 5) {
                // A scene saved at half rate is told that the framerate moved
                // to TimelinesCom; the notification was compiled out.
                int framerate = -1;
                Component& saved = const_cast<Component&>(SavedProps(args));
                if (saved._GetPropRegistry().FindProp(PropPath(Symbol("framerate")), nullptr) != nullptr) {
                    ReadSavedPropOrWarn(saved, PropPath(Symbol("framerate")), &framerate, kPropertyInt);
                    if (framerate == 1) {
                        static_cast<void>(scene->MakeErrorName());
                    }
                }
            }
        };
    if (&registry == &own) {
        own.SetCurrentRev(5);
        own.mRevisionFunc = onRevision;
    }

    PropRegistry& utilities = static_cast<GroupMetadata&>(
        *registry.RegisterProp("utilities", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    ActionMetadata& dumpQuality =
        static_cast<ActionMetadata&>(*utilities.RegisterAction("dump_quality_settings").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    dumpQuality.mHelp = String(
        "Dumps a report of quality settings for this scene (saves to ../logs/quality_settings_*.csv)");
    // The report is not part of this build.
    dumpQuality.mAction = [](const TagArgs& args) { static_cast<void>(args); };
}

// Reconstructed from eboot.elf at 0x4116A0. The poll manager is named after
// the entity's file.
bool RndSceneCom::_OnResourcesLoaded() {
    mFloats.Resize(4);
    mColors.Resize(4);
    if (mPollMgr == nullptr) {
        mPollMgr = new PollMgr(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    }
    if (mSceneDrawer == nullptr) {
        mSceneDrawer = new RndSceneDrawer();
    }
    if (mLightMgr.mId == kNoObject) {
        const Entity* entity = mObject->mEntity;
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            if (object->GetCom<RndLightMgrCom>() != nullptr) {
                mLightMgr = object->mId;
                break;
            }
        }
    }
    _CreatePropHysteresisTexture();
    mTimelines = mObject->GetBaseCom<TimelinesCom>();
    if (mTimelines != nullptr) {
        mTimelines->mPartialFramerate = PartialFramerateActive();
    }
    return true;
}

// Reconstructed from eboot.elf at 0x411880. The rows are written through a
// float canvas in the temporary heap and converted to the closest
// supported four-channel 16-bit float format.
void RndSceneCom::_CreatePropHysteresisTexture() {
    delete mPropHysteresisTexture;
    mPropHysteresisTexture = nullptr;
    const unsigned int rows = mPropHysteresis.mEntries.size();
    if (rows == 0) {
        return;
    }
    RndTexture2D::Description desc;
    desc.mName = "Prop Hysteresis";
    desc.mRequestedFormat.mWrapMode = static_cast<unsigned int>(mPropHysteresis.mWrapMode);
    desc.mRequestedFormat.mFilterMode = static_cast<unsigned int>(mPropHysteresis.mFilterMode);
    desc.mRequestedFormat.mFlags = 1;
    const RndDataFormatInfo info = {64, 4, 2, 1, -1};
    const int format = RndFindSupportedDataFormat(info, kPlatformPS4);
    unsigned int savedTemp;
    MemPushTemp(savedTemp, true, true);
    {
        RndPixelCanvas canvas;
        canvas.CreateUninitialized(static_cast<int>(mPropHysteresis.mTextureSize), static_cast<int>(rows), 1);
        for (unsigned long row = 0; row < mPropHysteresis.mEntries.size(); ++row) {
            const Vector4& values = mPropHysteresis.mEntries[row].mDefaultValues;
            for (int x = 0; x < static_cast<int>(mPropHysteresis.mTextureSize); ++x) {
                canvas.mPixels[x + static_cast<int>(row) * canvas.mWidth] =
                    Hmx::Color(values.x, values.y, values.z, values.w);
            }
        }
        desc.mPixels.CreateEmpty(format);
        desc.mPixels.ConvertFrom(canvas);
    }
    MemPopTemp(savedTemp);
    mPropHysteresisTexture = RndTexture2D::New(desc);
    mPropHysteresisPosition = 0.0F;
}

// Reconstructed from eboot.elf at 0x411A70.
void RndSceneCom::_Enter() {
    if (mTimelines != nullptr) {
        mTimelines->mPartialFramerate = PartialFramerateActive();
    }
}

// Reconstructed from eboot.elf at 0x411AB0.
void RndSceneCom::_Exit(DestroyType type) {
    static_cast<void>(type);
    if (mPollGroups != nullptr) {
        mPollGroups->Reset();
        delete mPollGroups;
    }
    mPollGroups = nullptr;
}

// Reconstructed from eboot.elf at 0x411AF0.
void RndSceneCom::ResetDrawRegistrations() {
    mSceneDrawer->ResetRegistrations();
}

// Reconstructed from eboot.elf at 0x411B40. The canvas is created in the
// temporary heap.
void RndSceneCom::_ClearPropHysteresisTexture() {
    RndPixelCanvas canvas;
    unsigned int savedTemp;
    MemPushTemp(savedTemp, true, true);
    canvas.CreateUninitialized(
        static_cast<int>(mPropHysteresis.mTextureSize), static_cast<int>(mPropHysteresis.mEntries.size()), 1);
    MemPopTemp(savedTemp);
    for (unsigned long row = 0; row < mPropHysteresis.mEntries.size(); ++row) {
        const Vector4& values = mPropHysteresis.mEntries[row].mDefaultValues;
        for (int x = 0; x < static_cast<int>(mPropHysteresis.mTextureSize); ++x) {
            canvas.mPixels[static_cast<int>(row) * canvas.mWidth + x] =
                Hmx::Color(values.x, values.y, values.z, values.w);
        }
    }
    mPropHysteresisTexture->mPixels.ConvertFrom(canvas);
}

// Reconstructed from eboot.elf at 0x411B00.
void RndSceneCom::_Poll() {
    if (mTimelines != nullptr) {
        mTimelines->mPartialFramerate = PartialFramerateActive();
    }
}

// Reconstructed from eboot.elf at 0x411C60.
Symbol RndSceneCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x411C70.
Symbol RndSceneCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x411C80.
int RndSceneCom::CurrentRev() const {
    return const_cast<RndSceneCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x411CA0.
bool RndSceneCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x411CD0.
Component* RndSceneCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x411CE0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndSceneCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSceneCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSceneCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x411FE0.
PropRegistry& RndSceneCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x411FF0.
ComMetaData& RndSceneCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x412000. The deleting destructor is at
// 0x412010.
RndSceneCom::PollSplitJob::~PollSplitJob() {}

// Reconstructed from eboot.elf at 0x412030.
bool RndSceneCom::PollSplitJob::IsPollEnabled() const {
    return mEntity->IsPollEnabled();
}

// Reconstructed from eboot.elf at 0x4121A0. Both groups are made threaded
// twice.
RndSceneCom::PollGroups::PollGroups(const char* path) {
    PollGroup* const drawer =
        new PollGroup((FormatString("%s SceneDrawer PG") << FileGetName(path)).Str());
    PollGroup* const scene = new PollGroup((FormatString("%s Scene PG") << FileGetName(path)).Str());
    drawer->SetThreaded(true);
    scene->SetThreaded(true);
    scene->PollAfter(drawer);
    drawer->SetThreaded(true);
    scene->SetThreaded(true);
    mPollGroups[0] = drawer;
    mPollGroups[1] = scene;
}

// Reconstructed from eboot.elf at 0x412450.
void RndSceneCom::PollGroups::Reset() {
    PollGroupMap::Reset();
    mPollGroups.find(1)->second->PollAfter(mPollGroups.find(0)->second);
}

// Reconstructed from eboot.elf at 0x412750. 0x412540 is a thunk to it.
RndSceneCom::PollGroups::~PollGroups() {
    if (mPollGroups.size() != 0) {
        mPollGroups[1]->RemovePollAfter(mPollGroups[0]);
        mPollGroups[0]->ClearJobs();
        mPollGroups[1]->ClearJobs();
    }
}

namespace {

// Reconstructed from eboot.elf at 0x412A00. The deprecated property reads
// as false.
bool GetFramerateDeprecated(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return false;
}

// Reconstructed from eboot.elf at 0x412A10.
void OnDefaultValueXChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A20.
void OnDefaultValueYChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A30.
void OnDefaultValueZChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A40.
void OnDefaultValueWChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A50.
void OnProp0Changed(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A60.
void OnProp1Changed(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A70.
void OnProp2Changed(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A80.
void OnProp3Changed(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412A90. Inserts a default row at the
// index the path's last node names.
void InsertPropHysteresisEntry(const PropAccessorArgs& args) {
    RndSceneCom* const scene = static_cast<RndSceneCom*>(args.mComponent);
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-1].mValue;
    const RndSceneCom::PropHysteresisEntry row;
    scene->mPropHysteresis.mEntries._Insert(index, &row);
    scene->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412B60. Removes the row the path's last
// index names, as the default removal does.
void RemovePropHysteresisEntry(const PropAccessorArgs& args) {
    RndSceneCom* const scene = static_cast<RndSceneCom*>(args.mComponent);
    PropArrayBase& entries = scene->mPropHysteresis.mEntries;
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-1].mValue;
    entries.Destruct(1, entries.ElementAt(index));
    const unsigned int last = entries.mSize - 1;
    if (static_cast<unsigned int>(index) < last) {
        entries.Move(last - index, entries.ElementAt(index), entries.ElementAt(index + 1));
    }
    entries.mSize = last;
    scene->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412C30.
void OnTextureSizeChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412C40.
void OnWrapModeChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412C50.
void OnFilterModeChanged(const PropAccessorArgs& args) {
    static_cast<RndSceneCom*>(args.mComponent)->_CreatePropHysteresisTexture();
}

// Reconstructed from eboot.elf at 0x412C60. Keeps the low distance at
// least the medium one.
void OnMediumLodDistanceChanged(const PropAccessorArgs& args) {
    RndSceneCom* const scene = static_cast<RndSceneCom*>(args.mComponent);
    scene->mLowLodDistance =
        scene->mMediumLodDistance > scene->mLowLodDistance ? scene->mMediumLodDistance : scene->mLowLodDistance;
}

// Reconstructed from eboot.elf at 0x412C80. Keeps the medium distance at
// most the low one.
void OnLowLodDistanceChanged(const PropAccessorArgs& args) {
    RndSceneCom* const scene = static_cast<RndSceneCom*>(args.mComponent);
    scene->mMediumLodDistance =
        scene->mLowLodDistance < scene->mMediumLodDistance ? scene->mLowLodDistance : scene->mMediumLodDistance;
}

}  // namespace

// Reconstructed from eboot.elf at 0x413C70. The colour copy is at
// 0x4158E0.
template <class T>
String RndSceneCom::GlobalEntry<T>::GetSummary(const Component& component, const PropPath& path) const {
    String summary;
    PropPath valuePath = path;
    PropPath::Node node;
    valuePath.PushNode(node = Symbol("value"));
    const void* value;
    if (mValue.mDriven && mValue.mProp.IsComplete()) {
        valuePath.PushNode(node = Symbol("prop"));
        value = &mValue.mProp;
    } else {
        valuePath.PushNode(node = Symbol("value"));
        value = &mValue.mValue;
    }
    const PropInfo* const info = const_cast<Component&>(component)._GetPropRegistry().FindProp(valuePath, nullptr);
    summary = ValueToString(component, *info, valuePath, value);
    if (mComment != Symbol()) {
        summary << " (" << mComment << ")";
    }
    return summary;
}

template struct RndSceneCom::GlobalEntry<float>;
template struct RndSceneCom::GlobalEntry<Hmx::Color>;

// Reconstructed from eboot.elf at 0x4048E0. The binary emits the factory in
// render/RndInit.o with the class's Init.
Component* RndSceneCom::_Create() {
    return new RndSceneCom();
}

// Reconstructed from eboot.elf at 0x3F1F60. The binary emits the registration
// in render/RndInit.o.
void RndSceneCom::Init() {
    InitComponentClass<RndSceneCom>();
}
