#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/color/Color.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/textures/RndTextureCubeResource.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Entity;
class EntityResource;
class Frustum;
class RndCameraContext;
class RndComputeBuffer;
class RndContext;
class RndDrawNodeCom;
class RndPixelDataCube;
class RndShaderCBuffer;
class RndTextureArrayCube;
class RndTextureBase;
class RndTextureCube;
class TransCom;
class Vector2i;

namespace RndComputeBufferStructs {
struct RndCSLightProbe;
}

// The two cube textures a probe captures for each lighting state. The map
// names the type; the enumerator names are not in the reference map.
enum RndLightProbeTexture : int {
    kLightProbeDiffuse = 0,
    kLightProbeSpecular = 1,
};

// A light probe (render/RndLightProbeCom.o, 0x498080-0x49DB9F). For each of
// the light manager's probe states it holds an inlined diffuse and specular
// cube texture, captured from a camera entity at the probe's position, and
// the light manager blends the two current states' textures through texture
// arrays. The probe registers with the scene's light manager when its
// resources load and keeps its draw node's sphere at the falloff end. The
// vtable at 0x1907370 has 41 slots. The object is 184 bytes.
class RndLightProbeCom : public Component {
public:
    // The probe's per-object runtime state (constructor 0x4982B0). The map
    // names the struct; its field names are not in the reference map.
    struct RuntimeData {
        // One probe state's textures. The constructor (map: 0x20 bytes) is
        // inlined into StateInserted.
        struct StateRuntimeInfo {
            StateRuntimeInfo() : mCapturedPixels{nullptr, nullptr} {}

            // The inlined cube textures, indexed by RndLightProbeTexture.
            ResourcePtr<RndTextureCubeResource> mTextures[2];
            // The pixels Capture read back, until CommitCaptureResults
            // inlines them.
            RndPixelDataCube* mCapturedPixels[2];
        };

        RuntimeData();  // 0x4982B0

        // The object's transform and draw node, cached when the resources
        // load.
        TransCom* mTrans;
        RndDrawNodeCom* mDrawNode;
        // Set when the falloff changed; the poll moves the draw node's
        // sphere.
        bool mSphereDirty;
        // Set while the probe is registered with the scene's light manager.
        bool mInBookkeeping;
        // Whether the light manager draws the probe: the probe is enabled
        // and its draw node is not hidden (_EditPoll).
        bool mVisible;
        // The textures of each of the light manager's probe states.
        eastl::vector<StateRuntimeInfo> mStates;
        // The probe's deferred-lighting constants, created when the
        // resources load.
        RndShaderCBuffer* mCBuffer;
        // The probe's first layer in the light manager's texture arrays
        // (SyncTexarrays), or -1.
        long mTexArrayIndex;
        // The "use_raw_texture" and "isolate" properties, which the
        // registry binds to these runtime fields.
        bool mUseRawTexture;
        bool mIsolate;
        // mIsolate as of the last edit poll.
        bool mWasIsolated;
        // The probe's index in the light manager's probe compute buffer,
        // assigned each frame (0x4843A5); -1 after the poll.
        int mComputeBufferIndex;
    };

    RndLightProbeCom();            // 0x498170
    // The map's RndLightProbeCom(RndLightProbeCom const&): copies the
    // properties for an imprint, the state names through
    // PropArrayBase::_Copy; the run-time state starts afresh. Inlined into
    // _Imprint (0x49CE90).
    RndLightProbeCom(const RndLightProbeCom& other)
        : Component(other),
          mEnabled(other.mEnabled),
          mFalloffStart(other.mFalloffStart),
          mFalloffEnd(other.mFalloffEnd),
          mFalloffFunction(other.mFalloffFunction),
          mRange(other.mRange),
          mIncludeAtmosphere(other.mIncludeAtmosphere),
          mBackgroundColor(other.mBackgroundColor),
          mRuntime() {
        mStateNames._Copy(other.mStateNames);
    }
    ~RndLightProbeCom() override;  // slots 0-1: 0x498330, 0x498420

    Symbol GetId() const override;            // slot 4: 0x49CE10
    Symbol GetClassName() const override;     // slot 5: 0x49CE20
    int CurrentRev() const override;          // slot 7: 0x49CE30
    bool IsA(Symbol type) const override;     // slot 8: 0x49CE50
    Component* AsComponent() override;        // slot 9: 0x49CE80
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x49CE90
    // Slot 18: the probe follows its object's draw node.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x49C790
    // Slot 20: leaves the light manager for an instance, object or
    // component destroy, and collapses the draw node's sphere onto the
    // object when the component alone is destroyed.
    void _PreDestroy(DestroyType type) override;  // 0x49BE90
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x49D080
    ComMetaData& _GetMetaData() override;       // slot 23: 0x49D090
    // Slot 29: caches the transform and draw node (creating the draw node),
    // syncs the sphere, joins the light manager, finds or creates each
    // state's inlined textures and creates the constant buffer. The map
    // names it _LoadResources(ObjPtr const&). Not reconstructed: the
    // inlined-resource lookup (the map's EntityResource::TryGetInline
    // <RndTextureCubeResource>) is not modelled.
    bool _OnResourcesLoaded() override;  // 0x49C080
    // Slot 33: moves the draw node's sphere after a falloff change.
    void _Poll() override;  // 0x49C850
    // Slot 38: follows the isolation flag, reloads changed textures and
    // polls.
    void _EditPoll() override;  // 0x49C900

    // The state names the light manager's state list starts from, gives
    // new states and shows for none.
    static Symbol GetDefaultStateName();      // 0x498080
    static Symbol GetNewStateName();          // 0x4980D0
    static Symbol GetNoneStateDisplayName();  // 0x498120

    // Sets where the falloff starts, pushing the end out to it.
    void SetFalloffStart(float distance);  // 0x498440
    // Sets where the falloff ends, pulling the start in to it.
    void SetFalloffEnd(float distance);  // 0x498480
    // The registry's change handlers for the two falloff properties; the
    // std::function wrappers at 0x49D0A0 and 0x49D0C0 inline them.
    void _SyncFalloffStart();  // 0x498460
    void _SyncFalloffEnd();    // 0x4984A0

    // The light manager inserted, removed or renamed a probe state.
    void StateInserted(unsigned long index);  // 0x4984C0
    void StateRemoved(unsigned long index);   // 0x4985B0
    // Moves the state's inlined textures to the paths of the new name. The
    // map's StateRenamed(ObjPtr const&, unsigned long, Symbol, Symbol). Not
    // reconstructed: EntityResource's uninline helper (0x1011F0) is not
    // modelled.
    void StateRenamed(unsigned long index, Symbol oldName, Symbol newName);  // 0x498680
    // The inlined resource path of a state's texture,
    // "light_probe_cubetex_0_<object serial>_<state>_diff" or "_spec". The
    // map's signature starts with an ObjPtr const&.
    ResourcePath _MakeResourcePath(Symbol state, RndLightProbeTexture texture) const;  // 0x4988D0
    // The loaded cube texture of a state, or null. Name not in the
    // reference map.
    RndTextureCube* GetStateTexture(unsigned long index, RndLightProbeTexture texture) const;  // 0x498A40

    // Renders the six faces from a camera entity at the probe, filters them
    // into diffuse and specular cubes and reads both back into the state's
    // captured pixels. Not reconstructed: the offscreen frame and the
    // shaders it drives are not modelled.
    void Capture(Symbol state, unsigned long index);  // 0x498A90
    // Creates and enters a RndSceneResource with a camera at the probe's
    // transform, its far plane at the capture range. Not reconstructed.
    EntityResource* _CreateCameraEntity() const;  // 0x499210
    // Filter passes from the captured cube into the state's cubes. Not
    // reconstructed.
    void _ProcessDiffuse(RndContext& context, RndTextureCube& source, RndTextureCube& target);   // 0x4993E0
    void _ProcessSpecular(RndContext& context, RndTextureCube& source, RndTextureCube& target);  // 0x4994A0
    // Replaces both of the state's textures with empty ones.
    void ZeroCaptureResults(Symbol state, unsigned long index);  // 0x4995E0
    // Inlines a new texture resource for the state at its path, dropping the
    // one inlined there before, and gives it the pixels when there are any.
    // Not reconstructed.
    void _ReplaceInlineCubetexResource(
        Symbol state,
        unsigned long index,
        RndLightProbeTexture texture,
        const RndPixelDataCube* pixels);  // 0x499620
    // Inlines the pixels Capture read back and frees them.
    void CommitCaptureResults(Symbol state, unsigned long index);  // 0x499850
    // Whether the frustum, or one of the extra planes, excludes the probe's
    // sphere; RndLightMgrCom culls probes with it as it culls lights with
    // RndLightCom's slot 43. Name not in the reference map. Not
    // reconstructed.
    bool _FrustumExcludes(const Frustum& frustum, const RndLightCullPlanes& planes) const;  // 0x499980
    // Applies the probe for the two blended states without the compute
    // path, with the ambient occlusion texture or null
    // (RndLightMgrCom::_AccumUntiledDeferredProbes, 0x48B300). Not
    // reconstructed.
    void DrawDeferredLightNoCompute(
        RndContext& context,
        unsigned long stateA,
        unsigned long stateB,
        float blend,
        RndTextureBase* ambientOcclusion);  // 0x499A60
    // The diffuse and specular textures of the two blended states, falling
    // back to the device's default cube; false when neither state is
    // given. Name not in the reference map. Not reconstructed.
    bool _GetBlendTextures(
        unsigned long stateA,
        unsigned long stateB,
        RndTextureCube* texturesA[2],
        RndTextureCube* texturesB[2],
        float* blend,
        float stateBlend) const;  // 0x499E40
    // Fills the probe's compute-buffer entry: the texture array layers, the
    // falloff and the world-to-probe transform. Not reconstructed.
    void _GetComputeShaderData(
        const RndCameraContext& camera,
        unsigned long stateA,
        unsigned long stateB,
        float blend,
        RndComputeBufferStructs::RndCSLightProbe& data) const;  // 0x49A000
    // Appends the probe's 96-byte entry to the buffer. Not reconstructed.
    void AddToComputeBuffer(
        const RndCameraContext& camera,
        unsigned long stateA,
        unsigned long stateB,
        float blend,
        RndComputeBuffer& buffer);  // 0x49A270
    // Builds the diffuse and specular cube-texture arrays from every
    // probe's state textures and assigns each probe its first layer. The
    // map's signature starts with an EntityPtr const& and takes the probe
    // ids before the states. Not reconstructed.
    static void SyncTexarrays(
        Entity* entity,
        eastl::vector<RndLightProbeCom*>& probes,
        const PropArray<GameObjectId>& probeIds,
        const PropArray<Symbol>& states,
        RndTextureArrayCube** textureArrays);  // 0x49A300

    // The capture's render-target format, RGBA 16-bit float.
    static int GetCaptureFormat();  // 0x49A9D0
    // A state texture's face size: 16 for diffuse, 128 for specular.
    static int GetTextureSize(RndLightProbeTexture texture);  // 0x49AAE0
    // Only the specular texture has mips.
    static bool TextureHasMips(RndLightProbeTexture texture);  // 0x49AAF0
    static int GetMaxCaptureBounces();                      // 0x49AB30
    static int GetCaptureSize();                            // 0x49AB40
    static int GetHighestCaptureDownsampleTextureSize();    // 0x49AB50
    static int GetHighestCaptureHelperTextureSize();        // 0x49AB60
    // A cube of each texel's direction and solid angle, which the filters
    // read. Not reconstructed.
    static RndTextureCube* CreateCaptureHelperTexture(int size);  // 0x49AB70
    // The solid angle of a cube texel. Not reconstructed.
    static float _PixelToSolidAngle(int size, const Vector2i& pixel);  // 0x49AE30
    static int GetNumSpecularMips();  // 0x49B280
    // Halves the cube through the light globals' downsample textures until
    // it is no larger than the size, and returns the last one. Not
    // reconstructed.
    RndTextureCube* _DownsampleTo(RndContext& context, RndTextureCube& source, int size) const;  // 0x49CB80

    // Joins and leaves the scene light manager's probe list. The map's
    // signatures take an ObjPtr const&.
    void _AddToBookkeeping();       // 0x49C700
    void _RemoveFromBookkeeping();  // 0x49BFF0

    // Registers the class. Not reconstructed: the registration helpers it
    // inlines are not modelled.
    static void Init();
    // The class factory.
    static Component* _Create();
    // Registers the class description and the properties. Not
    // reconstructed: the property metadata's attributes and the
    // std::function change handlers are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x49B290

    // The object's statics, constructed by the static initializer at
    // 0x49DAD0.
    static Symbol sId;  // 0x1A88DE8, "LightProbe"
    // Also "LightProbe". Name not in the reference map.
    static Symbol sClassName;           // 0x1A88DF0
    static PropRegistry sPropRegistry;  // 0x1A88E00
    static ComMetaData sMetaData;       // 0x1A88EA0

    // Field names are not in the reference map; the property members take
    // the names of the properties the registry binds to their offsets.
    // "enabled", in the Component base's tail padding. Cleared to switch
    // the probe off, as RndDefaults does with lights.
    bool mEnabled;
    float mFalloffStart;
    float mFalloffEnd;
    std::int32_t mFalloffFunction;
    // An array of symbols (vtable 0x1901938) the registry does not name;
    // the imprint copies it. The name is uncertain.
    PropArray<Symbol> mStateNames;
    // The "capture" group.
    float mRange;
    bool mIncludeAtmosphere;
    Hmx::Color mBackgroundColor;
    RuntimeData mRuntime;
};

static_assert(sizeof(RndLightProbeCom::RuntimeData::StateRuntimeInfo) == 32);
static_assert(offsetof(RndLightProbeCom::RuntimeData, mSphereDirty) == 16);
static_assert(offsetof(RndLightProbeCom::RuntimeData, mStates) == 24);
static_assert(offsetof(RndLightProbeCom::RuntimeData, mCBuffer) == 56);
static_assert(offsetof(RndLightProbeCom::RuntimeData, mTexArrayIndex) == 64);
static_assert(offsetof(RndLightProbeCom::RuntimeData, mUseRawTexture) == 72);
static_assert(offsetof(RndLightProbeCom::RuntimeData, mComputeBufferIndex) == 76);
static_assert(sizeof(RndLightProbeCom::RuntimeData) == 80);
static_assert(offsetof(RndLightProbeCom, mEnabled) == 22);
static_assert(offsetof(RndLightProbeCom, mFalloffStart) == 24);
static_assert(offsetof(RndLightProbeCom, mFalloffEnd) == 28);
static_assert(offsetof(RndLightProbeCom, mFalloffFunction) == 32);
static_assert(offsetof(RndLightProbeCom, mStateNames) == 40);
static_assert(offsetof(RndLightProbeCom, mRange) == 80);
static_assert(offsetof(RndLightProbeCom, mIncludeAtmosphere) == 84);
static_assert(offsetof(RndLightProbeCom, mBackgroundColor) == 88);
static_assert(offsetof(RndLightProbeCom, mRuntime) == 104);
static_assert(sizeof(RndLightProbeCom) == 184);
