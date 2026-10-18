// render/RndCMAACom.o (0x44E9E0 to 0x45048F). The object also emits the
// templates of PropArray<RndCMAACom::QualitySettings> (0x44EAC0,
// 0x44FE60-0x450150) and the registry's property callbacks
// (0x450170-0x4503B0).
#include "render/postprocessing/antialiasing/RndCMAACom.h"

#include <cstdint>
#include <new>

#include "entity/props/PropMetadata.h"
#include "math/color/Color.h"
#include "render/buffers/RndCShaderClearBuffer.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgeDetect.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgePrune.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAFinalProcess.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAShapeFit.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"

// The object's statics, in the order of its static initializer (0x4503C0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndCMAACom::sId("AntiAliasing");
Symbol RndCMAACom::sClassName("AntiAliasing");
PropRegistry RndCMAACom::sPropRegistry;
ComMetaData RndCMAACom::sMetaData;

namespace {

// The capability flag that CMAA, as a compute pass, needs; the other users
// call it the async-compute feature.
constexpr std::uint32_t kAsyncComputeFeature = 0x10;

// The light accumulation buffer the target selects, or the frame
// interval's own one outside a scene context. Inlined at each use.
RndTextureBase* LightAccum(
    const RndBufferCollection& buffers,
    unsigned long index) {
    if (buffers.mActiveSceneContext != 0) {
        return buffers.mLightAccum[index];
    }
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
        .mPartialLightAccum;
}

void SetBarrier(
    RndResourceBarrier& barrier,
    RndTextureBase* resource,
    RndResourceState before,
    RndResourceState after) {
    barrier.mResource = resource;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = before;
    barrier.mAfter = after;
}

// Unbinds the compute stage's read-write and source textures after a
// dispatch, resetting the stage's slot limits. Inlined after every
// dispatch.
void DeselectComputeTextures(RndContext& context) {
    constexpr unsigned int kComputeStage = 1U << kShaderProgramCompute;
    context._DeselectAllReadWriteTexturesImpl(kComputeStage);
    context.mInputSlotLimits[kShaderProgramCompute] = 0;
    context._DeselectAllSourceTexturesImpl(kComputeStage);
    context.mOutputSlotLimits[kShaderProgramCompute] = 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x44E9E0.
RndCMAACom::RndCMAACom()
    : mEdgeThreshold(0.05F), mNonDominantEdgeThreshold(0.5F) {}

// Reconstructed from eboot.elf at 0x44EA50. The deleting destructor is at
// 0x44EB20.
RndCMAACom::~RndCMAACom() {}

// Reconstructed from eboot.elf at 0x44EBB0. Each quality level's settings
// are summarized by the level's name; revision 1 gives components saved
// before it the three quality levels. The binary emits the struct
// registration at 0x44F260.
void RndCMAACom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<QualitySettings>(Symbol("RndCMAACom::QualitySettings"));
    metadata.mDescription = "Performs anti aliasing on the scene.";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    RndCMAACom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    PropInfo& quality = registry.RegisterPropArray(
        "quality_settings", offsetof(RndCMAACom, mQualitySettings), sizeof(PropArray<QualitySettings>));
    ArrayMetadata& qualityMetadata = static_cast<ArrayMetadata&>(*quality.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    qualityMetadata.mFixedSize = true;
    qualityMetadata.mAnimatable = false;
    PropRegistry::RegisterPropArrayItem(quality, kPropertyStruct, sizeof(QualitySettings));
    GroupMetadata& level = static_cast<GroupMetadata&>(*qualityMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const QualitySettings levelPrototype;
    level.mSummary._Set([](const TagArgs& args) {
        return String(RndQualityLevelName(static_cast<RndQualityLevel>(args.mPath[-1].mValue)));
    });
    PropRegistry& levelRegistry = level.mRegistry;

    BoolMetadata& levelEnabled = TypeSpecificMetadata(
        levelRegistry.RegisterProp("enabled", offsetof(QualitySettings, mEnabled), kPropertyBool, 1),
        levelPrototype.mEnabled);
    levelEnabled.mDefault = levelPrototype.mEnabled;
    static_cast<void>(Symbol("prop"));
    levelEnabled.mHelp = String("Is antialiasing enabled at this quality level?");

    FloatMetadata& edgeThreshold = TypeSpecificMetadata(
        registry.RegisterProp("edge_threshold", offsetof(RndCMAACom, mEdgeThreshold), kPropertyFloat, 4),
        prototype.mEdgeThreshold);
    edgeThreshold.mDefault = prototype.mEdgeThreshold;
    static_cast<void>(Symbol("prop"));
    edgeThreshold.mHelp = String(
        "Threshold controlling what is considered an edge. Lower values result in more edges being "
        "antialiased.");
    edgeThreshold.mMin = 0.001F;
    edgeThreshold.mMax = 0.3F;
    edgeThreshold.mSlider = true;

    FloatMetadata& nonDominant = TypeSpecificMetadata(
        registry.RegisterProp(
            "non_dominant_edge_threshold", offsetof(RndCMAACom, mNonDominantEdgeThreshold), kPropertyFloat, 4),
        prototype.mNonDominantEdgeThreshold);
    nonDominant.mDefault = prototype.mNonDominantEdgeThreshold;
    static_cast<void>(Symbol("prop"));
    nonDominant.mHelp = String(
        "Threshold controlling what is considered an unimportant edge. Lower values result in more edges "
        "being antialiased.");
    nonDominant.mMin = 0.001F;
    nonDominant.mMax = 0.99F;
    nonDominant.mSlider = true;

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            static_cast<RndCMAACom*>(args.mComponent)->mQualitySettings.Resize(3);
        };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
    }
}

// Reconstructed from eboot.elf at 0x44F410.
void RndCMAACom::_PostCreate() {
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x44F420.
bool RndCMAACom::IsEnabled(RndQualityLevel level) const {
    return mQualitySettings[static_cast<int>(level)].mEnabled;
}

// Reconstructed from eboot.elf at 0x44F430. The source light accumulation
// is read by the edge detection and the shape fit; the color buffer, or
// the destination light accumulation without one, receives the blend.
void RndCMAACom::Draw(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target) {
    RndDevice* const device = TheRndDevice();
    if (context.mTargetMode == kTargetModeCube
        || (device->mCapabilities[kPlatformPS4].mFeatureFlags
            & kAsyncComputeFeature) == 0) {
        return;
    }
    context.SetRenderTargets(nullptr, nullptr);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("CMAA");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndTextureBase* const source = LightAccum(buffers, target.mSrcLightAccum);
    FixedVector<RndResourceBarrier, 5> barriers;
    barriers.resize(5);
    SetBarrier(
        barriers[0],
        source,
        RndResourceState::kRenderTarget,
        RndResourceState::kNonPixelShaderResource);
    SetBarrier(
        barriers[1],
        buffers.mCMAAColor,
        RndResourceState::kNonPixelShaderResource,
        RndResourceState::kUnorderedAccess);
    if (buffers.mCMAAColor == nullptr) {
        barriers[1].mResource = LightAccum(buffers, target.mDstLightAccum);
        barriers[1].mBefore = RndResourceState::kAllShaderResource;
    }
    SetBarrier(
        barriers[2],
        buffers.mCMAAEdges[0],
        RndResourceState::kNonPixelShaderResource,
        RndResourceState::kUnorderedAccess);
    SetBarrier(
        barriers[3],
        buffers.mCMAAEdges[1],
        RndResourceState::kNonPixelShaderResource,
        RndResourceState::kUnorderedAccess);
    SetBarrier(
        barriers[4],
        buffers.mCMAACompressedEdges,
        RndResourceState::kNonPixelShaderResource,
        RndResourceState::kUnorderedAccess);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndShaderMgr& shaders = device->mShaderMgr;
    if (buffers.mCMAAState == 0) {
        static Symbol sInitStatName;
        if (sInitStatName == Symbol()) {
            sInitStatName = Symbol("CMAA Init");
        }
        RndScopedGpuStatBlock initStatBlock(context, sInitStatName.Str());
        RndCShaderClearBuffer* const clear = shaders.mClearBufferCShader;
        for (RndTextureBase* edges : buffers.mCMAAEdges) {
            RndCShaderClearBuffer::Params params;
            params.mTexture = edges;
            params.mClearValue = Hmx::Color::GetBlack();
            clear->Dispatch(context, params);
        }
        DeselectComputeTextures(context);
    }
    const unsigned long parity = buffers.mCMAAState & 1;

    shaders.mCMAAEdgeDetectCShader->Dispatch(
        context, buffers, target, mEdgeThreshold);
    DeselectComputeTextures(context);
    barriers.resize(2);
    barriers[0].mPhase = RndResourceBarrierPhase::kBegin;
    SetBarrier(
        barriers[0],
        source,
        RndResourceState::kNonPixelShaderResource,
        RndResourceState::kUnorderedAccess);
    SetBarrier(
        barriers[1],
        buffers.mCMAACompressedEdges,
        RndResourceState::kUnorderedAccess,
        RndResourceState::kNonPixelShaderResource);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    shaders.mCMAAEdgePruneCShader->Dispatch(
        context, buffers, parity, mNonDominantEdgeThreshold);
    DeselectComputeTextures(context);
    barriers[0].mPhase = RndResourceBarrierPhase::kEnd;
    SetBarrier(
        barriers[0],
        buffers.mCMAAEdges[parity],
        RndResourceState::kUnorderedAccess,
        RndResourceState::kNonPixelShaderResource);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    shaders.mCMAAShapeFitCShader->Dispatch(context, buffers, target, parity);
    DeselectComputeTextures(context);
    barriers[0].mPhase = RndResourceBarrierPhase::kImmediate;
    SetBarrier(
        barriers[0],
        buffers.mCMAAColor,
        RndResourceState::kUnorderedAccess,
        RndResourceState::kNonPixelShaderResource);
    if (buffers.mCMAAColor == nullptr) {
        barriers[0].mResource = LightAccum(buffers, target.mDstLightAccum);
        barriers[0].mAfter = RndResourceState::kAllShaderResource;
    }
    SetBarrier(
        barriers[1],
        buffers.mCMAAEdges[1 - parity],
        RndResourceState::kUnorderedAccess,
        RndResourceState::kNonPixelShaderResource);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    shaders.mCMAAFinalProcessCShader->Dispatch(
        context, buffers, target, parity);
    barriers.resize(1);
    SetBarrier(
        barriers[0],
        source,
        RndResourceState::kUnorderedAccess,
        RndResourceState::kRenderTarget);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    ++buffers.mCMAAState;
}

// Reconstructed from eboot.elf at 0x44FC60.
Symbol RndCMAACom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x44FC70.
Symbol RndCMAACom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x44FC80.
int RndCMAACom::CurrentRev() const {
    return const_cast<RndCMAACom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x44FCA0.
bool RndCMAACom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x44FCD0.
Component* RndCMAACom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x44FCE0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndCMAACom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndCMAACom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndCMAACom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x44FE40.
PropRegistry& RndCMAACom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x44FE50.
ComMetaData& RndCMAACom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4041B0. The binary emits the factory
// with the class's Init (0x3EC880); the placement factory at 0x4041E0
// constructs in given storage.
Component* RndCMAACom::_Create() {
    return new RndCMAACom();
}

// Reconstructed from eboot.elf at 0x3EC880. The binary emits the
// registration in render/RndInit.o.
void RndCMAACom::Init() {
    InitComponentClass<RndCMAACom>();
}
