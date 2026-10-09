#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/core/DynamicCom.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/color/Color.h"
#include "render/meshes/RndDynamicGpuData.h"
#include "render/shadergraph/RndShaderGraphResource.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class GameObject;
class RndMaterialRuntimeData;
class RndMaterialRuntimeDataResource;

// How a material blends into the target. Values confirmed from the material
// blend-mode metadata table. The map names the type
// (RndMaterialCom::SetBlendMode, PS4Context::_SetBlendModeImpl); the
// enumerator names are not in the reference map.
enum class RndBlendMode : std::int32_t {
    kSourceAlpha = 0,
    kSourceAlphaAdd = 1,
    kPremultipliedAlpha = 2,
    kScreen = 3,
    kDestination = 4,
    kSource = 5,
    kAdd = 6,
    kSubtract = 7,
    kMultiply = 8,
    kLighten = 9,
    kDarken = 10,
    kDecalLitSourceAlpha = 11,
};

// Whether a material is shared between the objects that use it. The map
// names the type (RndMaterialCom::SetSharingType); the enumerator names are
// not in the reference map.
enum class RndMaterialSharing : std::int32_t {
    kAutomatic = 0,
    kShared = 1,
    kUnique = 2,
};

// Whether the material on the object must be unique under automatic
// sharing: false when the entity holds one material object shared by every
// user. Emitted near the shader-graph categories (0x50F2D0); not
// reconstructed. Name not in the reference map; the evidence is weak.
bool NeedsUniqueMaterial(const GameObject& owner);  // 0x50F2D0

// The material component: a shader graph, the graph's exposed properties as
// dynamic properties, and the render state the scene drawer reads
// (render/RndMaterialCom.o, 0x4F1BD0 to 0x4F662F). Its class id is
// "Material". The material builds an RndMaterialRuntimeData for its graph,
// shared through a "mat_rt:" resource in the entity unless the material is
// unique, and refreshes its constant buffer through the scene drawer's GPU
// data queue (RndDynamicGpuData, the secondary base at +200). The vtable at
// 0x1910AF8 has 48 slots; the DynamicComResource and RndDynamicGpuData
// vtables are at 0x1910C88 and 0x1910CA0. The object is 368 bytes. Field
// names are not in the reference map; the properties are named after the
// registry (0x4F1C10).
class RndMaterialCom : public DynamicCom<RndShaderGraphResource>, public RndDynamicGpuData {
public:
    RndMaterialCom();  // 0x4F3290
    // Slots 0-1 at 0x4F34F0 and 0x4F3740; the RndDynamicGpuData thunks are
    // at 0x4F3730 and 0x4F3760.
    ~RndMaterialCom() override;

    // Slot 2: "leaving_playmode" and "leaving_record_mode" mark a shared
    // material's runtime data for a refresh; any other message is
    // unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x4F48A0
    Symbol GetId() const override;             // slot 4: 0x4F4A00
    Symbol GetClassName() const override;      // slot 5: 0x4F4A10
    int CurrentRev() const override;           // slot 7: 0x4F4A20
    bool IsA(Symbol type) const override;      // slot 8: 0x4F4A40
    Component* AsComponent() override;         // slot 9: 0x4F4A70
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Not
    // reconstructed: the copy inlines DynamicCom<RndShaderGraphResource>'s
    // copy constructor (the map's DynamicCom(DynamicCom const&), which
    // copies "prop_storage" through PropArrayBase::_Copy, "prop_crc" and the
    // file, and starts the saved properties and the resources afresh),
    // which src/entity does not model; the copy's properties go through
    // DynamicComBase::_ImprintProps (0x1D1540).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x4F4A80
    // Slot 19 at 0x4F3A20: takes the shader graph and blend mode of the
    // object's draw instance's default material.
    void _PostCreate() override;
    ComMetaData& _GetMetaData() override;  // slot 23: 0x4F4D40
    // Slot 29 at 0x4F3AB0: loads the graph, checks the render state against
    // it and rebuilds the runtime data. The map's _LoadResources(ObjPtr
    // const&) does this work.
    bool _OnResourcesLoaded() override;
    // Slot 33 at 0x4F4380.
    void _Poll() override;
    // Slot 38 at 0x4F45C0: DynamicCom's edit poll, then the poll's work.
    void _EditPoll() override;
    // Slot 44 at 0x4F4E10.
    PropRegistry& _GetBasePropRegistry() override;
    // Slot 47 at 0x4F4670 (thunk 0x4F46D0): refreshes the constant buffer
    // through the context and clears the sync flags.
    void _SyncDynamicGpuDataImpl(RndContext& context) override;

    // The interface symbol of the classes that provide a material,
    // "MaterialProvider".
    static Symbol GetMaterialProviderId();  // 0x4F3240
    // The map's signature is SetSharingType(ObjPtr const&,
    // RndMaterialSharing); this build passes the owning object.
    void SetSharingType(const GameObject& owner, RndMaterialSharing sharing);  // 0x4F3870
    void SetShaderGraphFile(const char* file);  // 0x4F3790
    void SetBlendMode(RndBlendMode mode);       // 0x4F3A00

    // Applies the sharing type: unique for kUnique, or for kAutomatic when
    // the object needs it. The map's signature is _SyncSharing(ObjPtr
    // const&, bool).
    void _SyncSharing(const GameObject& owner);  // 0x4F38C0
    // Switches between a unique and a shared runtime data, and recomputes
    // "is_puppet". Name not in the reference map.
    void _SetUnique(bool unique);  // 0x4F3900
    // Finds or builds the runtime data for the graph: a shared material
    // looks up its "mat_rt:" resource in the entity and creates it when it
    // is missing. The map's signature starts with an ObjPtr const&.
    void _InitRuntimeData();  // 0x4F3E50
    // Copies the render state into the runtime data and rebuilds its usage
    // hints when it changed.
    void _SyncRenderState();  // 0x4F4730
    // Releases the runtime data: a unique material deletes it, a shared one
    // releases its resource. The map's signature starts with an ObjPtr
    // const&.
    void _ReleaseRuntimeData();  // 0x4F47D0
    // The poll's work: syncs a dirty render state, and queues the runtime
    // data with the scene drawer once it needs a GPU refresh. Name not in
    // the reference map.
    void _PollRuntimeData();  // 0x4F43A0
    // The "leaving_playmode" and "leaving_record_mode" handlers, inlined
    // into Handle. Names not in the reference map.
    DataNode _OnLeavingPlaymode();    // 0x4F4840
    DataNode _OnLeavingRecordMode();  // 0x4F4870

    // Registers the class: the properties, the description and the base
    // registry's builder.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x4F1BD0
    // Registers the properties. Not reconstructed: the property metadata's
    // attributes are written through helpers that are not modelled.
    static void InitPropRegistry(PropRegistry& registry);  // 0x4F1C10
    // Writes the class description ("Material used for rendering"), the
    // author, the resource it may be created in, the editor name and the
    // "MaterialProvider" interface. Not reconstructed: ComMetaData's string
    // and list members are written inline. Name not in the reference map.
    static void _InitMetaData(ComMetaData& metadata);  // 0x4F3010

    static Symbol sId;  // 0x1A8B920, "Material"
    // The class's second symbol, also "Material", which the component
    // factory and the component-order lists take. Name not in the reference
    // map.
    static Symbol sClassName;  // 0x1A8B928
    static PropRegistry sPropRegistry;  // 0x1A8B930
    static ComMetaData sMetaData;       // 0x1A8B9D0

    // "sharing_type".
    RndMaterialSharing mSharing;
    // "bucket".
    std::int32_t mBucket;
    // "blend_mode".
    RndBlendMode mBlendMode;
    // "blend_factor"; starts white.
    Hmx::Color mBlendFactor;
    // "cull_mode".
    std::int32_t mCullMode;
    // "receive_atmosphere", "receive_decals", "depth_prepass",
    // "force_opaque", "scene_mask" and "unique".
    bool mReceiveAtmosphere;
    bool mReceiveDecals;
    bool mDepthPrepass;
    bool mForceOpaque;
    bool mSceneMask;
    bool mUnique;
    // Alignment padding; the constructor does not write it.
    unsigned char mPad[2];
    // Set by the "sharing_type" property's change handler (0x4F4EE0) and
    // cleared once the sharing is applied.
    bool mSharingDirty;
    // Set by the render-state properties' change handlers (0x4F4EF0 to
    // 0x4F4F30) when the render state must be copied to the runtime data.
    bool mRenderStateDirty;
    // Set until the runtime data was first built; while it is set, a reused
    // runtime data still loads the exposed textures.
    bool mFirstRuntimeData;
    // "is_puppet": a shared material on an object whose instancing object
    // holds a MaterialPuppet component; the poll leaves it alone.
    bool mIsPuppet;
    // The "mat_rt:" resource of a shared material's runtime data.
    ResourcePtr<RndMaterialRuntimeDataResource> mRuntimeDataResource;
    RndMaterialRuntimeData* mRuntimeData;
    // Set when the runtime data must be refreshed on the GPU.
    bool mRuntimeDataDirty;
    // Keeps _OnResourcesLoaded from replacing a blend mode the graph does
    // not allow. Its writer is not identified, so the name is weak.
    bool mKeepBlendMode;
    // Two subscriptions for the editor's mode changes that Handle answers.
    // Their writers are not identified, so the names are weak.
    MsgSource::EventSinkElem mPlaymodeSink;
    MsgSource::EventSinkElem mRecordModeSink;
};

static_assert(offsetof(RndMaterialCom, mFile) == 184);
static_assert(offsetof(RndMaterialCom, mSharing) == 216);
static_assert(offsetof(RndMaterialCom, mBucket) == 220);
static_assert(offsetof(RndMaterialCom, mBlendMode) == 224);
static_assert(offsetof(RndMaterialCom, mBlendFactor) == 228);
static_assert(offsetof(RndMaterialCom, mCullMode) == 244);
static_assert(offsetof(RndMaterialCom, mReceiveAtmosphere) == 248);
static_assert(offsetof(RndMaterialCom, mUnique) == 253);
static_assert(offsetof(RndMaterialCom, mSharingDirty) == 256);
static_assert(offsetof(RndMaterialCom, mRenderStateDirty) == 257);
static_assert(offsetof(RndMaterialCom, mIsPuppet) == 259);
static_assert(offsetof(RndMaterialCom, mRuntimeDataResource) == 264);
static_assert(offsetof(RndMaterialCom, mRuntimeData) == 272);
static_assert(offsetof(RndMaterialCom, mRuntimeDataDirty) == 280);
static_assert(offsetof(RndMaterialCom, mPlaymodeSink) == 288);
static_assert(offsetof(RndMaterialCom, mRecordModeSink) == 328);
static_assert(sizeof(RndMaterialCom) == 368);
