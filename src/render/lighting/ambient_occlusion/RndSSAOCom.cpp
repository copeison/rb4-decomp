// render/RndSSAOCom.o (0x4AC310 to 0x4ADB7F). The object also emits the
// templates of PropArray<RndSSAOCom::QualitySettings> (0x4AC420,
// 0x4AD550-0x4AD850), the class description's type registration
// (0x4ACD10) and the registry's property callbacks (0x4AD860-0x4ADAA0).
#include "render/lighting/ambient_occlusion/RndSSAOCom.h"

#include <new>

#include "entity/props/PropMetadata.h"
#include "math/color/Color.h"
#include "math/random/Rand.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/lighting/ambient_occlusion/RndCShaderSSAOGen.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTexture2D.h"
#include "utl/containers/FixedVector.h"

// The object's statics, in the order of its static initializer (0x4ADAB0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndSSAOCom::sId("SSAO");
Symbol RndSSAOCom::sClassName("SSAO");
PropRegistry RndSSAOCom::sPropRegistry;
ComMetaData RndSSAOCom::sMetaData;

namespace {

constexpr int kNoiseSize = 256;
// The radius is given in pixels of a 1080-line target.
constexpr float kReferenceHeight = 1080.0F;

// A noise channel in [0.1, 1).
float NoiseChannel() {
    return gRand.Float() * 0.9F + 0.1F;
}

}  // namespace

// Reconstructed from eboot.elf at 0x4AC310.
RndSSAOCom::RndSSAOCom()
    : mRadius(1.0F), mAngleBias(0.2F), mIntensity(1.0F) {}

// Reconstructed from eboot.elf at 0x4AC390. The deleting destructor is at
// 0x4AC480.
RndSSAOCom::~RndSSAOCom() {
    delete mRuntimeData.mNoiseTexture;
    mRuntimeData.mNoiseTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x4AC540. Each quality level's settings
// are summarized by the level's name; the help of their "enabled" is
// RndCMAACom's. Revision 1 gives components saved before it the three
// quality levels. The binary emits the struct registration at 0x4ACD10.
void RndSSAOCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<QualitySettings>(Symbol("RndSSAOCom::QualitySettings"));
    metadata.mDescription = "Generates and applies screen space ambient occlusion";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    RndSSAOCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            static_cast<RndSSAOCom*>(args.mComponent)->mQualitySettings.Resize(3);
        };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    PropInfo& quality = registry.RegisterPropArray(
        "quality_settings", offsetof(RndSSAOCom, mQualitySettings), sizeof(PropArray<QualitySettings>));
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

    FloatMetadata& radius = TypeSpecificMetadata(
        registry.RegisterProp("radius", offsetof(RndSSAOCom, mRadius), kPropertyFloat, 4), prototype.mRadius);
    radius.mDefault = prototype.mRadius;
    static_cast<void>(Symbol("prop"));
    radius.mHelp = String(
        "Radius in which to search for occluders. Smaller values give a more localized occlusion effect.");
    radius.mMin = 0.1F;
    radius.mMax = 3.0F;
    radius.mSlider = true;

    FloatMetadata& angleBias = TypeSpecificMetadata(
        registry.RegisterProp("angleBias", offsetof(RndSSAOCom, mAngleBias), kPropertyFloat, 4),
        prototype.mAngleBias);
    angleBias.mDefault = prototype.mAngleBias;
    static_cast<void>(Symbol("prop"));
    angleBias.mHelp =
        String("Angle threshold to ignore as occluder. Increase this value if you are seeing artifacting.");
    angleBias.mMin = 0.0F;
    angleBias.mMax = 1.0F;
    angleBias.mSlider = true;

    FloatMetadata& intensity = TypeSpecificMetadata(
        registry.RegisterProp("intensity", offsetof(RndSSAOCom, mIntensity), kPropertyFloat, 4),
        prototype.mIntensity);
    intensity.mDefault = prototype.mIntensity;
    static_cast<void>(Symbol("prop"));
    intensity.mHelp = String(
        "Intensity of the occlusion. Increase this to get a greater effect from the ambient occlusion.");
    intensity.mMin = 0.0F;
    intensity.mMax = 10.0F;
    intensity.mSlider = true;
}

// Reconstructed from eboot.elf at 0x4ACEC0.
void RndSSAOCom::_PostCreate() {
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x4ACED0. A texture made by an earlier
// load is not freed. The pixels are filled column by column.
bool RndSSAOCom::_OnResourcesLoaded() {
    RndTexture2D::Description desc;
    // A 32-bit four-channel linear format.
    const RndDataFormatInfo info{32, 4, 0, 1, -1};
    desc.mName = "ssao_noise";
    desc.mRequestedFormat.mWrapMode = 2;
    desc.mRequestedFormat.mFilterMode = 2;
    const int dataFormat = RndFindSupportedDataFormat(info, kPlatformPS4);
    RndPixelCanvas canvas;
    canvas.CreateUninitialized(kNoiseSize, kNoiseSize, 1);
    for (int x = 0; x < kNoiseSize; ++x) {
        for (int y = 0; y < kNoiseSize; ++y) {
            const float r = NoiseChannel();
            const float g = NoiseChannel();
            const float b = NoiseChannel();
            const float a = NoiseChannel();
            canvas.mPixels[canvas.mWidth * y + x] = Hmx::Color(r, g, b, a);
        }
    }
    desc.mPixels.CreateEmpty(kNoiseSize, kNoiseSize, 1, dataFormat);
    desc.mPixels.ConvertFrom(canvas);
    mRuntimeData.mNoiseTexture = RndTexture2D::New(desc);
    return true;
}

// Reconstructed from eboot.elf at 0x4AD140.
bool RndSSAOCom::IsEnabled(RndQualityLevel level) const {
    return mQualitySettings[static_cast<int>(level)].mEnabled;
}

// Reconstructed from eboot.elf at 0x4AD150. The AO buffer leaves the
// shader-resource states for the dispatch and returns to them.
void RndSSAOCom::GenerateAO(
    RndContext& context,
    RndBufferCollection& buffers,
    RndCameraContext& camera,
    const RndSceneDrawParams& params) {
    if (context.mTargetMode == kTargetModeCube) {
        return;
    }
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("SSAO");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    FixedVector<RndResourceBarrier, 1> barriers;
    RndResourceBarrier barrier;
    barrier.mResource =
        buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval].mAO;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = RndResourceState::kAllShaderResource;
    barrier.mAfter = RndResourceState::kUnorderedAccess;
    barriers.push_back(barrier);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndCShaderSSAOGen::Params ssao;
    ssao.mBuffers = &buffers;
    ssao.mCamera = &camera;
    ssao.mNoiseTexture = mRuntimeData.mNoiseTexture;
    ssao.mRadius = static_cast<float>(buffers.mSize.y)
        * (1.0F / kReferenceHeight) * mRadius;
    ssao.mAngleBias = mAngleBias;
    ssao.mIntensity = mIntensity;
    ssao.mUseSceneMask = params.mDrawSceneMask;
    TheRndDevice()->mShaderMgr.mSSAOGenCShader->Dispatch(context, ssao);

    barriers[0].mBefore = RndResourceState::kUnorderedAccess;
    barriers[0].mAfter = RndResourceState::kAllShaderResource;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
}

// Reconstructed from eboot.elf at 0x4AD340.
Symbol RndSSAOCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4AD350.
Symbol RndSSAOCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4AD360.
int RndSSAOCom::CurrentRev() const {
    return const_cast<RndSSAOCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4AD380.
bool RndSSAOCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4AD3B0.
Component* RndSSAOCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4AD3C0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndSSAOCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSSAOCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSSAOCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4AD530.
PropRegistry& RndSSAOCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4AD540.
ComMetaData& RndSSAOCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x404750. The binary emits the factory
// with the class's Init (0x3F0CA0); the placement factory at 0x404780
// constructs in given storage.
Component* RndSSAOCom::_Create() {
    return new RndSSAOCom();
}

// Reconstructed from eboot.elf at 0x3F0CA0. The binary emits the
// registration in render/RndInit.o.
void RndSSAOCom::Init() {
    InitComponentClass<RndSSAOCom>();
}
