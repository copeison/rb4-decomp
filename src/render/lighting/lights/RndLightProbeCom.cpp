// render/RndLightProbeCom.o (0x498080-0x49DB9F). The registry's revision
// handler (0x49D400 to 0x49D910) is the lambda in _Init. The vector growth
// helpers (0x49AA20, 0x49D0E0, 0x49D920) and the unreferenced description
// setup at 0x49AB00 are not reconstructed.
#include "render/lighting/lights/RndLightProbeCom.h"

#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "math/geometry/Sphere.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "os/platform/PlatformMgr.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/textures/RndPixelDataCube.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTextureCubeResource.h"
#include "utl/text/MakeString.h"

// The object's statics, in the order of its static initializer (0x49DAD0).
// The three ints it first sets (0x1A88DD8-0x1A88DE0, to -1, 8 and 4) come
// from a shared header and are not modelled.
Symbol RndLightProbeCom::sId("LightProbe");
Symbol RndLightProbeCom::sClassName("LightProbe");
PropRegistry RndLightProbeCom::sPropRegistry;
ComMetaData RndLightProbeCom::sMetaData;

namespace {

// The scene light manager of the probe's entity, or null. Inlined into
// every user. Name not in the reference map.
RndLightMgrCom* FindLightMgr(const Component& component) {
    RndSceneCom* scene = component.mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    if (scene == nullptr) {
        return nullptr;
    }
    return scene->GetLightMgr();
}

// Reads a property of the component that holds the properties as they were
// saved into `value`; a missing or unconvertible property keeps the value.
// The release build keeps only the arguments of the error it reports.
// Inlined into the revision handler. Name not in the reference map.
template <typename T>
void ReadSavedProp(const Component& saved, const PropPath& path, T& value, PropertyType type) {
    void* storage = const_cast<Component*>(&saved);
    const PropInfo* const info = const_cast<Component&>(saved)._GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _GetProp(saved, path, *info, storage, &value, type)) {
        return;
    }
    static_cast<void>(saved.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

// The falloff callbacks _Init installs. Names not in the reference map.
void OnFalloffStartChanged(const PropAccessorArgs& args);  // 0x49D0A0
void OnFalloffEndChanged(const PropAccessorArgs& args);    // 0x49D0C0

}  // namespace

// Reconstructed from eboot.elf at 0x498080.
Symbol RndLightProbeCom::GetDefaultStateName() {
    return Symbol("default");
}

// Reconstructed from eboot.elf at 0x4980D0.
Symbol RndLightProbeCom::GetNewStateName() {
    return Symbol("new_state");
}

// Reconstructed from eboot.elf at 0x498120.
Symbol RndLightProbeCom::GetNoneStateDisplayName() {
    return Symbol("<None>");
}

// Reconstructed from eboot.elf at 0x498170. The RuntimeData constructor is
// inlined here.
RndLightProbeCom::RndLightProbeCom()
    : mEnabled(true),
      mFalloffStart(0.0F),
      mFalloffEnd(100.0F),
      mFalloffFunction(3),
      mRange(1000.0F),
      mIncludeAtmosphere(true),
      mBackgroundColor(Hmx::Color::GetBlack()) {}

// Reconstructed from eboot.elf at 0x4982B0.
RndLightProbeCom::RuntimeData::RuntimeData()
    : mTrans(nullptr),
      mDrawNode(nullptr),
      mSphereDirty(false),
      mInBookkeeping(false),
      mVisible(false),
      mCBuffer(nullptr),
      mTexArrayIndex(-1),
      mUseRawTexture(false),
      mIsolate(false),
      mWasIsolated(false),
      mComputeBufferIndex(-1) {}

// Reconstructed from eboot.elf at 0x498330.
RndLightProbeCom::~RndLightProbeCom() {
    RndShaderCBuffer::SafeDelete(mRuntime.mCBuffer);
}

// Reconstructed from eboot.elf at 0x498440.
void RndLightProbeCom::SetFalloffStart(float distance) {
    mFalloffStart = distance;
    _SyncFalloffStart();
}

// Reconstructed from eboot.elf at 0x498460.
void RndLightProbeCom::_SyncFalloffStart() {
    const float end = mFalloffStart > mFalloffEnd ? mFalloffStart : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        mRuntime.mSphereDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x498480.
void RndLightProbeCom::SetFalloffEnd(float distance) {
    mFalloffEnd = distance;
    _SyncFalloffEnd();
}

// Reconstructed from eboot.elf at 0x4984A0.
void RndLightProbeCom::_SyncFalloffEnd() {
    mFalloffStart = mFalloffEnd < mFalloffStart ? mFalloffEnd : mFalloffStart;
    mRuntime.mSphereDirty = true;
}

// Reconstructed from eboot.elf at 0x4984C0.
void RndLightProbeCom::StateInserted(unsigned long index) {
    mRuntime.mStates.insert(mRuntime.mStates.begin() + index, RuntimeData::StateRuntimeInfo());
}

// Reconstructed from eboot.elf at 0x4985B0.
void RndLightProbeCom::StateRemoved(unsigned long index) {
    mRuntime.mStates.erase(mRuntime.mStates.begin() + index);
}

// Reconstructed from eboot.elf at 0x4988D0. The upper half of the object id
// is its serial.
ResourcePath RndLightProbeCom::_MakeResourcePath(Symbol state, RndLightProbeTexture texture) const {
    const unsigned int serial = mObject->mId.mId >> 16;
    const char* suffix = texture == kLightProbeDiffuse ? "diff" : "spec";
    FormatString format("light_probe_cubetex_%d_%d_%s_%s");
    format << 0 << serial << state << suffix;
    const char* path = format.Str();
    ResourcePath resourcePath;
    resourcePath = path;
    return resourcePath;
}

// Reconstructed from eboot.elf at 0x498A40.
RndTextureCube* RndLightProbeCom::GetStateTexture(unsigned long index, RndLightProbeTexture texture) const {
    RndTextureCubeResource* resource = mRuntime.mStates[index].mTextures[texture].Get();
    if (resource == nullptr || resource->Fail()) {
        return nullptr;
    }
    return resource->mTexture;
}

// Reconstructed from eboot.elf at 0x4995E0.
void RndLightProbeCom::ZeroCaptureResults(Symbol state, unsigned long index) {
    _ReplaceInlineCubetexResource(state, index, kLightProbeDiffuse, nullptr);
    _ReplaceInlineCubetexResource(state, index, kLightProbeSpecular, nullptr);
}

// Reconstructed from eboot.elf at 0x499850.
void RndLightProbeCom::CommitCaptureResults(Symbol state, unsigned long index) {
    RuntimeData::StateRuntimeInfo& info = mRuntime.mStates[index];
    _ReplaceInlineCubetexResource(
        state, index, kLightProbeDiffuse, info.mCapturedPixels[kLightProbeDiffuse]);
    delete info.mCapturedPixels[kLightProbeDiffuse];
    info.mCapturedPixels[kLightProbeDiffuse] = nullptr;
    _ReplaceInlineCubetexResource(
        state, index, kLightProbeSpecular, info.mCapturedPixels[kLightProbeSpecular]);
    delete info.mCapturedPixels[kLightProbeSpecular];
    info.mCapturedPixels[kLightProbeSpecular] = nullptr;
}

// Reconstructed from eboot.elf at 0x49A9D0.
int RndLightProbeCom::GetCaptureFormat() {
    const RndDataFormatInfo info = {64, 4, 2, 1, -1};
    return RndFindSupportedDataFormat(info, kPlatformPS4);
}

// Reconstructed from eboot.elf at 0x49AAE0.
int RndLightProbeCom::GetTextureSize(RndLightProbeTexture texture) {
    return texture == kLightProbeDiffuse ? 16 : 128;
}

// Reconstructed from eboot.elf at 0x49AAF0.
bool RndLightProbeCom::TextureHasMips(RndLightProbeTexture texture) {
    return texture != kLightProbeDiffuse;
}

// Reconstructed from eboot.elf at 0x49AB30.
int RndLightProbeCom::GetMaxCaptureBounces() {
    return 4;
}

// Reconstructed from eboot.elf at 0x49AB40.
int RndLightProbeCom::GetCaptureSize() {
    return 512;
}

// Reconstructed from eboot.elf at 0x49AB50.
int RndLightProbeCom::GetHighestCaptureDownsampleTextureSize() {
    return 256;
}

// Reconstructed from eboot.elf at 0x49AB60.
int RndLightProbeCom::GetHighestCaptureHelperTextureSize() {
    return 128;
}

// Reconstructed from eboot.elf at 0x49B280.
int RndLightProbeCom::GetNumSpecularMips() {
    return 6;
}

// Reconstructed from eboot.elf at 0x49B290. "isolate" and
// "use_raw_texture" are bound to run-time fields and not saved. Revision 2
// takes the probe-state names from the "states" array of probes saved
// before it.
void RndLightProbeCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Light probe, used to capture and apply global illumination at a given location";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryLight;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    metadata.mRequiredComponents.push_back(RndDrawNodeCom::sClassName);
    RndLightProbeCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 1) {
                return;
            }
            RndLightProbeCom* const probe = static_cast<RndLightProbeCom*>(args.mComponent);
            const Component& saved = *args.mSavedProps;
            const unsigned long count = static_cast<unsigned long>(saved.Size(PropPath(Symbol("states"))));
            probe->mStateNames.Resize(count);
            for (unsigned long i = 0; i < count; ++i) {
                PropPath::Node states;
                PropPath::Node index;
                PropPath::Node name;
                const PropPath path(states = Symbol("states"), index = i, name = Symbol("name"));
                Symbol stateName;
                ReadSavedProp(saved, path, stateName, kPropertySymbol);
                probe->mStateNames[i] = stateName;
            }
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(2);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    BoolMetadata& enabled = TypeSpecificMetadata(
        registry.RegisterProp("enabled", offsetof(RndLightProbeCom, mEnabled), kPropertyBool, 1),
        prototype.mEnabled);
    enabled.mDefault = prototype.mEnabled;

    BoolMetadata& isolate = TypeSpecificMetadata(
        registry.RegisterProp(
            "isolate",
            offsetof(RndLightProbeCom, mRuntime) + offsetof(RuntimeData, mIsolate),
            kPropertyBool,
            1),
        prototype.mRuntime.mIsolate);
    isolate.mDefault = prototype.mRuntime.mIsolate;
    static_cast<void>(Symbol("prop"));
    isolate.mSaves = false;

    FloatMetadata& falloffStart = TypeSpecificMetadata(
        registry.RegisterProp("falloff_start", offsetof(RndLightProbeCom, mFalloffStart), kPropertyFloat, 4),
        prototype.mFalloffStart);
    falloffStart.mDefault = prototype.mFalloffStart;
    static_cast<void>(Symbol("prop"));
    falloffStart.mMin = 0.0F;
    falloffStart.mMax = 2000.0F;
    falloffStart.mSlider = true;
    falloffStart.mSliderLogResolution = 2.0F;
    falloffStart.mOnChanged = OnFalloffStartChanged;

    FloatMetadata& falloffEnd = TypeSpecificMetadata(
        registry.RegisterProp("falloff_end", offsetof(RndLightProbeCom, mFalloffEnd), kPropertyFloat, 4),
        prototype.mFalloffEnd);
    falloffEnd.mDefault = prototype.mFalloffEnd;
    static_cast<void>(Symbol("prop"));
    falloffEnd.mMin = 0.1F;
    falloffEnd.mMax = 2000.0F;
    falloffEnd.mSlider = true;
    falloffEnd.mSliderLogResolution = 2.0F;
    falloffEnd.mOnChanged = OnFalloffEndChanged;

    IntMetadata<int>& falloffFunction = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "falloff_function",
        offsetof(RndLightProbeCom, mFalloffFunction), kPropertyInt, 4).mMetadata.Get());
    falloffFunction.mDefault = prototype.mFalloffFunction;
    static_cast<void>(Symbol("prop"));
    falloffFunction.mHelp =
        String("Controls how light intensity falls off as you move away from the probe center.");
    {
        eastl::vector<AllowedValue<int>> functions;
        unsigned int savedTemp;
        MemPushTemp(savedTemp, true, true);
        for (int index = 0; index < kNumFalloffFunctions; ++index) {
            const RndFalloffFunction function = static_cast<RndFalloffFunction>(index);
            functions.push_back(
                AllowedValue<int>{index, String(ToString(function)), String(ToDescription(function))});
        }
        MemPopTemp(savedTemp);
        falloffFunction.mAllowedValues = functions;
    }

    PropRegistry& capture =
        static_cast<GroupMetadata&>(*registry.RegisterProp("capture", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));

    FloatMetadata& range = TypeSpecificMetadata(
        capture.RegisterProp("range", offsetof(RndLightProbeCom, mRange), kPropertyFloat, 4),
        prototype.mRange);
    range.mDefault = prototype.mRange;
    static_cast<void>(Symbol("prop"));
    range.mHelp = String("The range (ie. camera far distance) to use when running the probe capture");
    range.mMin = 1.0F;
    range.mMax = 10000.0F;
    range.mSlider = true;
    range.mSliderLogResolution = 1.0F;

    BoolMetadata& atmosphere = TypeSpecificMetadata(
        capture.RegisterProp(
            "include_atmosphere", offsetof(RndLightProbeCom, mIncludeAtmosphere), kPropertyBool, 1),
        prototype.mIncludeAtmosphere);
    atmosphere.mDefault = prototype.mIncludeAtmosphere;
    static_cast<void>(Symbol("prop"));
    atmosphere.mHelp = String("Should we draw the atmosphere during probe capture?");

    ColorMetadata& background = TypeSpecificMetadata(
        capture.RegisterProp(
            "background_color", offsetof(RndLightProbeCom, mBackgroundColor), kPropertyColor, 16),
        prototype.mBackgroundColor);
    background.mDefault = prototype.mBackgroundColor;
    static_cast<void>(Symbol("prop"));
    background.mHelp = String(
        "Background color to use during probe capture (ignored if the scene has its own background color)");

    BoolMetadata& rawTexture = TypeSpecificMetadata(
        capture.RegisterProp(
            "use_raw_texture",
            offsetof(RndLightProbeCom, mRuntime) + offsetof(RuntimeData, mUseRawTexture),
            kPropertyBool,
            1),
        prototype.mRuntime.mUseRawTexture);
    rawTexture.mDefault = prototype.mRuntime.mUseRawTexture;
    static_cast<void>(Symbol("prop"));
    rawTexture.mDebug = true;
    rawTexture.mSaves = false;
}

// Reconstructed from eboot.elf at 0x49BE90.
void RndLightProbeCom::_PreDestroy(DestroyType type) {
    if (type < kDestroyInstance) {
        return;
    }
    _RemoveFromBookkeeping();
    if (type != kDestroyComponent) {
        return;
    }
    RndDrawNodeCom* node = mObject->GetCom<RndDrawNodeCom>();
    if (node == nullptr) {
        return;
    }
    const Transform& xfm = mObject->GetExistingCom<TransCom>()->mWorldXfm;
    node->SetLocalSphere(Sphere{xfm.v, 0.0F});
}

// Reconstructed from eboot.elf at 0x49BFF0.
void RndLightProbeCom::_RemoveFromBookkeeping() {
    if (!mRuntime.mInBookkeeping) {
        return;
    }
    mRuntime.mInBookkeeping = false;
    if (RndLightMgrCom* mgr = FindLightMgr(*this)) {
        mgr->RemoveProbe(this);
    }
}

// Reconstructed from eboot.elf at 0x49C700.
void RndLightProbeCom::_AddToBookkeeping() {
    mRuntime.mInBookkeeping = true;
    if (RndLightMgrCom* mgr = FindLightMgr(*this)) {
        mgr->AddProbe(this);
    }
}

// Reconstructed from eboot.elf at 0x49C790.
void RndLightProbeCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(RndDrawNodeCom::sClassName);
}

// Reconstructed from eboot.elf at 0x49C850. The map's
// _SyncLocalSphere(ObjPtr const&) is inlined here.
void RndLightProbeCom::_Poll() {
    if (mRuntime.mSphereDirty) {
        mRuntime.mSphereDirty = false;
        mRuntime.mDrawNode->SetLocalSphere(Sphere{Vector3::sZero, mFalloffEnd});
    }
    mRuntime.mComputeBufferIndex = -1;
    mRuntime.mVisible = false;
    if (mEnabled && (mObject->GetExistingCom<RndDrawNodeCom>()->mRuntime.mWorldShowHideFlags & 1) == 0) {
        mRuntime.mVisible = true;
    }
}

// Reconstructed from eboot.elf at 0x49C900. A changed texture file reloads
// the resources; the rest is the poll, inlined.
void RndLightProbeCom::_EditPoll() {
    RndLightMgrCom* mgr = FindLightMgr(*this);
    if (mgr == nullptr) {
        mRuntime.mIsolate = false;
    } else if (mRuntime.mIsolate == mRuntime.mWasIsolated) {
        mRuntime.mIsolate = mgr->mIsolatedProbe == static_cast<std::int32_t>(mObject->mId.mId);
    } else {
        mgr->SetIsolatedProbe(mRuntime.mIsolate ? this : nullptr);
    }
    mRuntime.mWasIsolated = mRuntime.mIsolate;
    for (RuntimeData::StateRuntimeInfo& info : mRuntime.mStates) {
        bool changed = false;
        for (ResourcePtr<RndTextureCubeResource>& texture : info.mTextures) {
            if (texture != nullptr && (texture->mFileChangedOnDisk || texture->NeedsReload())) {
                changed = true;
                break;
            }
        }
        if (changed) {
            _OnResourcesLoaded();
            break;
        }
    }
    RndLightProbeCom::_Poll();
}

// Reconstructed from eboot.elf at 0x49CE10.
Symbol RndLightProbeCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x49CE20.
Symbol RndLightProbeCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x49CE30.
int RndLightProbeCom::CurrentRev() const {
    return const_cast<RndLightProbeCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x49CE50.
bool RndLightProbeCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x49CE80.
Component* RndLightProbeCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x49CE90. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightProbeCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightProbeCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightProbeCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x49D080.
PropRegistry& RndLightProbeCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x49D090.
ComMetaData& RndLightProbeCom::_GetMetaData() {
    return sMetaData;
}

namespace {

// Reconstructed from eboot.elf at 0x49D0A0.
void OnFalloffStartChanged(const PropAccessorArgs& args) {
    static_cast<RndLightProbeCom*>(args.mComponent)->_SyncFalloffStart();
}

// Reconstructed from eboot.elf at 0x49D0C0.
void OnFalloffEndChanged(const PropAccessorArgs& args) {
    static_cast<RndLightProbeCom*>(args.mComponent)->_SyncFalloffEnd();
}

}  // namespace

// Reconstructed from eboot.elf at 0x4046B0. The binary emits the factory
// with the class's Init (0x3F04E0), before the renderer's components.
Component* RndLightProbeCom::_Create() {
    return new RndLightProbeCom();
}

// Reconstructed from eboot.elf at 0x3F04E0. The binary emits the registration
// in render/RndInit.o.
void RndLightProbeCom::Init() {
    InitComponentClass<RndLightProbeCom>();
}
