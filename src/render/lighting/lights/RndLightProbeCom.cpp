// render/RndLightProbeCom.o (0x498080-0x49DB9F). The object's std::function
// handlers for the registry (0x49D0A0-0x49D910), the vector growth
// helpers (0x49AA20, 0x49D0E0, 0x49D920) and the unreferenced description
// setup at 0x49AB00 are not reconstructed.
#include "render/lighting/lights/RndLightProbeCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/geometry/Sphere.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "os/platform/PlatformMgr.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneCom.h"
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

// Reconstructed from eboot.elf at 0x4046B0. The binary emits the factory
// with the class's Init (0x3F04E0), before the renderer's components.
Component* RndLightProbeCom::_Create() {
    return new RndLightProbeCom();
}
