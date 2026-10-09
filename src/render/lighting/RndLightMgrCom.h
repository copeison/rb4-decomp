#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "render/lighting/tiled/RndTiledLightsComputeBuffer.h"
#include "utl/containers/Vector.h"
#include "utl/containers/VectorAdapter.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class Frustum;
class RndFence;
class RndScenePartialFramerateData;
class RndShaderCBuffer;
class Vector2i;
struct RndDrawInstance;
struct RndSceneCullResults;
struct RndShowHideContext;
template <typename T>
class PodVector;
template <typename T>
struct AllowedValue;
class RndBufferCollection;
class RndCameraContext;
class RndComputeBuffer;
class RndContext;
class RndLightCom;
class RndLightEnvironCom;
class RndLightProbeCom;
class RndPixelData;
class RndPixelDataCube;
class RndSceneDrawParams;
class RndSceneDrawer;
class RndSSAOCom;
struct RndSceneDrawTarget;
class RndTextureArray2D;
class RndTextureArrayCube;
class RndTextureBase;

// The scene's light manager (render/RndLightMgrCom.o, 0x480280 to
// 0x48ED8F). Its class id is "LightMgr" ("Manages global lighting settings
// for an entire scene"). The lights, probes and environments of the scene
// register with it; each frame it culls the lights against the camera,
// fills the tiled lights' buffers, assigns the shadow maps and
// contributions, accumulates the deferred lighting and probes and
// tonemaps the scene. The vtable at 0x1906480 has 41 slots. The object is
// 1856 bytes. Field names are not in the reference map; the property
// members take the names of the properties the registry (0x486830) binds to
// their offsets. The map's RuntimeData (constructor 0x4803B0, destructor
// 0x48CD50) holds the members from mCulling at 200; they are declared in
// the class because the light components read them by name.
class RndLightMgrCom : public Component {
public:
    // One element of "quality_settings", per RndQualityLevel. Name not in
    // the reference map.
    struct QualitySettings {
        // "max_shadow_casting_spotlights": the layers of the spot shadow
        // depth array, which the shadow-casting lights share.
        std::uint64_t mMaxShadowCastingSpotlights;
        // "spotlight_shadowmap_resolution": an index into the resolutions
        // ("Use %dx%d spotlight shadowmaps").
        std::uint32_t mSpotlightShadowmapResolution;
        // "tonemapping_enabled".
        bool mTonemappingEnabled;
    };

    // A cookie texture's place in the cookie texture arrays. The map names
    // the type; its field names are not in the reference map, and the
    // second field's meaning is weakly supported.
    struct CookieArrayElemInfo {
        RndTextureBase* mTexture;
        unsigned long mSlice;
    };

    // A probe state renamed in the editor, applied by the next poll: its
    // old name and its index in "states". The map names the type
    // (RuntimeData::RenamedProbeState); the field names are not in the
    // reference map.
    struct RenamedProbeState {
        Symbol mOldName;
        unsigned long mIndex;
    };

    // The lights and probes the culling keeps for one camera:
    // per light type (RndLightCom::_GetTypeImpl), the lights that add light
    // and those that subtract it (illumination type 3); the probes; and
    // the lights given a shadow contribution this frame.
    // RndScenePartialFramerateData keeps a copy (StoreCullResults). Name not
    // in the reference map.
    // The binary emits the implicit copy assignment at 0x484DA0.
    struct CullResults {
        // Reserves each list for the other's size. Name not in the
        // reference map.
        void ReserveFor(const CullResults& other);  // 0x48B080

        eastl::vector<RndLightCom*> mLights[3][2];
        eastl::vector<RndLightProbeCom*> mProbes;
        eastl::vector<RndLightCom*> mShadowLights;
    };

    RndLightMgrCom();  // 0x480280

    // Slots 0-1: 0x480AD0, 0x480D60.
    ~RndLightMgrCom() override;
    // Slot 2: "leaving_playmode" and "leaving_record_mode" mark the cookie
    // texture arrays for a rebuild; any other message is unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x48C8D0
    Symbol GetId() const override;          // slot 4: 0x48CA50
    Symbol GetClassName() const override;   // slot 5: 0x48CA60
    int CurrentRev() const override;        // slot 7: 0x48CA70
    bool IsA(Symbol type) const override;   // slot 8: 0x48CA90
    Component* AsComponent() override;      // slot 9: 0x48CAC0
    // Copies the properties; the copy's run-time state starts empty. The
    // map's RndLightMgrCom(RndLightMgrCom const&); inlined into _Imprint.
    RndLightMgrCom(const RndLightMgrCom& other);
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x48CAD0
    // Slot 19 at 0x489F70: the default probe state and one quality setting
    // per quality level.
    void _PostCreate() override;
    // Slot 20 at 0x48A1A0: empty.
    void _PreDestroy(DestroyType type) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x48CCD0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x48CCE0
    // Slot 29 at 0x48A1B0: creates the buffers and moves the probe states
    // of an old scene from its probes into "states". The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 30 at 0x480D80. This build passes the load pass (0 to 9) that
    // Entity::_LoadResources (0xEEA60) gives Component::AreResourcesReady
    // (0xE8250), which forwards it in esi; Component's declaration lacks
    // it. The light manager reports not ready on pass 0; on pass 1 it
    // counts the lights per type and illumination, reserves both light
    // lists for them and the probes (0x6DB070), syncs the cookie texture
    // arrays, rebuilds the probe texture arrays with tiled lighting, and
    // reports ready. Not reconstructed until the parameter is declared.
    bool _AreResourcesReady() override;
    // Slot 32 at 0x48A640: forgets the environments, lights and probes;
    // an exit of instanced entities (types 2-4) also unregisters the
    // environments.
    void _Exit(DestroyType type) override;
    // Slot 33 at 0x48A6E0: applies the default environment and the renamed
    // probe states, and rebuilds the cookie, probe and spot shadow texture
    // arrays when needed.
    void _Poll() override;
    // Slots 37-38 at 0x48ADA0 and 0x48AD90: as in the game mode.
    void _EditExit(DestroyType type) override;
    void _EditPoll() override;

    // The "leaving_playmode" and "leaving_record_mode" handler, inlined
    // into Handle. Name not in the reference map.
    DataNode _OnLeavingPlayMode();  // 0x48C8B0

    // Registers an environment, the default one first; unregisters it.
    // The map's signatures take an ObjPtr const&.
    void AddEnviron(RndLightEnvironCom* environ);     // 0x482210
    void RemoveEnviron(RndLightEnvironCom* environ);  // 0x4824D0
    // Looks up the environments' components from `first` to `last` and
    // gives each its index; at most six environments are indexed. The
    // map's signature starts with an EntityPtr const&.
    void _ReindexEnvirons(unsigned long first, unsigned long last);  // 0x4822C0
    // The default environment's RndLightEnvironCom, or null. The map's
    // signature is GetDefaultEnviron(EntityPtr const&) const.
    RndLightEnvironCom* GetDefaultEnviron() const;  // 0x482770
    void SetDefaultEnvironId(GameObjectId id);       // 0x482800
    // Adds the light, copies "master_intensity_mult" into it and marks the
    // cookie texture arrays for a rebuild when it has a cookie. The map's
    // AddLight(ObjPtr const&, ObjPtr const&).
    void AddLight(RndLightCom* light);  // 0x482810
    // Removes the light, forgets it as the isolated light and clears its
    // cookie slot. The map's RemoveLight(ObjPtr const&, ObjPtr const&).
    void RemoveLight(RndLightCom* light);  // 0x482920
    // Adds a probe to the probe list and its object id to "probes", and
    // marks the probe texture arrays for a rebuild; removes it again. The
    // map's signatures start with an ObjPtr const& and take the probe's
    // ObjPtr.
    void AddProbe(RndLightProbeCom* probe);     // 0x4829F0
    void RemoveProbe(RndLightProbeCom* probe);  // 0x482B30
    // Isolates the probe, or none for null: stores its object id in
    // mIsolatedProbe and the probe after it. The map's signature takes an
    // ObjPtr const&.
    void SetIsolatedProbe(RndLightProbeCom* probe);  // 0x482C80
    // Copies "master_intensity_mult" into every light. Name not in the
    // reference map.
    void _SyncMasterIntensityMult();  // 0x48AE40

    // Culls the lights and probes for the camera, then reserves the shadow
    // maps and contributions of the lights that cast shadows.
    // The map's CullLights(RndContext&, RndCameraContext const&,
    // RndSceneDrawParams const&); this build takes the scene draw id and
    // index that RndCuller::Cull also takes, and passes them on unused.
    void CullLights(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const RndCameraContext& camera,
        const RndSceneDrawParams& params);  // 0x482CB0
    // Fills mCullResults with the visible lights and probes and gives each
    // environment its culled lights; for a camera whose target mode is 1,
    // also fills mSliceZeroCullResults for the first depth slice.
    void _FrustumCullLights(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const RndCameraContext& camera,
        const RndSceneDrawParams& params);  // 0x482EB0
    // Writes the culled lights of each type into their light buffer and the
    // probes into the probe buffer, giving each its index, and uploads the
    // buffers. A full buffer drops the remaining lights of its type with a
    // warning naming the type (compiled out).
    void _FillTiledLightBuffers(RndContext& ctx, const RndCameraContext& camera);  // 0x484160
    // Writes the buffer indices of the first depth slice's probes and
    // lights into the slice-zero id buffer and uploads it. Name not in the
    // reference map.
    void _FillSliceZeroLightIds(RndContext& ctx);  // 0x484410
    // Takes the next shadow-contribution slot this frame, or -1 when the
    // device's shadow-contribution buffers are used up. Name not in the
    // reference map.
    long AcquireShadowContribution();  // 0x485FC0
    // Takes `count` consecutive layers of the spot shadow depth array this
    // frame and returns the first, or -1 (with a warning naming the quality
    // level) when the active configuration has too few. Name not in the
    // reference map.
    long AcquireSpotShadowDepthLayers(unsigned long count);  // 0x486000
    // Draws the shadow map of the shadow light at `index` under the GPU
    // timers "Shadows" and "Shadow Map Generation". The map's
    // DrawShadowMaps(RndContext&, RndSceneDrawer&, RndCameraContext const&);
    // this build draws one light per call and passes the scene's show and
    // hide flags and the drawer's instance lists through.
    void DrawShadowMaps(
        RndContext& ctx,
        RndSceneDrawer& drawer,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
        unsigned long index);  // 0x485E40
    // Saves mLightsCulled and mCullResults for the following frames of a
    // partial-framerate scene. The map's signature is
    // StoreCullResults(RndScenePartialFramerateData&) const; this build
    // keeps the lights in the drawer's cull results.
    void StoreCullResults(
        RndScenePartialFramerateData& data,
        RndSceneCullResults& results) const;  // 0x484D80
    // Restores what StoreCullResults saved and refills the light buffers
    // for the camera. The map's signature is RestoreCullResults(
    // RndContext&, RndCameraContext const&,
    // RndScenePartialFramerateData const&).
    void RestoreCullResults(
        RndContext& ctx,
        const RndCameraContext& camera,
        RndScenePartialFramerateData& data,
        RndSceneCullResults& results);  // 0x485220
    // With tiled lighting, fills the light buffers (and, for target mode
    // 1, the first depth slice's ids) and culls the lights into the screen
    // tiles of the collections; the second is the right eye of a stereo
    // pair. The map's signature is _CullTiledLights(RndContext&,
    // RndCameraContext const&, RndSceneDrawParams const&).
    void _CullTiledLights(
        RndContext& ctx,
        const RndCameraContext& camera,
        const RndSceneDrawParams& params,
        RndBufferCollection& buffers,
        RndBufferCollection* rightEyeBuffers);  // 0x4840D0
    // The cull pass of _CullTiledLights ("Lighting"): clears the tile-list
    // counter and dispatches RndCShaderTiledLightsCull into the
    // collections. Name not in the reference map.
    void _DispatchTiledLightCull(
        RndContext& ctx,
        const RndCameraContext& camera,
        const RndSceneDrawParams& params,
        RndBufferCollection& buffers,
        RndBufferCollection* rightEyeBuffers);  // 0x484620
    // Culls the first depth slice's lights for one eye's camera into the
    // eye's collection, then splits the stereo tile lists of `source` into
    // it ("Lighting"). The light buffers are refilled unless the eye's
    // target mode is 3. Name not in the reference map; the evidence is
    // weak.
    void _CullTiledLightsStereo(
        RndContext& ctx,
        const RndSceneDrawParams& params,
        const RndCameraContext& stereoCamera,
        const RndCameraContext& eyeCamera,
        RndBufferCollection& source,
        RndBufferCollection& target);  // 0x484980

    // Generates the shadow contributions of the lights that add light
    // ("Shadows"), then accumulates the lighting into the source light
    // buffer ("Lighting"), tiled or untiled. The map's signature is
    // AccumDeferredLight(RndContext&, RndSceneDrawParams const&); this
    // build passes the camera, the collection, the draw target, the
    // source light buffer and a fence the tiled pass waits on.
    void AccumDeferredLight(
        RndContext& ctx,
        const RndCameraContext& camera,
        const RndSceneDrawParams& params,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* srcLightAccum,
        RndFence* fence);  // 0x485270
    // Shades the screen tiles with RndCShaderTiledLightsApplication,
    // interpolates the skipped pixels when enabled, and swaps the draw
    // target's light buffers.
    void _AccumTiledDeferredLight(
        RndContext& ctx,
        const RndCameraContext& camera,
        const RndSceneDrawParams& params,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* srcLightAccum,
        RndFence* fence);  // 0x485590
    // Draws each light and the probes into the source light buffer without
    // the compute path. With "combine_single_probe" set and one culled
    // probe, the directional lights draw the probe with them.
    void _AccumUntiledDeferredLight(
        RndContext& ctx,
        const RndSceneDrawParams& params,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* srcLightAccum);  // 0x4859B0
    // "Accumulate Probes": one probe at full intensity draws straight into
    // the light buffer; otherwise the probes accumulate into the light
    // probe buffer, which is added scaled by "master_intensity_mult".
    // `params` and `target` are not read.
    void _AccumUntiledDeferredProbes(
        RndContext& ctx,
        const RndSceneDrawParams& params,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* srcLightAccum);  // 0x48B300
    // Runs the "ambient_occlusion" object's RndSSAOCom when the platform
    // has async compute and it is enabled for the quality level; records
    // whether it ran in mAmbientOcclusionGenerated. The map's signature
    // starts with an EntityPtr const&.
    void GenerateAmbientOcclusion(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndCameraContext& camera,
        const RndSceneDrawParams& params);  // 0x486060
    // The "ambient_occlusion" object's RndSSAOCom, or null. The map's
    // _GetAmbientOcclusion(EntityPtr const&); inlined into
    // GenerateAmbientOcclusion.
    RndSSAOCom* _GetAmbientOcclusion() const;
    // With tiled lighting, writes the screen's light-tile counts and the
    // number of directional lights to the forward-lighting constants.
    void SetFwdLightingConstants(
        const Vector2i& size,
        RndShaderCBuffer& cbuffer) const;  // 0x486200
    // Writes zeros to the forward-lighting constants.
    static void SetNoFwdLightingConstants(
        RndShaderCBuffer& cbuffer);  // 0x4862B0
    // Whether the tonemapping does anything: the quality level enables it,
    // "tonemapping" is set, and the exposure differs from 1 or the
    // operator is Power or Linear/Power Hybrid. Inlined into TonemapScene
    // and UpdateDrawTarget. Name not in the reference map.
    bool _TonemapsScene() const;
    // "Tonemapping": draws the source light buffer through the tonemap
    // shader into the destination and swaps the draw target's light
    // buffers. The map's signature is TonemapScene(RndContext&,
    // RndSceneDrawParams&).
    void TonemapScene(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target);  // 0x4862F0
    // Swaps the draw target's light buffers as TonemapScene would, for the
    // threaded scene draw. Name not in the reference map; it follows
    // RndPostProcCom::UpdateDrawTarget.
    void UpdateDrawTarget(RndSceneDrawTarget& target) const;  // 0x4867C0

    // Creates the tiled-light buffers when tiled lighting is enabled, then
    // the spot shadow depth array.
    void _InitBuffers();  // 0x48A400
    // Recreates the spot shadow depth array for the active configuration.
    // Name not in the reference map.
    void _SyncSpotShadowDepthTexArray();  // 0x48AB30
    // Looks up the indices of "cur_state_a" and "cur_state_b" in "states",
    // or -1.
    void _SyncCurProbeStateIndices();  // 0x48A5A0
    // Inserts a probe state and tells the probes.
    void _InsertProbeState(unsigned long index, Symbol name);  // 0x48A090
    // Removes a probe state and tells the probes; the current and capture
    // states fall back to no state when theirs is gone.
    void _RemoveProbeState(unsigned long index);  // 0x48BB20
    // The values "cur_state_a", "cur_state_b" and "capture/state" offer:
    // no state, then each probe state.
    void GetAllowedProbeStates(
        eastl::vector<AllowedValue<Symbol>>& values) const;  // 0x48BD10
    // Captures every probe for "capture/state", "capture/num_bounces"
    // times, with the lights at full intensity. The map's signature takes
    // an ObjPtr const&.
    void CaptureAllProbes();  // 0x48BEE0
    // Empties every probe's capture for "capture/state". The map's
    // signature takes an ObjPtr const&.
    void ZeroAllProbes();  // 0x48C3C0
    // Rebuilds the probe texture arrays with tiled lighting and clears
    // mProbeTexArraysDirty. Name not in the reference map; inlined into
    // its callers.
    void _RebuildProbeTexArrays();
    // Rebuilds the cookie texture arrays from the lights' cookies and gives
    // each light its slice.
    void _SyncCookieTexArrays();  // 0x480F10
    // The level of a cookie's pixels that is "cookie_texture_size" square,
    // else that level of the error cookie, else `scratch` made black at
    // that size. `name` is the cookie's and is not read. Name not in the
    // reference map.
    const RndPixelData* _GetCookieArrayPixels(
        const char* name,
        int dataFormat,
        const RndPixelData& pixels,
        RndPixelData& scratch) const;  // 0x48B870
    // The same for each face of a cube cookie, copied into `cube`. Name
    // not in the reference map.
    RndPixelDataCube& _GetCookieArrayPixels(
        const char* name,
        int dataFormat,
        const RndPixelDataCube& pixels,
        RndPixelDataCube& cube) const;  // 0x48B9D0

    // Not reconstructed: the property metadata, and the std::function
    // handlers it binds, are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x486830

    // The class symbol, "LightMgr", constructed by the static initializer
    // at 0x48ECC0.
    static Symbol sId;  // 0x1A88308
    // The class symbol the component factory is given, also "LightMgr".
    // Name not in the reference map.
    static Symbol sClassName;  // 0x1A88310
    static PropRegistry sPropRegistry;  // 0x1A88320
    static ComMetaData sMetaData;       // 0x1A883C0

    // "default_environment" and "ambient_occlusion": object ids.
    GameObjectId mDefaultEnvironment;
    GameObjectId mAmbientOcclusion;
    float mMasterIntensityMult;
    // "tonemapping": 0 none, 1 before the first post-process and 2 before
    // the second.
    std::int32_t mTonemapping;
    // "tonemapping_operator": Kevin's, Linear, Power or Linear/Power Hybrid.
    std::int32_t mTonemappingOperator;
    float mTonemappingExposure;
    float mTonemappingPower;
    float mMaterialSmoothnessAdjustment;
    std::uint64_t mCookieTextureSize;
    // "probe_lighting": the probe states, the two blended states and the
    // blend.
    PropArray<Symbol> mProbeStates;
    Symbol mCurStateA;
    Symbol mCurStateB;
    float mCurStateBlend;
    // "capture/state" and "capture/num_bounces".
    Symbol mCaptureState;
    std::uint64_t mCaptureNumBounces;
    PropArray<QualitySettings> mQualitySettings;
    // The "lights" filters.
    std::int32_t mActiveFilter;
    std::int32_t mEntityFilter;
    std::int32_t mShadowFilter;
    std::int32_t mVolumetricFilter;
    // Set while CullLights runs.
    bool mCulling;
    // Set by CullLights; every poll clears it.
    bool mLightsCulled;
    // The quality level the spot shadow depth array was built for, an index
    // into mQualitySettings.
    std::int32_t mSpotShadowConfigIndex;
    // Counts the lights and probes removed.
    std::uint64_t mLightRemovalCount;
    // Set when the default environment changed (SetDefaultEnvironId).
    bool mDefaultEnvironDirty;
    // "environments", "lights" and "probes": the registered objects, with
    // the components of the environments, lights and probes.
    PropArray<GameObjectId> mEnvironments;
    eastl::vector<RndLightEnvironCom*> mEnvironComs;
    eastl::vector<RndLightCom*> mLights;
    eastl::vector<RndLightProbeCom*> mProbes;
    PropArray<GameObjectId> mLightIds;
    // Constructed and destroyed only; no reader or writer was found.
    eastl::vector<void*> mUnusedList;
    PropArray<GameObjectId> mProbeIds;
    // The cookies' places per texture type (RndTextureBase slot 2) and
    // whether the texture's flags have bit 1 set.
    eastl::vector<CookieArrayElemInfo> mCookieArrayElems[8][2];
    // Set when a light's cookie texture changed; the update rebuilds the
    // cookie texture arrays.
    bool mCookieTexArraysDirty;
    // The cookie texture arrays, which the destructor releases. Their
    // order is not decoded.
    RndTextureBase* mCookieTexArrays[4];
    // Set when the probes or their states changed; the poll rebuilds the
    // probe texture arrays.
    bool mProbeTexArraysDirty;
    // The diffuse and specular probe texture arrays
    // (RndLightProbeCom::SyncTexarrays).
    RndTextureArrayCube* mProbeTexArrays[2];
    // Set when the quality level's spot shadow configuration changed.
    bool mSpotShadowDepthTexArrayDirty;
    RndTextureArray2D* mSpotShadowDepthTexArray;
    // The shadow contributions and spot shadow depth layers taken this
    // frame; CullLights resets them.
    std::uint64_t mNumShadowContributions;
    std::uint64_t mNumSpotShadowDepthLayers;
    // "isolated_light" and "isolated_probe": object ids, the null id when
    // none, each followed by its component.
    std::int32_t mIsolatedLight;
    RndLightCom* mIsolatedLightCom;
    std::int32_t mIsolatedProbe;
    RndLightProbeCom* mIsolatedProbeCom;
    // "cur_probe_state_a_index" and "cur_probe_state_b_index".
    std::uint64_t mCurProbeStateAIndex;
    std::uint64_t mCurProbeStateBIndex;
    eastl::vector<RenamedProbeState> mRenamedProbeStates;
    // Set for a scene saved before the probe states moved to the light
    // manager; _OnResourcesLoaded collects them from the probes.
    bool mUpgradeProbeStates;
    CullResults mCullResults;
    CullResults mSliceZeroCullResults;
    // Set when GenerateAmbientOcclusion ran the SSAO this frame; the
    // lighting passes then read the frame interval's AO texture.
    bool mAmbientOcclusionGenerated;
    // The tiled lights' buffers by RndTiledLightsBufferType, each with the
    // number of lights that add and subtract light.
    RndTiledLightsComputeBuffer mLightBuffers[kNumTiledLightsBufferTypes];
    RndComputeBuffer* mLightProbeBuffer;
    RndComputeBuffer* mSliceZeroLightIdBuffer;
    // Subscriptions whose handlers are "leaving_playmode" and
    // "leaving_record_mode"; the subscribing code is not identified.
    MsgSource::EventSinkElem mLeavingPlayModeSink;
    MsgSource::EventSinkElem mLeavingRecordModeSink;
};

static_assert(sizeof(RndLightMgrCom::QualitySettings) == 16);
static_assert(offsetof(RndLightMgrCom::QualitySettings, mTonemappingEnabled) == 12);
static_assert(sizeof(RndLightMgrCom::RenamedProbeState) == 16);
static_assert(sizeof(RndLightMgrCom::CullResults) == 256);
static_assert(offsetof(RndLightMgrCom, mDefaultEnvironment) == 24);
static_assert(offsetof(RndLightMgrCom, mCookieTextureSize) == 56);
static_assert(offsetof(RndLightMgrCom, mProbeStates) == 64);
static_assert(offsetof(RndLightMgrCom, mCurStateA) == 104);
static_assert(offsetof(RndLightMgrCom, mCurStateBlend) == 120);
static_assert(offsetof(RndLightMgrCom, mCaptureState) == 128);
static_assert(offsetof(RndLightMgrCom, mQualitySettings) == 144);
static_assert(offsetof(RndLightMgrCom, mActiveFilter) == 184);
static_assert(offsetof(RndLightMgrCom, mVolumetricFilter) == 196);
static_assert(offsetof(RndLightMgrCom, mCulling) == 200);
static_assert(offsetof(RndLightMgrCom, mSpotShadowConfigIndex) == 204);
static_assert(offsetof(RndLightMgrCom, mLightRemovalCount) == 208);
static_assert(offsetof(RndLightMgrCom, mDefaultEnvironDirty) == 216);
static_assert(offsetof(RndLightMgrCom, mEnvironments) == 224);
static_assert(offsetof(RndLightMgrCom, mEnvironComs) == 264);
static_assert(offsetof(RndLightMgrCom, mLights) == 296);
static_assert(offsetof(RndLightMgrCom, mProbes) == 328);
static_assert(offsetof(RndLightMgrCom, mLightIds) == 360);
static_assert(offsetof(RndLightMgrCom, mUnusedList) == 400);
static_assert(offsetof(RndLightMgrCom, mProbeIds) == 432);
static_assert(offsetof(RndLightMgrCom, mCookieArrayElems) == 472);
static_assert(offsetof(RndLightMgrCom, mCookieTexArraysDirty) == 984);
static_assert(offsetof(RndLightMgrCom, mCookieTexArrays) == 992);
static_assert(offsetof(RndLightMgrCom, mProbeTexArraysDirty) == 1024);
static_assert(offsetof(RndLightMgrCom, mProbeTexArrays) == 1032);
static_assert(offsetof(RndLightMgrCom, mSpotShadowDepthTexArrayDirty) == 1048);
static_assert(offsetof(RndLightMgrCom, mSpotShadowDepthTexArray) == 1056);
static_assert(offsetof(RndLightMgrCom, mNumShadowContributions) == 1064);
static_assert(offsetof(RndLightMgrCom, mIsolatedLight) == 1080);
static_assert(offsetof(RndLightMgrCom, mIsolatedLightCom) == 1088);
static_assert(offsetof(RndLightMgrCom, mIsolatedProbe) == 1096);
static_assert(offsetof(RndLightMgrCom, mCurProbeStateAIndex) == 1112);
static_assert(offsetof(RndLightMgrCom, mRenamedProbeStates) == 1128);
static_assert(offsetof(RndLightMgrCom, mUpgradeProbeStates) == 1160);
static_assert(offsetof(RndLightMgrCom, mCullResults) == 1168);
static_assert(offsetof(RndLightMgrCom, mSliceZeroCullResults) == 1424);
static_assert(offsetof(RndLightMgrCom, mAmbientOcclusionGenerated) == 1680);
static_assert(offsetof(RndLightMgrCom, mLightBuffers) == 1688);
static_assert(offsetof(RndLightMgrCom, mLightProbeBuffer) == 1760);
static_assert(offsetof(RndLightMgrCom, mSliceZeroLightIdBuffer) == 1768);
static_assert(offsetof(RndLightMgrCom, mLeavingPlayModeSink) == 1776);
static_assert(offsetof(RndLightMgrCom, mLeavingRecordModeSink) == 1816);
static_assert(sizeof(RndLightMgrCom) == 1856);
