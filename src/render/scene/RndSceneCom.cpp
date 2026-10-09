// render/RndSceneCom.o (0x407F30 to 0x419170).
#include "render/scene/RndSceneCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "os/memory/MemMgr.h"
#include "os/memory/PoolAlloc.h"
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
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTexture2D.h"
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

// The size of the small-pool blocks the nested poll managers live in.
// Name not in the reference map.
constexpr unsigned long kPollMgrBlockSize = 544;

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
        PollMgr* pollMgr = entry.mPollMgr;
        pollMgr->Reset();
        if (pollMgr != nullptr) {
            pollMgr->~PollMgr();
            PoolFree(kPollMgrBlockSize, pollMgr);
        }
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
      mPollGroup(nullptr),
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
    delete mPollGroup;
    mPollGroup = nullptr;
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
    PollMgr* pollMgr = static_cast<PollMgr*>(PoolAlloc(kPollMgrBlockSize, nullptr));
    new (pollMgr) PollMgr((FormatString("%s PG") << name).Str());
    pollMgr->mSharePollMgr = true;
    job->SetPollWeight(weight, false);
    mPollSplitJobs.push_back(PollSplitJobEntry{job, pollMgr});
}

// Reconstructed from eboot.elf at 0x408A80.
void RndSceneCom::LinkPollSplitJobs() {
    for (unsigned long i = 0; i < mPollSplitJobs.size(); ++i) {
        mPollSplitJobs[i].mPollMgr->AddJob(mPollSplitJobs[i].mJob);
        if (i != 0) {
            mPollSplitJobs[i].mPollMgr->PollAfter(mPollSplitJobs[i - 1].mPollMgr);
        }
    }
}

// Reconstructed from eboot.elf at 0x408B10.
void RndSceneCom::CreatePollGroups(Entity* entity, bool root) {
    mPollGroups = new PollGroups(FileGetName(entity->mResource->mPath.Str()));
    if (root) {
        return;
    }
    mPollGroups->mPollMgrs.find(1)->second->RemovePollAfter(mPollGroups->mPollMgrs.find(0)->second);
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

// Reconstructed from eboot.elf at 0x4116A0. The poll group is named after
// the entity's file.
bool RndSceneCom::_OnResourcesLoaded() {
    mFloats.Resize(4);
    mColors.Resize(4);
    if (mPollGroup == nullptr) {
        mPollGroup = new PollGroup(FileGetName(mObject->mEntity->mResource->mPath.Str()));
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
        canvas.CreateUninitialized(mPropHysteresis.mTextureSize, static_cast<int>(rows), 1);
        for (unsigned long row = 0; row < mPropHysteresis.mEntries.size(); ++row) {
            const Vector4& values = mPropHysteresis.mEntries[row].mDefaultValues;
            for (int x = 0; x < mPropHysteresis.mTextureSize; ++x) {
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

// Reconstructed from eboot.elf at 0x4048E0. The binary emits the factory in
// render/RndInit.o with the class's Init.
Component* RndSceneCom::_Create() {
    return new RndSceneCom();
}
