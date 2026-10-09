// render/RndMaterialCom.o (0x4F1BD0 to 0x4F662F). The object also emits the
// DynamicCom<RndShaderGraphResource> members (see entity/core/DynamicCom.h)
// and its static initializer at 0x4F6560.
#include "render/materials/RndMaterialCom.h"

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/materials/RndMaterialPuppetCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/shadergraph/RndShaderGraph.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "utl/data/DataArray.h"
#include "utl/text/MakeString.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x4F658C; see RndTypesetter.cpp). Names not in
// the reference map.
[[maybe_unused]] int gMaterialGroupSizeUnused = -1;  // 0x1A8B910
[[maybe_unused]] int gMaterialGroupSize2D = 8;       // 0x1A8B914
[[maybe_unused]] int gMaterialGroupSize3D = 4;       // 0x1A8B918

// Whether the instancing object of the material's entity holds a
// MaterialPuppet component; such a shared material is left to the puppet.
// Inlined into _SetUnique and _OnResourcesLoaded. Name not in the reference
// map; the map's _IsPuppet(ObjPtr const&) const may be its source.
bool IsPuppetedMaterial(const RndMaterialCom& material) {
    if (material.mUnique) {
        return false;
    }
    const GameObject* parent = material.mObject->mEntity->mParentObject;
    return parent != nullptr && parent->GetCom<RndMaterialPuppetCom>() != nullptr;
}

}  // namespace

// The object's statics, in the order of its static initializer.
Symbol RndMaterialCom::sId("Material");
Symbol RndMaterialCom::sClassName("Material");
PropRegistry RndMaterialCom::sPropRegistry;
ComMetaData RndMaterialCom::sMetaData;

// Reconstructed from eboot.elf at 0x4F1BD0.
void RndMaterialCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    InitPropRegistry(registry);
    _InitMetaData(metadata);
    RegisterBasePropRegistryInit(sClassName, InitPropRegistry);
}

// Reconstructed from eboot.elf at 0x4F3240.
Symbol RndMaterialCom::GetMaterialProviderId() {
    return Symbol("MaterialProvider");
}

// Reconstructed from eboot.elf at 0x4F3290.
RndMaterialCom::RndMaterialCom()
    : mSharing(RndMaterialSharing::kAutomatic),
      mBucket(0),
      mBlendMode(RndBlendMode::kSource),
      mBlendFactor(Hmx::Color::GetWhite()),
      mCullMode(1),
      mReceiveAtmosphere(true),
      mReceiveDecals(true),
      mDepthPrepass(false),
      mForceOpaque(false),
      mSceneMask(false),
      mUnique(false),
      mSharingDirty(false),
      mRenderStateDirty(false),
      mFirstRuntimeData(true),
      mIsPuppet(false),
      mRuntimeDataResource(),
      mRuntimeData(nullptr),
      mRuntimeDataDirty(false),
      mKeepBlendMode(false) {
    mPlaymodeSink.mSink = nullptr;
    mPlaymodeSink.mSource = nullptr;
    mRecordModeSink.mSink = nullptr;
    mRecordModeSink.mSource = nullptr;
}

// Reconstructed from eboot.elf at 0x4F34F0. A linked subscription leaves its
// source before the link unlinks itself; the binary does this for each
// subscription as it is destroyed, the later one first.
RndMaterialCom::~RndMaterialCom() {
    if (mUnique && mRuntimeData != nullptr) {
        RndMaterialRuntimeData::Delete(mRuntimeData);
    }
    for (MsgSource::EventSinkElem* sink : {&mRecordModeSink, &mPlaymodeSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x4F3790.
void RndMaterialCom::SetShaderGraphFile(const char* file) {
    mFile = file;
}

// Reconstructed from eboot.elf at 0x4F3870. The body of _SyncSharing
// follows inline.
void RndMaterialCom::SetSharingType(const GameObject& owner, RndMaterialSharing sharing) {
    if (mSharing == sharing) {
        return;
    }
    mSharing = sharing;
    _SyncSharing(owner);
}

// Reconstructed from eboot.elf at 0x4F38C0.
void RndMaterialCom::_SyncSharing(const GameObject& owner) {
    mSharingDirty = false;
    bool unique;
    if (mSharing == RndMaterialSharing::kUnique) {
        unique = true;
    } else if (mSharing == RndMaterialSharing::kAutomatic) {
        unique = NeedsUniqueMaterial(owner);
    } else {
        unique = false;
    }
    _SetUnique(unique);
}

// Reconstructed from eboot.elf at 0x4F3900. The body of _ReleaseRuntimeData
// is inlined.
void RndMaterialCom::_SetUnique(bool unique) {
    if (unique == mUnique) {
        return;
    }
    _ReleaseRuntimeData();
    mUnique = unique;
    _InitRuntimeData();
    mIsPuppet = IsPuppetedMaterial(*this);
}

// Reconstructed from eboot.elf at 0x4F3A00.
void RndMaterialCom::SetBlendMode(RndBlendMode mode) {
    if (mBlendMode != mode) {
        mBlendMode = mode;
        mRenderStateDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x4F3A20.
void RndMaterialCom::_PostCreate() {
    RndDrawInstanceCom* drawInstance = mObject->GetBaseCom<RndDrawInstanceCom>();
    if (drawInstance == nullptr) {
        return;
    }
    const RndMaterialCom* material = drawInstance->_GetDefaultMaterial();
    mFile = material->mFile.Str();
    mBlendMode = material->mBlendMode;
}

// Reconstructed from eboot.elf at 0x4F3AB0. The blend-mode warning's
// message (the error name and the two blend-mode names) is compiled out;
// only MakeErrorName's call remains.
bool RndMaterialCom::_OnResourcesLoaded() {
    if (!_LoadResource()) {
        return false;
    }
    mIsPuppet = IsPuppetedMaterial(*this);
    RndShaderGraphResource* resource = mResource;
    RndShaderGraph* graph =
        resource != nullptr && !resource->Fail() ? resource->GetShaderGraph() : nullptr;
    Hmx::Color blendFactor = Hmx::Color::GetWhite();
    if (graph != nullptr) {
        if (!mKeepBlendMode
            && ((graph->GetAllowedBlendModes() >> static_cast<int>(mBlendMode)) & 1) == 0) {
            const RndBlendMode defaultMode = static_cast<RndBlendMode>(graph->GetDefaultBlendMode());
            if (defaultMode != mBlendMode) {
                MakeErrorName();
                mBlendMode = defaultMode;
                mRenderStateDirty = true;
            }
        }
        _InitRuntimeData();
        if (graph->HasBlendFactor()) {
            blendFactor = graph->GetBlendFactor();
        }
    } else {
        _InitRuntimeData();
    }
    if (blendFactor.red != mBlendFactor.red || blendFactor.green != mBlendFactor.green
        || blendFactor.blue != mBlendFactor.blue || blendFactor.alpha != mBlendFactor.alpha) {
        mBlendFactor = blendFactor;
        mRenderStateDirty = true;
    }
    if (graph != nullptr) {
        if (!graph->AllowsBucket() && mBucket != 0) {
            mBucket = 0;
            mRenderStateDirty = true;
        }
        if (!graph->AllowsCullMode() && mCullMode != 1) {
            mCullMode = 1;
        }
        if (!graph->AllowsDepthPrepass() && mDepthPrepass) {
            mDepthPrepass = false;
            mRenderStateDirty = true;
        }
    }
    resource = mResource;
    if (resource != nullptr && !resource->Fail()) {
        graph = resource->GetShaderGraph();
        if (graph != nullptr) {
            for (unsigned int hints : mRuntimeData->mUsageHints) {
                gRndDevice->PrecacheMaterialShaders(static_cast<int>(mBlendMode), hints, graph->mShaders);
            }
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x4F3E50. A shared material looks for the
// runtime data of its graph in the entity's inlined "mat_rt:" resources
// first; new data of a shared material is inlined under that path into the
// object's layer.
void RndMaterialCom::_InitRuntimeData() {
    RndShaderGraphResource* graph = mResource;
    if (graph != nullptr && graph->Fail()) {
        graph = nullptr;
    }
    ResourcePath path;
    bool reused = false;
    if (!mUnique) {
        const GameObjectId id = mObject->mId;
        ResourcePath file;
        if (mFile == ResourcePath()) {
            file = "null";
        } else {
            file = mFile;
        }
        FormatString format("mat_rt:%s:%d_%d");
        format << file.mPath << id.Layer() << (id.mId >> 16);
        path = format.Str();
        ResourcePtr<RndMaterialRuntimeDataResource> found =
            mObject->mEntity->mResource->TryGetInline<RndMaterialRuntimeDataResource>(path);
        if (found.Get() != mRuntimeDataResource.Get()) {
            _ReleaseRuntimeData();
            mRuntimeDataResource = found.Get();
            if (found != nullptr && !found->Fail()) {
                reused = true;
                mRuntimeData = mRuntimeDataResource->mData;
            }
        }
    }
    if (mRuntimeData != nullptr && !mRuntimeData->IsValid(graph)) {
        _ReleaseRuntimeData();
        reused = false;
    }
    if (mRuntimeData == nullptr) {
        mRuntimeData = RndMaterialRuntimeData::New(graph);
        _SyncRenderState();
        mRuntimeData->SyncCBufferMaterialParams();
        mRuntimeData->SyncCBufferExposedProps(mPropStorage);
        mRuntimeData->SyncCBufferStatic();
        if (!mUnique) {
            mRuntimeDataResource = new RndMaterialRuntimeDataResource(mRuntimeData);
            mRuntimeDataResource->SetFile(path, false);
            EntityResource* entityResource = mObject->mEntity->mResource;
            ResourcePtr<RndMaterialRuntimeDataResource> old =
                entityResource->TryGetInline<RndMaterialRuntimeDataResource>(path);
            if (old != nullptr) {
                entityResource->UninlineResource(old);
            }
            entityResource->InlineResource(mRuntimeDataResource, mObject->mId.Layer());
        }
    }
    if (!mFirstRuntimeData || !reused) {
        mRuntimeData->LoadExposedTextures(*this);
    }
    mFirstRuntimeData = false;
}

// Reconstructed from eboot.elf at 0x4F4380.
void RndMaterialCom::_Poll() {
    _PollRuntimeData();
}

// Reconstructed from eboot.elf at 0x4F43A0. The queue keeps the data's
// RndDynamicGpuData base until the drawer syncs it on the render thread.
void RndMaterialCom::_PollRuntimeData() {
    if (mIsPuppet) {
        return;
    }
    if (mRenderStateDirty) {
        _SyncRenderState();
    }
    if (!mRuntimeDataDirty) {
        return;
    }
    mRuntimeDataDirty = false;
    const unsigned int flags = mRuntimeData->mSyncFlags.fetch_or(
        RndMaterialRuntimeData::kSyncQueued | RndMaterialRuntimeData::kSyncExposedProps);
    if ((flags & RndMaterialRuntimeData::kSyncQueued) != 0) {
        return;
    }
    RndSceneDrawer* drawer = RndSceneDrawer::FindForEntity(mObject->mEntity);
    if (drawer == nullptr) {
        mRuntimeData->mSyncFlags &= 0;
        mRuntimeDataDirty = true;
        return;
    }
    if (mMgr == nullptr) {
        drawer->mDynamicGpuDataMgr->Enqueue(*this);
    }
}

// Reconstructed from eboot.elf at 0x4F45C0. DynamicCom's edit poll is
// inlined.
void RndMaterialCom::_EditPoll() {
    DynamicCom::_EditPoll();
    _PollRuntimeData();
}

// Reconstructed from eboot.elf at 0x4F4670.
void RndMaterialCom::_SyncDynamicGpuDataImpl(RndContext& context) {
    mRuntimeData->SyncCBufferMaterialParams();
    if ((mRuntimeData->mSyncFlags & RndMaterialRuntimeData::kSyncExposedProps) != 0) {
        mRuntimeData->SyncCBufferExposedProps(mPropStorage);
    }
    mRuntimeData->SyncCBufferGlobals(context);
    mRuntimeData->mSyncFlags &= 0;
}

// Reconstructed from eboot.elf at 0x4F4730.
void RndMaterialCom::_SyncRenderState() {
    RndMaterialRuntimeData* data = mRuntimeData;
    const Hmx::Color blendFactor = mBlendFactor;
    if (data->mBlendMode != static_cast<int>(mBlendMode) || data->mBucket != mBucket
        || data->mDepthPrepass != mDepthPrepass || data->mForceOpaque != mForceOpaque
        || data->mSceneMask != mSceneMask) {
        data->mBlendMode = static_cast<int>(mBlendMode);
        data->mBucket = mBucket;
        data->mDepthPrepass = mDepthPrepass;
        data->mForceOpaque = mForceOpaque;
        data->mSceneMask = mSceneMask;
        data->InitUsageHints();
    }
    data->mBlendFactor = blendFactor;
    mRenderStateDirty = false;
}

// Reconstructed from eboot.elf at 0x4F47D0. The resource's Fail call remains
// of a compiled-out assertion.
void RndMaterialCom::_ReleaseRuntimeData() {
    if (mRuntimeData == nullptr) {
        return;
    }
    if (mUnique) {
        RndMaterialRuntimeData::Delete(mRuntimeData);
    } else {
        if (mRuntimeDataResource != nullptr) {
            static_cast<void>(mRuntimeDataResource->Fail());
        }
        mRuntimeDataResource = nullptr;
    }
    mRuntimeData = nullptr;
}

// Reconstructed from eboot.elf at 0x4F4840.
DataNode RndMaterialCom::_OnLeavingPlaymode() {
    if (!mUnique) {
        mRuntimeDataDirty = true;
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x4F4870.
DataNode RndMaterialCom::_OnLeavingRecordMode() {
    if (!mUnique) {
        mRuntimeDataDirty = true;
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x4F48A0.
DataNode RndMaterialCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol leavingPlaymode;
    if (leavingPlaymode == Symbol()) {
        leavingPlaymode = Symbol("leaving_playmode");
    }
    if (type == leavingPlaymode) {
        return _OnLeavingPlaymode();
    }
    static Symbol leavingRecordMode;
    if (leavingRecordMode == Symbol()) {
        leavingRecordMode = Symbol("leaving_record_mode");
    }
    if (type == leavingRecordMode) {
        return _OnLeavingRecordMode();
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x4F4A00.
Symbol RndMaterialCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4F4A10.
Symbol RndMaterialCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4F4A20.
int RndMaterialCom::CurrentRev() const {
    return const_cast<RndMaterialCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4F4A40.
bool RndMaterialCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4F4A70.
Component* RndMaterialCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4F4D40.
ComMetaData& RndMaterialCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4F4E10.
PropRegistry& RndMaterialCom::_GetBasePropRegistry() {
    return sPropRegistry;
}
