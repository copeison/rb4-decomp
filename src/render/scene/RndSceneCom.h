#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "math/color/Color.h"
#include "math/vector/Vector4.h"
#include "render/scene/RndDrawableEntityCom.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/threading/ThreadedJob.h"

class Entity;
class EntityResource;
class PollGroup;
class PollMgr;
class RndAtmosphereCom;
class RndCMAACom;
class RndCameraCom;
class RndContext;
class RndLightMgrCom;
class RndPostProcCom;
class RndSceneDrawer;
class RndSkyCom;
class RndTexture2D;
class RndVolumetricScatteringCom;
class TimelinesCom;

// The scene settings component on a scene entity's root object
// (render/RndSceneCom.o, 0x407F30-0x419170). Its class id is "Scene". It
// names the scene's cameras, light manager, sky, atmosphere, post-processing
// and antialiasing objects; holds the shader-graph globals, the property
// hysteresis texture and the LOD settings; owns the scene drawer; and
// splits the scene's poll into jobs. The vtable at 0x18FFA70 has 41 slots.
// The object is 400 bytes.
class RndSceneCom : public RndDrawableEntityCom {
public:
    // The "mask_buffer" settings. Name and field names not in the reference
    // map; the fields follow the properties.
    struct MaskBuffer {
        bool mEnabled;             // "enabled"
        bool mCalcSignedDistance;  // "calc_signed_distance"
        int mSignedDistanceWidth;  // "signed_distance_width"
    };

    // The map's PropRef (entity/PropRef.o), a reference to a property of a
    // component of an object, which is not modelled under src/entity. These
    // are its members; the default constructor (PropRef::PropRef() at
    // 0x17F4F0, which the arrays' element constructors call) leaves no
    // object, no component and an empty path. Name not in the reference
    // map.
    struct PropRefData {
        PropRefData() : mObject{0xFFFFFFFFU} {}

        GameObjectId mObject;
        Symbol mCom;
        PropPath mPath;
    };

    // A value a property can drive: the value, the driving property and
    // whether it drives. The registrations at 0x4107C0 and 0x410EF0, which
    // other classes call as well, describe them ("value", the property and
    // "is_driven"). Names not in the reference map.
    struct DrivenFloat {
        DrivenFloat() : mValue(0.0F), mIsDriven(false) {}

        float mValue;
        PropRefData mProp;
        bool mIsDriven;
    };
    struct DrivenColor {
        DrivenColor() : mValue(0.0F, 0.0F, 0.0F, 0.0F), mIsDriven(false) {}

        Hmx::Color mValue;
        PropRefData mProp;
        bool mIsDriven;
    };

    // The elements of the shader-graph globals' "floats" and "colors"
    // arrays: a "comment" and the value. Their construction is inlined into
    // the arrays' Construct (0x412CE0 and its colour counterpart). Names
    // not in the reference map.
    struct GlobalFloat {
        Symbol mComment;
        DrivenFloat mValue;
    };
    struct GlobalColor {
        Symbol mComment;
        DrivenColor mValue;
    };

    // One row of the property hysteresis texture: the "default_values"
    // and the four properties ("prop_0" to "prop_3") whose values fill it.
    // Inlined into the array's Construct (0x413390).
    struct PropHysteresisEntry {
        PropHysteresisEntry() : mDefaultValues(Vector4::sZero) {}

        Vector4 mDefaultValues;
        PropRefData mProps[4];
    };

    // The "prop_hysteresis" settings: the clock that advances the texture,
    // its width and sampling, and its rows. Name and field names not in the
    // reference map; the fields follow the properties.
    struct PropHysteresis {
        // Inlined into RndSceneCom's constructor (0x4080E0).
        PropHysteresis() : mTimeline(0), mTextureSize(64), mWrapMode(1), mFilterMode(2) {}
        // Copies the settings and, through PropArrayBase::_Copy, the rows.
        // Inlined into RndSceneCom's copy constructor.
        PropHysteresis(const PropHysteresis& other)
            : mTimeline(other.mTimeline),
              mTextureSize(other.mTextureSize),
              mWrapMode(other.mWrapMode),
              mFilterMode(other.mFilterMode) {
            mEntries._Copy(other.mEntries);
        }

        int mTimeline;     // "timeline", a TimeUnits.
        int mTextureSize;  // "texture_size", the width in texels.
        int mWrapMode;     // "wrap_mode"
        int mFilterMode;   // "filter_mode"
        PropArray<PropHysteresisEntry> mEntries;  // "entries"
    };

    // A job that polls one window of the scene entity's poll order on a
    // worker, under a nested poll manager (AddPollSplitJob). Name not in
    // the reference map. The vtable is at 0x18FFBC8.
    class PollSplitJob : public ThreadedJob {
    public:
        // Inlined into AddPollSplitJob (0x408810), which then names the
        // job's perf timer.
        PollSplitJob(
            EntityResource* resource,
            Entity* entity,
            int index,
            int pollWindowStart,
            int pollWindowEnd)
            : mResource(resource),
              mEntity(entity),
              mIndex(index),
              mPollWindowStart(pollWindowStart),
              mPollWindowEnd(pollWindowEnd),
              mTimerIndex(static_cast<unsigned long>(-1)) {
            SetThreaded(false);
        }
        ~PollSplitJob() override;  // slots 0-1: 0x412000, 0x412010

        // Slot 2: polls only while the entity does.
        bool IsPollEnabled() const override;  // 0x412030
        // Slot 10: polls the window of the entity's poll order under the
        // audio lock the window asks for. Not reconstructed: the audio lock
        // switch (0x24F550) and the entity's poll window calls (0xF3F10,
        // 0xF3EF0) are not modelled.
        void _DoPoll() override;  // 0x407F30
        // Slot 11 at 0x412070: empty.
        void _DoPostPoll() override {}

        // Field names are not in the reference map.
        EntityResource* mResource;
        Entity* mEntity;
        // The job's place in the split; job 0 polls under the audio lock
        // when "use_pollmgr_audio_lock" is set, job 1 without it.
        int mIndex;
        int mPollWindowStart;
        int mPollWindowEnd;
        // The perf timer "poll-split-job: <index>".
        unsigned long mTimerIndex;
    };

    // A split job and the nested manager it polls under. Name and field
    // names not in the reference map.
    struct PollSplitJobEntry {
        PollSplitJob* mJob;
        PollMgr* mPollMgr;
    };

    // The scene's two nested poll managers, "<file> SceneDrawer PG" (key
    // 0) and "<file> Scene PG" (key 1); the scene's polls wait for the
    // drawer's. The vtable is at 0x18FFC38, over a base at 0x18FFC60 that
    // holds the map. Not reconstructed. Name not in the reference map.
    class PollGroups {
    public:
        explicit PollGroups(const char* path);  // 0x4121A0
        // Slot 0: resets each manager and links the scene's after the
        // drawer's again.
        virtual void Reset();  // 0x412450
        virtual ~PollGroups();  // slots 1-2: 0x412540, 0x412550

        eastl::map<int, PollMgr*> mPollMgrs;
    };

    RndSceneCom();  // 0x4080E0
    // Copies the properties for an imprint, the arrays through
    // PropArrayBase::_Copy. The run-time state starts afresh, and so does
    // "isolate_lod". Inlined into _Imprint (0x411CE0). Not in the reference
    // map.
    RndSceneCom(const RndSceneCom& other)
        : RndDrawableEntityCom(other),
          mDrawOrder(other.mDrawOrder),
          mClearType(other.mClearType),
          mClearColor(other.mClearColor),
          mClearDepthAfterPostProc{
              other.mClearDepthAfterPostProc[0],
              other.mClearDepthAfterPostProc[1]},
          mCameras{other.mCameras[0], other.mCameras[1]},
          mLightMgr(other.mLightMgr),
          mSky(other.mSky),
          mAtmosphere(other.mAtmosphere),
          mPostProcs{other.mPostProcs[0], other.mPostProcs[1]},
          mAntialiasing(other.mAntialiasing),
          mMaskBuffer(other.mMaskBuffer),
          mShaderGraphTrans{
              other.mShaderGraphTrans[0],
              other.mShaderGraphTrans[1],
              other.mShaderGraphTrans[2],
              other.mShaderGraphTrans[3]},
          mPropHysteresis(other.mPropHysteresis),
          mHighLodDistance(other.mHighLodDistance),
          mMediumLodDistance(other.mMediumLodDistance),
          mLowLodDistance(other.mLowLodDistance),
          mEnableLod(other.mEnableLod),
          mNumDrawEntries(other.mNumDrawEntries),
          mNeverCull(other.mNeverCull),
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
        mFloats._Copy(other.mFloats);
        mColors._Copy(other.mColors);
    }
    ~RndSceneCom() override;  // slots 0-1: 0x4083D0, 0x4087F0

    Symbol GetId() const override;         // slot 4: 0x411C60
    Symbol GetClassName() const override;  // slot 5: 0x411C70
    int CurrentRev() const override;       // slot 7: 0x411C80
    bool IsA(Symbol type) const override;  // slot 8: 0x411CA0
    Component* AsComponent() override;     // slot 9: 0x411CD0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x411CE0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x411FE0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x411FF0
    // Slot 29: sizes the globals, creates the poll group and the scene
    // drawer, finds a light manager when none is named, creates the
    // property hysteresis texture and finds the entity's clocks. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0x4116A0
    // Slots 31-33: entering and polling tell the clocks whether the scene
    // runs at a partial frame rate; exiting destroys the poll groups.
    void _Enter() override;                  // 0x411A70
    void _Exit(DestroyType type) override;   // 0x411AB0
    void _Poll() override;                   // 0x411B00

    // The camera (0) or aux camera (1) component, or null. The map's
    // GetCameraObj(EntityPtr const&) const returns the object; this build
    // returns the component, in a const and a non-const copy.
    RndCameraCom* GetCamera(int index) const;  // 0x408D90
    RndCameraCom* GetCamera(int index);        // 0x408E20
    // Names the camera's object as camera `index`, or none.
    // Name not in the reference map.
    void SetCamera(int index, RndCameraCom* camera);  // 0x408D70
    // The light manager component of the object the scene names, or null.
    // The map's signature is GetLightMgr(EntityPtr const&); this build
    // reads the entity through the component's owner. The const copy is at
    // 0x408ED0.
    RndLightMgrCom* GetLightMgr() const;  // 0x408ED0
    RndLightMgrCom* GetLightMgr();        // 0x408F60
    // Names the light manager's object. Name not in the reference map.
    void SetLightMgr(RndLightMgrCom* lightMgr);  // 0x408EB0
    // The sky component; the const copy is at 0x409010. Names not in the
    // reference map.
    RndSkyCom* GetSky() const;  // 0x409010
    RndSkyCom* GetSky();        // 0x4090A0
    void SetSky(RndSkyCom* sky);  // 0x408FF0
    // The atmosphere object's component with the atmosphere interface.
    // The non-const copy is at 0x4091F0.
    RndAtmosphereCom* GetAtmosphere();  // 0x4091F0
    // Names the atmosphere's object. Name not in the reference map.
    void SetAtmosphere(RndAtmosphereCom* atmosphere);  // 0x409130
    // The atmosphere object's volumetric scattering component.
    RndVolumetricScatteringCom* GetVolumetricScattering();  // 0x4093B0
    // Post-processing chain `index` (0 or 1); the const copy is at
    // 0x4094D0.
    RndPostProcCom* GetPostProc(unsigned long index) const;  // 0x4094D0
    RndPostProcCom* GetPostProc(unsigned long index);        // 0x409560
    // The const copy is at 0x4095F0.
    RndCMAACom* GetAntialiasing() const;  // 0x4095F0
    RndCMAACom* GetAntialiasing();        // 0x409680

    // The entity's frame-rate type, its clocks' mode while the renderer
    // runs partial-framerate scenes (0 full, 1 half), and 0 otherwise.
    int GetFramerateType() const;  // 0x408C10
    // Advances the frame-interval counter through the frames of one
    // interval of the frame-rate type (one frame at full rate, two at
    // half) and returns 1 when it wrapped, starting a new interval.
    unsigned long StepFrameInterval();  // 0x408CD0

    // Adds a split job for the poll window, under a new nested manager
    // ("<name> PG") that shares the releasing job's manager. Called by the
    // scene resource's poll. Name not in the reference map.
    void AddPollSplitJob(
        const char* name,
        int pollWindowStart,
        int pollWindowEnd,
        unsigned int weight);  // 0x408810
    // Queues each split job on its manager, each manager after the
    // previous. Name not in the reference map.
    void LinkPollSplitJobs();  // 0x408A80
    // Destroys the split jobs and their managers. Name not in the
    // reference map.
    void ClearPollSplitJobs();  // 0x408600
    // Creates the poll groups; a scene below another stops its polls from
    // waiting for its drawer's. Name not in the reference map.
    void CreatePollGroups(Entity* entity, bool root);  // 0x408B10

    // Has the scene drawer forget its draw instance registrations. Called
    // by game code (0x8C99B3). Name not in the reference map.
    void ResetDrawRegistrations();  // 0x411AF0

    // Steps the property hysteresis texture by the clock, writing a column
    // of each row's property values per elapsed texel. The map's
    // UpdatePropHysteresisTexture(EntityPtr const&, RndContext&). Not
    // reconstructed: the property lookups it inlines are not modelled.
    void UpdatePropHysteresisTexture(Entity* entity, RndContext& context);  // 0x409710
    // Recreates the property hysteresis texture, one row per entry
    // filled with its default values. The map has
    // _CreatePropHysteresisTexture() and _ClearPropHysteresisTexture().
    void _CreatePropHysteresisTexture();  // 0x411880

    // Registers the class. Inline in the map (render/RndInit.o's
    // RndSceneCom::Init); not reconstructed.
    static void Init();
    // The class factory. Emitted in render/RndInit.o at 0x4048E0.
    static Component* _Create();
    // Describes the class and registers its properties: draw_order,
    // clear_type, clear_color, the clear_depth group (after_post_proc_0
    // and 1), framerate_deprecated, camera, aux_camera, light_manager, sky,
    // atmosphere, post_proc_0 and 1, antialiasing, mask_buffer, the globals
    // group (floats, colors), shadergraph_data (trans_0 to trans_3),
    // prop_hysteresis, the lod group (enable_lod, medium_lod_distance,
    // low_lod_distance, isolate_lod), never_cull, num_draw_entries and the
    // utilities. The element types are registered by 0x410090, 0x410240
    // and 0x4103F0. Not reconstructed: the property metadata it fills is
    // not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x40B490

    static Symbol sId;  // 0x1A722A8, "Scene"
    // Also "Scene". Name not in the reference map.
    static Symbol sClassName;           // 0x1A722B0
    static PropRegistry sPropRegistry;  // 0x1A722C0
    static ComMetaData sMetaData;       // 0x1A72360

    // Field names are not in the reference map; they follow the
    // properties where there is one.
    int mDrawOrder;          // "draw_order"
    int mClearType;          // "clear_type"
    Hmx::Color mClearColor;  // "clear_color"
    // "after_post_proc_0" and "_1", in the "clear_depth" group: whether
    // each post-processing chain clears the depth after it runs. Weak
    // evidence.
    bool mClearDepthAfterPostProc[2];
    GameObjectId mCameras[2];        // "camera", "aux_camera"
    GameObjectId mLightMgr;          // "light_manager"
    GameObjectId mSky;               // "sky"
    GameObjectId mAtmosphere;        // "atmosphere"
    GameObjectId mPostProcs[2];      // "post_proc_0", "post_proc_1"
    GameObjectId mAntialiasing;      // "antialiasing"
    MaskBuffer mMaskBuffer;          // "mask_buffer"
    PropArray<GlobalFloat> mFloats;  // "floats"
    PropArray<GlobalColor> mColors;  // "colors"
    // "trans_0" to "trans_3": the objects whose transforms the shader
    // graph receives as TransInfo #0 to #3.
    GameObjectId mShaderGraphTrans[4];
    PropHysteresis mPropHysteresis;  // "prop_hysteresis"
    // Zero; not registered. Presumably the start of the high LOD band;
    // the evidence is weak.
    float mHighLodDistance;
    float mMediumLodDistance;      // "medium_lod_distance"
    float mLowLodDistance;         // "low_lod_distance"
    bool mEnableLod;               // "enable_lod"
    unsigned long mNumDrawEntries;  // "num_draw_entries"
    bool mNeverCull;               // "never_cull"
    // The scene entity's poll group, created when the resources load.
    PollGroup* mPollGroup;
    eastl::vector<PollSplitJobEntry> mPollSplitJobs;
    // Created by the resource's first immediate enter.
    PollGroups* mPollGroups;
    // Created when the resources load and owned.
    RndSceneDrawer* mSceneDrawer;
    // The interval index StepFrameInterval keeps; -1 before the first
    // step. The drawer selects the partial-framerate buffers by it.
    unsigned long mFrameInterval;
    // The scene's partial-framerate buffer index, which the drawer passes
    // to RndBufferCollection. Its writer is not identified.
    long mPartialFramerateScene;
    int mIsolateLod;  // "isolate_lod", -1 for none.
    // The device's frame count when the current interval started
    // (RndSceneResource's poll).
    long mIntervalStartFrame;
    RndTexture2D* mPropHysteresisTexture;
    // The hysteresis texture's fractional texel position.
    float mPropHysteresisPosition;
    // Set when the resource enters the entity (RndSceneResource slots 16
    // and 19) and cleared by its immediate enter; the first ready entity
    // unhooks the poll groups and split jobs.
    bool mEntered;
    // The root's Timelines component, or null.
    TimelinesCom* mTimelines;
};

static_assert(sizeof(RndSceneCom::MaskBuffer) == 8);
static_assert(sizeof(RndSceneCom::PropRefData) == 216);
static_assert(sizeof(RndSceneCom::DrivenFloat) == 0xE8);
static_assert(sizeof(RndSceneCom::DrivenColor) == 0xF0);
static_assert(sizeof(RndSceneCom::GlobalFloat) == 0xF0);
static_assert(sizeof(RndSceneCom::GlobalColor) == 0xF8);
static_assert(sizeof(RndSceneCom::PropHysteresisEntry) == 0x370);
static_assert(offsetof(RndSceneCom::PropHysteresis, mEntries) == 0x10);
static_assert(offsetof(RndSceneCom::PollSplitJob, mResource) == 0x90);
static_assert(offsetof(RndSceneCom::PollSplitJob, mIndex) == 0xA0);
static_assert(offsetof(RndSceneCom::PollSplitJob, mTimerIndex) == 0xB0);
static_assert(sizeof(RndSceneCom::PollSplitJob) == 0xB8);
static_assert(sizeof(RndSceneCom::PollGroups) == 0x40);
static_assert(offsetof(RndSceneCom, mDrawOrder) == 0x20);
static_assert(offsetof(RndSceneCom, mClearColor) == 0x28);
static_assert(offsetof(RndSceneCom, mClearDepthAfterPostProc) == 0x38);
static_assert(offsetof(RndSceneCom, mCameras) == 0x3C);
static_assert(offsetof(RndSceneCom, mLightMgr) == 0x44);
static_assert(offsetof(RndSceneCom, mSky) == 0x48);
static_assert(offsetof(RndSceneCom, mAtmosphere) == 0x4C);
static_assert(offsetof(RndSceneCom, mPostProcs) == 0x50);
static_assert(offsetof(RndSceneCom, mAntialiasing) == 0x58);
static_assert(offsetof(RndSceneCom, mMaskBuffer) == 0x5C);
static_assert(offsetof(RndSceneCom, mFloats) == 0x68);
static_assert(offsetof(RndSceneCom, mColors) == 0x90);
static_assert(offsetof(RndSceneCom, mShaderGraphTrans) == 0xB8);
static_assert(offsetof(RndSceneCom, mPropHysteresis) == 0xC8);
static_assert(offsetof(RndSceneCom, mHighLodDistance) == 0x100);
static_assert(offsetof(RndSceneCom, mEnableLod) == 0x10C);
static_assert(offsetof(RndSceneCom, mNumDrawEntries) == 0x110);
static_assert(offsetof(RndSceneCom, mNeverCull) == 0x118);
static_assert(offsetof(RndSceneCom, mPollGroup) == 0x120);
static_assert(offsetof(RndSceneCom, mPollSplitJobs) == 0x128);
static_assert(offsetof(RndSceneCom, mPollGroups) == 0x148);
static_assert(offsetof(RndSceneCom, mSceneDrawer) == 0x150);
static_assert(offsetof(RndSceneCom, mFrameInterval) == 0x158);
static_assert(offsetof(RndSceneCom, mPartialFramerateScene) == 0x160);
static_assert(offsetof(RndSceneCom, mIsolateLod) == 0x168);
static_assert(offsetof(RndSceneCom, mIntervalStartFrame) == 0x170);
static_assert(offsetof(RndSceneCom, mPropHysteresisTexture) == 0x178);
static_assert(offsetof(RndSceneCom, mPropHysteresisPosition) == 0x180);
static_assert(offsetof(RndSceneCom, mEntered) == 0x184);
static_assert(offsetof(RndSceneCom, mTimelines) == 0x188);
static_assert(sizeof(RndSceneCom) == 0x190);
