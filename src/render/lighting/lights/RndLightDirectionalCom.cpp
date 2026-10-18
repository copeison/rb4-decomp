// render/RndLightDirectionalCom.o (0x470600 to 0x47730F). The registry's
// tag functions (0x475F80 to 0x477230) are the lambdas in _Init.
#include "render/lighting/lights/RndLightDirectionalCom.h"

#include <cmath>
#include <cstring>
#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "os/memory/MemMgr.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraCom.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/deferred/RndLightDirectionalDeferredShader.h"
#include "render/lighting/lights/RndLightProbeCom.h"
#include "render/lighting/shadows/RndShaderLightDirectionalShadowGen.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/shaders/RndShader.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture2DResource.h"
#include "utl/text/MakeString.h"

namespace {

// The shared header's null id and thread-group widths, at 0x1A87628, which
// the static initializer (0x477240) sets; nothing in the object reads them.
// Names not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFu};
[[maybe_unused]] int gLightDirectionalGroupSize2D = 8;  // 0x1A8762C
[[maybe_unused]] int gLightDirectionalGroupSize3D = 4;  // 0x1A87630

// Divisors and lengths are kept this far from zero.
constexpr float kEpsilon = 0.0001F;

// The 16-byte constant at the shader's offset in the buffer.
float* Constant(RndShaderCBuffer* cbuffer, unsigned long offset) {
    return reinterpret_cast<float*>(static_cast<char*>(cbuffer->mData) + offset * 16);
}

// The transform as the three rows of a 3x4 matrix, as the shaders read it.
void StoreRows(const Transform& xfm, float* rows) {
    const Hmx::Matrix3& m = xfm.m;
    const float values[12] = {
        m.x.x, m.y.x, m.z.x, xfm.v.x,
        m.x.y, m.y.y, m.z.y, xfm.v.y,
        m.x.z, m.y.z, m.z.z, xfm.v.z,
    };
    std::memcpy(rows, values, sizeof(values));
}

// The resource at the path among those inlined into the entity's layers,
// or null. Inlined into _OnResourcesLoaded. Name not in the reference map.
Resource* FindInlinedResource(const Entity* entity, ResourcePath path) {
    const EntityResource* resource = entity->mResource;
    for (const EntityResource::LayerInfo& layer : resource->mLayers) {
        for (Resource* inlined : layer.mInlineResources) {
            if (inlined->mPath.mPath == path.mPath) {
                return inlined;
            }
        }
    }
    return nullptr;
}

// The property callbacks _Init installs (0x475950 to 0x475A10). Names not
// in the reference map.
void OnCastsShadowsChanged(const PropAccessorArgs& args);
void OnDistanceFracChanged(const PropAccessorArgs& args);
void OnMinSoftnessChanged(const PropAccessorArgs& args);
void OnMaxSoftnessChanged(const PropAccessorArgs& args);

}  // namespace

// The object's statics, in the order of its static initializer (0x477240).
Symbol RndLightDirectionalCom::sId("LightDirectional");
Symbol RndLightDirectionalCom::sClassName("LightDirectional");
PropRegistry RndLightDirectionalCom::sPropRegistry;
ComMetaData RndLightDirectionalCom::sMetaData;

// Reconstructed from eboot.elf at 0x470600. The cascades start as identity
// transforms (RuntimeData's constructor).
RndLightDirectionalCom::RndLightDirectionalCom()
    : mCookieTileSize{10.0F, 10.0F},
      mCastsShadows(false),
      mMaxDistance(100.0F),
      mNumCascades(1),
      mMaxOffscreenOccluderDist(50.0F),
      mSoften(true),
      mMinSoftness(0.05F),
      mMaxSoftness(1.0F),
      mMaxSoftnessDist(5.0F),
      mOnlyFlaggedObjects(true),
      mCastContext(3),
      mShadowOffset(0.05F) {}

// Reconstructed from eboot.elf at 0x475A30.
RndLightDirectionalCom::RndLightDirectionalCom(const RndLightDirectionalCom& other)
    : RndLightCom(other),
      mCookie(other.mCookie),
      mCookieTileSize{other.mCookieTileSize[0], other.mCookieTileSize[1]},
      mCastsShadows(other.mCastsShadows),
      mMaxDistance(other.mMaxDistance),
      mNumCascades(other.mNumCascades),
      mMaxOffscreenOccluderDist(other.mMaxOffscreenOccluderDist),
      mSoften(other.mSoften),
      mMinSoftness(other.mMinSoftness),
      mMaxSoftness(other.mMaxSoftness),
      mMaxSoftnessDist(other.mMaxSoftnessDist),
      mOnlyFlaggedObjects(other.mOnlyFlaggedObjects),
      mCastContext(other.mCastContext),
      mShadowOffset(other.mShadowOffset),
      mRuntimeData() {
    mDistanceFracs._Copy(other.mDistanceFracs);
    mQualitySettings._Copy(other.mQualitySettings);
}

// Reconstructed from eboot.elf at 0x470810.
RndLightDirectionalCom::~RndLightDirectionalCom() {
    RndShaderCBuffer::SafeDelete(mRuntimeData.mDeferredCBuffer);
    RndShaderCBuffer::SafeDelete(mRuntimeData.mShadowGenCBuffer);
}

// Reconstructed from eboot.elf at 0x4709A0. Every shadow setting is
// read-only while the light casts no shadows. Revision 1 gives lights saved
// before it the four cascade distances and the three quality levels.
void RndLightDirectionalCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<ShadowQualitySettings>(Symbol("ShadowQualitySettings"));
    RndLightCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Directional light";
    metadata.mAuthor = "Daniel Sproul";
    RndLightDirectionalCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            RndLightDirectionalCom* const light = static_cast<RndLightDirectionalCom*>(args.mComponent);
            light->mDistanceFracs.Resize(4);
            light->mQualitySettings.Resize(3);
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    PropRegistry& shadows =
        static_cast<GroupMetadata&>(*registry.RegisterProp("shadows", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));

    BoolMetadata& castsShadows = TypeSpecificMetadata(
        shadows.RegisterProp(
            "casts_shadows", offsetof(RndLightDirectionalCom, mCastsShadows), kPropertyBool, 1),
        prototype.mCastsShadows);
    castsShadows.mDefault = prototype.mCastsShadows;
    static_cast<void>(Symbol("prop"));
    castsShadows.mHelp = String("Does this light cast shadows?");
    castsShadows.mOnChanged = OnCastsShadowsChanged;

    BoolMetadata& onlyFlagged = TypeSpecificMetadata(
        shadows.RegisterProp(
            "only_flagged_objects", offsetof(RndLightDirectionalCom, mOnlyFlaggedObjects), kPropertyBool, 1),
        prototype.mOnlyFlaggedObjects);
    onlyFlagged.mDefault = prototype.mOnlyFlaggedObjects;
    static_cast<void>(Symbol("prop"));
    onlyFlagged.mHelp = String("Are shadows cast only from objects specifically tagged to do so?");
    onlyFlagged.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });

    IntMetadata<unsigned int>& castContext = TypeSpecificMetadata(
        shadows.RegisterProp("cast_context", offsetof(RndLightDirectionalCom, mCastContext), kPropertyUInt, 4),
        prototype.mCastContext);
    castContext.mDefault = prototype.mCastContext;
    static_cast<void>(Symbol("prop"));
    castContext.mBitfield = true;
    castContext.mAllowedValues = eastl::vector<AllowedValue<unsigned int>>{
        AllowedValue<unsigned int>{1, String("Volumetric"), String("Shadows affect volumetrics.")},
        AllowedValue<unsigned int>{2, String("Geometry"), String("Shadows are cast on geometry.")}};
    castContext.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });

    FloatMetadata& maxDistance = TypeSpecificMetadata(
        shadows.RegisterProp("max_distance", offsetof(RndLightDirectionalCom, mMaxDistance), kPropertyFloat, 4),
        prototype.mMaxDistance);
    maxDistance.mDefault = prototype.mMaxDistance;
    static_cast<void>(Symbol("prop"));
    maxDistance.mHelp = String("Maximum distance from the scene camera that an object can receive shadows");
    maxDistance.mUnits = Symbol("m");
    maxDistance.mMin = 0.0F;
    maxDistance.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });

    GroupMetadata& cascadesMetadata =
        static_cast<GroupMetadata&>(*shadows.RegisterProp("cascades", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    cascadesMetadata.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });
    PropRegistry& cascades = cascadesMetadata.mRegistry;

    IntMetadata<unsigned int>& numCascades = TypeSpecificMetadata(
        cascades.RegisterProp("num_cascades", offsetof(RndLightDirectionalCom, mNumCascades), kPropertyUInt, 4),
        prototype.mNumCascades);
    numCascades.mDefault = prototype.mNumCascades;
    static_cast<void>(Symbol("prop"));
    numCascades.mHelp = String(
        "How many cascades should we use? (more cascades generally means better-looking shadows across a range "
        "of distances, but costs more in terms of both CPU and GPU time)");
    numCascades.mAllowedValues = eastl::vector<AllowedValue<unsigned int>>{
        AllowedValue<unsigned int>{1, String(""), String("")},
        AllowedValue<unsigned int>{2, String(""), String("")},
        AllowedValue<unsigned int>{3, String(""), String("")},
        AllowedValue<unsigned int>{4, String(""), String("")}};

    PropInfo& fracs = cascades.RegisterPropArray(
        "distance_fracs",
        offsetof(RndLightDirectionalCom, mDistanceFracs),
        sizeof(PropArray<float>));
    ArrayMetadata& fracsMetadata = static_cast<ArrayMetadata&>(*fracs.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    fracsMetadata.mFixedSize = true;
    fracsMetadata.mHelp = String("Fractional distance from the scene camera each cascade ends at");
    PropRegistry::RegisterPropArrayItem(fracs, kPropertyFloat, sizeof(float));
    FloatMetadata& frac = TypeSpecificMetadata(fracsMetadata.mItemInfo, float());
    static_cast<void>(Symbol("item"));
    frac.mDisplayName._Set([](const TagArgs& args) {
        const RndLightDirectionalCom& light = static_cast<const RndLightDirectionalCom&>(args.mComponent);
        const unsigned long index = args.mPath[-1].mValue;
        const float distance = light.mMaxDistance * light.mDistanceFracs[index];
        FormatString format("%d (%.1f m)");
        format << index << distance;
        return String(format.Str());
    });
    frac.mMin = 0.0F;
    frac.mMax = 1.0F;
    frac.mSlider = true;
    frac.mOnChanged = OnDistanceFracChanged;
    frac.mReadOnly._Set([](const TagArgs& args) {
        const RndLightDirectionalCom& light = static_cast<const RndLightDirectionalCom&>(args.mComponent);
        return args.mPath[-1].mValue >= light.mNumCascades;
    });

    FloatMetadata& occluderDist = TypeSpecificMetadata(
        shadows.RegisterProp(
            "max_offscreen_occluder_dist",
            offsetof(RndLightDirectionalCom, mMaxOffscreenOccluderDist),
            kPropertyFloat,
            4),
        prototype.mMaxOffscreenOccluderDist);
    occluderDist.mDefault = prototype.mMaxOffscreenOccluderDist;
    static_cast<void>(Symbol("prop"));
    occluderDist.mHelp = String(
        "How far offscreen can an object be and still cast shadows? (the higher this is, the lower the "
        "effective precision of the shadowmap)");
    occluderDist.mMin = 0.0F;
    occluderDist.mUnits = Symbol("m");
    occluderDist.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });

    GroupMetadata& softeningMetadata =
        static_cast<GroupMetadata&>(*shadows.RegisterProp("softening", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    softeningMetadata.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });
    PropRegistry& softening = softeningMetadata.mRegistry;

    BoolMetadata& soften = TypeSpecificMetadata(
        softening.RegisterProp("soften", offsetof(RndLightDirectionalCom, mSoften), kPropertyBool, 1),
        prototype.mSoften);
    soften.mDefault = prototype.mSoften;
    static_cast<void>(Symbol("prop"));
    soften.mHelp = String("Soften the shadows?");

    FloatMetadata& minSoftness = TypeSpecificMetadata(
        softening.RegisterProp("min_softness", offsetof(RndLightDirectionalCom, mMinSoftness), kPropertyFloat, 4),
        prototype.mMinSoftness);
    minSoftness.mDefault = prototype.mMinSoftness;
    static_cast<void>(Symbol("prop"));
    minSoftness.mHelp = String("Minimum softness of shadows when nearest to occluder.");
    minSoftness.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mSoften;
    });
    minSoftness.mOnChanged = OnMinSoftnessChanged;
    minSoftness.mMin = 0.0F;
    minSoftness.mMax = 5.0F;
    minSoftness.mSlider = true;

    FloatMetadata& maxSoftness = TypeSpecificMetadata(
        softening.RegisterProp("max_softness", offsetof(RndLightDirectionalCom, mMaxSoftness), kPropertyFloat, 4),
        prototype.mMaxSoftness);
    maxSoftness.mDefault = prototype.mMaxSoftness;
    static_cast<void>(Symbol("prop"));
    maxSoftness.mHelp = String("Maximum softness of shadows when further than max_softness_dist from occluder.");
    maxSoftness.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mSoften;
    });
    maxSoftness.mOnChanged = OnMaxSoftnessChanged;
    maxSoftness.mMin = 0.0F;
    maxSoftness.mMax = 5.0F;
    maxSoftness.mSlider = true;

    FloatMetadata& softnessDist = TypeSpecificMetadata(
        softening.RegisterProp(
            "max_softness_dist", offsetof(RndLightDirectionalCom, mMaxSoftnessDist), kPropertyFloat, 4),
        prototype.mMaxSoftnessDist);
    softnessDist.mDefault = prototype.mMaxSoftnessDist;
    static_cast<void>(Symbol("prop"));
    softnessDist.mHelp = String("Distance from occluder at which shadows are at max softness.");
    softnessDist.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mSoften;
    });
    softnessDist.mUnits = Symbol("m");
    softnessDist.mMin = 0.0F;
    softnessDist.mMax = 20.0F;
    softnessDist.mSlider = true;

    FloatMetadata& shadowOffset = TypeSpecificMetadata(
        shadows.RegisterProp("shadow_offset", offsetof(RndLightDirectionalCom, mShadowOffset), kPropertyFloat, 4),
        prototype.mShadowOffset);
    shadowOffset.mDefault = prototype.mShadowOffset;
    static_cast<void>(Symbol("prop"));
    shadowOffset.mHelp = String("Increase this offset to remove shadow acne artifacts.");
    shadowOffset.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });
    shadowOffset.mMin = 0.0F;
    shadowOffset.mMax = 0.2F;
    shadowOffset.mSlider = true;

    PropInfo& quality = shadows.RegisterPropArray(
        "quality_settings",
        offsetof(RndLightDirectionalCom, mQualitySettings),
        sizeof(PropArray<ShadowQualitySettings>));
    ArrayMetadata& qualityMetadata = static_cast<ArrayMetadata&>(*quality.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    qualityMetadata.mFixedSize = true;
    qualityMetadata.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightDirectionalCom&>(args.mComponent).mCastsShadows;
    });
    PropRegistry::RegisterPropArrayItem(quality, kPropertyStruct, sizeof(ShadowQualitySettings));
    GroupMetadata& level = static_cast<GroupMetadata&>(*qualityMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const ShadowQualitySettings levelPrototype = {true, true};
    level.mSummary._Set([](const TagArgs& args) {
        return String(RndQualityLevelDisplayName(static_cast<RndQualityLevel>(args.mPath[-1].mValue)));
    });
    PropRegistry& levelRegistry = level.mRegistry;

    BoolMetadata& levelCasts = TypeSpecificMetadata(
        levelRegistry.RegisterProp(
            "casts_shadows", offsetof(ShadowQualitySettings, mCastsShadows), kPropertyBool, 1),
        levelPrototype.mCastsShadows);
    levelCasts.mDefault = levelPrototype.mCastsShadows;
    static_cast<void>(Symbol("prop"));
    levelCasts.mHelp = String("Should this light cast shadows at this quality level?");

    BoolMetadata& levelSoften = TypeSpecificMetadata(
        levelRegistry.RegisterProp(
            "soften_shadows", offsetof(ShadowQualitySettings, mSoftenShadows), kPropertyBool, 1),
        levelPrototype.mSoftenShadows);
    levelSoften.mDefault = levelPrototype.mSoftenShadows;
    static_cast<void>(Symbol("prop"));
    levelSoften.mHelp = String("Should shadows be softened at this quality level?");
    levelSoften.mReadOnly._Set([](const TagArgs& args) {
        const RndLightDirectionalCom& light = static_cast<const RndLightDirectionalCom&>(args.mComponent);
        if (!light.mSoften) {
            return true;
        }
        return !light.mQualitySettings[static_cast<int>(args.mPath[-2].mValue)].mCastsShadows;
    });

    ResourcePathMetadata& cookie = TypeSpecificMetadata(
        registry.RegisterProp("cookie", offsetof(RndLightDirectionalCom, mCookie), kPropertyResourcePath, 8),
        prototype.mCookie);
    cookie.mDefault = prototype.mCookie;
    static_cast<void>(Symbol("prop"));
    cookie.mHelp = String("Projected cookie texture, multiplied with the light color");
    cookie.mResourceId = RndTexture2DResource::Id();

    GroupMetadata& tileSize = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "cookie_tile_size",
        offsetof(RndLightDirectionalCom, mCookieTileSize), kPropertyStruct, 8).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    tileSize.mHelp = String("Tile size of the projected cookie texture");
    tileSize.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndLightDirectionalCom&>(args.mComponent).mCookie == ResourcePath();
    });
    // The members' defaults come from a zeroed (u, v) pair, not the
    // prototype.
    const float tilePrototype[2] = {0.0F, 0.0F};
    FloatMetadata& tileU =
        TypeSpecificMetadata(tileSize.mRegistry.RegisterProp("u", 0, kPropertyFloat, 4), tilePrototype[0]);
    tileU.mDefault = tilePrototype[0];
    static_cast<void>(Symbol("prop"));
    tileU.mMin = 0.0F;
    FloatMetadata& tileV = TypeSpecificMetadata(
        tileSize.mRegistry.RegisterProp("v", sizeof(float), kPropertyFloat, 4),
        tilePrototype[1]);
    tileV.mDefault = tilePrototype[1];
    static_cast<void>(Symbol("prop"));
    tileV.mMin = 0.0F;
}

// Reconstructed from eboot.elf at 0x472D80.
void RndLightCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("RndLightCom", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

// Reconstructed from eboot.elf at 0x473920.
void RndLightDirectionalCom::_PostCreate() {
    mDistanceFracs.Resize(4);
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x473960. A cookie inlined into the
// entity's resource is used before one loaded from its path; a changed
// cookie unlinks the old texture.
bool RndLightDirectionalCom::_OnResourcesLoaded() {
    RndLightCom::_OnResourcesLoaded();
    if (mRuntimeData.mDeferredCBuffer == nullptr) {
        mRuntimeData.mDeferredCBuffer =
            RndShaderCBuffer::New(*gRndDevice->mLighting.mDirectionalShader->mCBufferConfig, 0);
    }
    const ResourcePtr<RndTexture2DResource> previous(mRuntimeData.mCookieTexture.Get());
    if (mCookie == ResourcePath()) {
        mRuntimeData.mCookieTexture = nullptr;
    } else if (Resource* inlined = FindInlinedResource(mObject->mEntity, mCookie)) {
        mRuntimeData.mCookieTexture = ResourcePtr<RndTexture2DResource>(static_cast<RndTexture2DResource*>(inlined));
    } else {
        mRuntimeData.mCookieTexture = Resource::GetOrLoad<RndTexture2DResource>(mCookie, false);
    }
    if (previous.Get() != mRuntimeData.mCookieTexture.Get()) {
        if (previous.Get() != nullptr && !previous->Fail()) {
            previous->mTexture->SetLinkedTexture(nullptr, -1);
        }
        _CookieTextureChanged();
    }
    _SyncShadowResources();
    return true;
}

// Reconstructed from eboot.elf at 0x473B50. The entity is built in
// temporary memory, copied into a lasting allocation that replaces the
// resource's entity, and destroyed; the camera is then found in the copy.
// The parent component id passed is constructed from the empty string.
void RndLightDirectionalCom::_SyncShadowResources() {
    if (!mCastsShadows) {
        return;
    }
    if (mRuntimeData.mShadowCamera == nullptr) {
        mRuntimeData.mShadowEntity = new RndSceneResource();
        unsigned int temp;
        MemPushTemp(temp, true, true);
        Entity* const entity = mRuntimeData.mShadowEntity->CreateEntity();
        GameObject* const object = entity->CreateObject(0, 0);
        const GameObjectId id = object->mId;
        object->CreateComponent(RndCameraCom::sClassName, false);
        MemPopTemp(temp);
        Entity* const copy = entity->Clone(nullptr, false, nullptr, Symbol(""));
        mRuntimeData.mShadowEntity->SwapEntity(copy);
        if (entity != nullptr) {
            entity->_Destroy();
        }
        mRuntimeData.mShadowCamera = copy->GetObject(id)->GetCom<RndCameraCom>();
    }
    if (mRuntimeData.mShadowGenCBuffer == nullptr) {
        mRuntimeData.mShadowGenCBuffer = RndShaderCBuffer::New(
            *gRndDevice->mLighting.mDirectionalShadowGenShader->mCBufferConfig, 0);
    }
}

// Reconstructed from eboot.elf at 0x473D30.
void RndLightDirectionalCom::_Poll() {
    RndLightCom::_Poll();
}

// Reconstructed from eboot.elf at 0x473D40.
int RndLightDirectionalCom::_GetCapabilitiesImpl() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x473D50.
Symbol RndLightDirectionalCom::_GetIntensityUnitsImpl() const {
    return Symbol("W/m^2");
}

// Reconstructed from eboot.elf at 0x473DA0. Tiny falloff ranges are pushed
// out to the epsilon, keeping their sign.
void RndLightDirectionalCom::_PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) {
    RndComputeBufferStructs::RndCSLightDirectional data = {};
    _GetComputeShaderData(ctx.mCameras[0], data);
    const RndLightDirectionalDeferredShader* shader = gRndDevice->mLighting.mDirectionalShader;
    RndShaderCBuffer* cbuffer = mRuntimeData.mDeferredCBuffer;
    std::memcpy(Constant(cbuffer, shader->mLightColor), data.mColor, sizeof(data.mColor));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mLightWrapParams), data.mLightWrapParams, sizeof(data.mLightWrapParams));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mCookieScale), data.mCookieScale, sizeof(data.mCookieScale));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mLightDirCamSpace), data.mDirection, sizeof(data.mDirection));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mCamToLightXfm), data.mCamToLightXfm, sizeof(data.mCamToLightXfm));
    cbuffer->mSyncPending = true;

    if (const RndLightProbeCom* light = probe.mProbe) {
        *Constant(cbuffer, shader->mProbeBlendAmount) = probe.mStateBlend;
        float range = light->mFalloffEnd - light->mFalloffStart;
        if (range < 0.0F ? range > -kEpsilon : range < kEpsilon) {
            range = range < 0.0F ? -kEpsilon : kEpsilon;
        }
        const float scale = 1.0F / range;
        float* falloff = Constant(cbuffer, shader->mProbeFalloffParams);
        falloff[0] = scale;
        falloff[1] = -(light->mFalloffStart * scale);
        falloff[2] = static_cast<float>(light->mFalloffFunction);
        cbuffer->mSyncPending = true;

        Transform worldToProbe;
        Invert(light->mRuntime.mTrans->mWorldXfm, worldToProbe);
        Transform camToProbe;
        Multiply(ctx.mCameras[0].mViews[0].mWorldXfm, worldToProbe, camToProbe);
        StoreRows(camToProbe, Constant(cbuffer, shader->mCamToProbeXfm));
        cbuffer->mSyncPending = true;
    }
    if (cbuffer->mSyncPending) {
        cbuffer->_SyncImpl(ctx, 0, cbuffer->mNumElements);
        cbuffer->mSyncPending = false;
    }
}

// Reconstructed from eboot.elf at 0x4741D0.
void RndLightDirectionalCom::_GetComputeShaderData(
    const RndCameraContext& camera,
    RndComputeBufferStructs::RndCSLightDirectional& data) const {
    data.mVolumetric = mVolumetric;
    data.mIlluminationType = mIlluminationType;
    data.mEnvironBits = mRuntime.mEnvironBits;
    data.mCookieIndex = mRuntime.mTiledCookieIndex != static_cast<unsigned long>(-1)
        ? (mRuntime.mCookieTexArrayIndex << 11) + static_cast<int>(mRuntime.mTiledCookieIndex)
        : -1;

    const float intensity = mRuntime.mMasterIntensityMult * mIntensity;
    if (mIlluminationType == 3) {
        const float blend = intensity < 1.0F ? intensity : 1.0F;
        const Hmx::Color& white = Hmx::Color::GetWhite();
        data.mColor[0] = (mColor.red - white.red) * blend + white.red;
        data.mColor[1] = (mColor.green - white.green) * blend + white.green;
        data.mColor[2] = (mColor.blue - white.blue) * blend + white.blue;
    } else {
        data.mColor[0] = intensity * mColor.red;
        data.mColor[1] = intensity * mColor.green;
        data.mColor[2] = intensity * mColor.blue;
    }

    if (mLightWrap > 2.0F) {
        const float wrap = mLightWrap * 0.25F;
        const float diffuse = 1.0F - wrap;
        data.mLightWrapParams[0] = diffuse > 0.0F ? diffuse : 0.0F;
        data.mLightWrapParams[1] = wrap;
    } else {
        const float scale = 1.0F / (mLightWrap > kEpsilon ? mLightWrap : kEpsilon);
        data.mLightWrapParams[0] = scale;
        data.mLightWrapParams[1] = 1.0F - scale;
    }

    // The light travels along the object's -y axis.
    const Vector3& axis = mRuntime.mTrans->mWorldXfm.m.y;
    const RndCameraContext::View& view = camera.mViews[0];
    const Hmx::Matrix3& rotation = view.mWorldRotationTranspose;
    const float x = -(axis.x * rotation.x.x + axis.y * rotation.y.x + axis.z * rotation.z.x);
    const float y = -(axis.x * rotation.x.y + axis.y * rotation.y.y + axis.z * rotation.z.y);
    const float z = -(axis.x * rotation.x.z + axis.y * rotation.y.z + axis.z * rotation.z.z);
    const float length = std::sqrt(x * x + y * y + z * z);
    const float scale = length != 0.0F ? 1.0F / length : 0.0F;
    data.mDirection[0] = scale * x;
    data.mDirection[1] = scale * y;
    data.mDirection[2] = scale * z;

    Transform camToLight;
    Multiply(view.mWorldXfm, mRuntime.mDrawNode->mRuntime.mInvWorldXfm, camToLight);
    StoreRows(camToLight, &data.mCamToLightXfm[0][0]);

    data.mCookieScale[0] = 1.0F / (mCookieTileSize[0] > kEpsilon ? mCookieTileSize[0] : kEpsilon);
    data.mCookieScale[1] = -1.0F / (mCookieTileSize[1] > kEpsilon ? mCookieTileSize[1] : kEpsilon);
}

// Reconstructed from eboot.elf at 0x474530. The probe's second-state
// textures make the shader combine with the probe; the blend the probe
// returns is not used.
void RndLightDirectionalCom::_DrawDeferredNoComputeImpl(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    const RndLightProbeParams& probe) {
    static_cast<void>(buffers);
    static_cast<void>(target);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Directional Light");
    }
    static Symbol sProbeStatName;
    if (sProbeStatName == Symbol()) {
        sProbeStatName = Symbol("Directional Light + Probe");
    }
    RndScopedGpuStatBlock statBlock(ctx, probe.mProbe != nullptr ? sProbeStatName.Str() : sStatName.Str());
    mRuntimeData.mDeferredCBuffer->_SelectImpl(ctx);

    RndLightDirectionalDeferredShader::Params params;
    params.mShadowMap = nullptr;
    params.mCookie = nullptr;
    params.mCombineWithProbe = false;
    params.mCombineWithProbeBlend = false;
    std::memset(params.mProbeTextures, 0, sizeof(params.mProbeTextures));
    params.mIllumType = mIlluminationType;
    params.mCookie = RndLightDirectionalCom::_GetCookieTextureImpl();
    if (probe.mProbe != nullptr) {
        float blend;
        // The shader's slots hold the cube textures as RndTextureBase.
        if (probe.mProbe->_GetBlendTextures(
                probe.mStateIndices[0],
                probe.mStateIndices[1],
                reinterpret_cast<RndTextureCube**>(params.mProbeTextures[0]),
                reinterpret_cast<RndTextureCube**>(params.mProbeTextures[1]),
                &blend,
                probe.mStateBlend)) {
            params.mCombineWithProbe = true;
        }
    }
    gRndDevice->mLighting.mDirectionalShader->Select(ctx, params);
    ctx._SetDepthModeImpl(0);
    ctx._SetCullModeImpl(kCullNone);

    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    quad.mKeepState = true;
    RndDrawUtl::DrawQuad2D(ctx, quad);
}

// Reconstructed from eboot.elf at 0x4747E0. The light's data is appended to
// the buffer's staging copy.
void RndLightDirectionalCom::_AddToComputeBufferImpl(
    RndQualityLevel quality,
    const RndCameraContext& camera,
    RndComputeBuffer& buffer) {
    static_cast<void>(quality);
    RndComputeBufferStructs::RndCSLightDirectional data = {};
    _GetComputeShaderData(camera, data);
    std::memcpy(static_cast<char*>(buffer.mStagingData) + buffer.mStagingSize, &data, sizeof(data));
    buffer.mStagingSize += sizeof(data);
}

// Reconstructed from eboot.elf at 0x4748C0.
RndTextureBase* RndLightDirectionalCom::_GetCookieTextureImpl() const {
    RndTexture2DResource* cookie = mRuntimeData.mCookieTexture.Get();
    if (cookie == nullptr || cookie->Fail()) {
        return nullptr;
    }
    return cookie->mTexture;
}

// Reconstructed from eboot.elf at 0x474900.
bool RndLightDirectionalCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    if (!mCastsShadows) {
        return false;
    }
    return mQualitySettings[static_cast<int>(quality)].mCastsShadows;
}

// Reconstructed from eboot.elf at 0x474EE0. Bit 3 of the TransCom's flags
// marks a world transform set from outside.
void RndLightDirectionalCom::_SetCascadeCamera(unsigned int cascade, const Frustum& frustum) {
    const ShadowCascade camera = _CalcCascadeCamera(frustum);
    RndCameraCom* shadowCamera = mRuntimeData.mShadowCamera;
    shadowCamera->mOrthographic = true;
    shadowCamera->mNearDistance = camera.mNearPlane;
    shadowCamera->mFarDistance = camera.mFarPlane;
    if (!shadowCamera->mPixelAccurate) {
        shadowCamera->mOrthoHeight = camera.mOrthoHeight;
    }
    TransCom* trans = shadowCamera->mObject->GetExistingCom<TransCom>();
    trans->mWorldXfm = camera.mXfm;
    trans->mDirtyFlags |= 8;
    mRuntimeData.mCascades[cascade] = camera;
}

// Reconstructed from eboot.elf at 0x474FE0.
bool RndLightDirectionalCom::_AcquireDeferredShadowContributionResourcesImpl(
    bool castsShadows,
    RndLightMgrCom& mgr) {
    mRuntimeData.mShadowMapLayer = -1;
    mRuntimeData.mShadowContributionIndex = -1;
    if (!castsShadows) {
        return false;
    }
    mRuntimeData.mShadowMapLayer = mgr.AcquireSpotShadowDepthLayers(mNumCascades);
    if (mRuntimeData.mShadowMapLayer == -1) {
        return false;
    }
    mRuntimeData.mShadowContributionIndex = mgr.AcquireShadowContribution();
    return mRuntimeData.mShadowContributionIndex != -1 && mRuntimeData.mShadowMapLayer != -1;
}

// Reconstructed from eboot.elf at 0x475730.
Symbol RndLightDirectionalCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x475740.
Symbol RndLightDirectionalCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x475750.
int RndLightDirectionalCom::CurrentRev() const {
    return const_cast<RndLightDirectionalCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x475770.
bool RndLightDirectionalCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4757A0.
Component* RndLightDirectionalCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4757B0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightDirectionalCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightDirectionalCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightDirectionalCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4758C0.
PropRegistry& RndLightDirectionalCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4758D0.
ComMetaData& RndLightDirectionalCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4758E0.
int RndLightDirectionalCom::_GetTypeImpl() const {
    return 2;
}

namespace {

// Reconstructed from eboot.elf at 0x475950.
void OnCastsShadowsChanged(const PropAccessorArgs& args) {
    static_cast<RndLightDirectionalCom*>(args.mComponent)->_SyncShadowResources();
}

// Reconstructed from eboot.elf at 0x475960. Keeps the cascade distances in
// order: the later ones end no nearer than the new one, the earlier ones
// no farther.
void OnDistanceFracChanged(const PropAccessorArgs& args) {
    PropArray<float>& fracs = static_cast<RndLightDirectionalCom*>(args.mComponent)->mDistanceFracs;
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-1].mValue;
    const float value = *static_cast<const float*>(args.mRequestedValue);
    for (unsigned long i = index + 1; i < 4; ++i) {
        fracs[i] = value > fracs[i] ? value : fracs[i];
    }
    for (unsigned long i = index; i != 0; --i) {
        fracs[i - 1] = value < fracs[i - 1] ? value : fracs[i - 1];
    }
}

// Reconstructed from eboot.elf at 0x4759F0.
void OnMinSoftnessChanged(const PropAccessorArgs& args) {
    RndLightDirectionalCom* const light = static_cast<RndLightDirectionalCom*>(args.mComponent);
    light->mMaxSoftness = light->mMinSoftness > light->mMaxSoftness ? light->mMinSoftness : light->mMaxSoftness;
}

// Reconstructed from eboot.elf at 0x475A10.
void OnMaxSoftnessChanged(const PropAccessorArgs& args) {
    RndLightDirectionalCom* const light = static_cast<RndLightDirectionalCom*>(args.mComponent);
    light->mMinSoftness = light->mMaxSoftness < light->mMinSoftness ? light->mMaxSoftness : light->mMinSoftness;
}

}  // namespace

// Reconstructed from eboot.elf at 0x404610. The binary emits the factory
// with the class's Init; the placement factory at 0x404640 constructs in
// given storage.
Component* RndLightDirectionalCom::_Create() {
    return new RndLightDirectionalCom();
}

// Reconstructed from eboot.elf at 0x3EFD40. The binary emits the
// registration in render/RndInit.o.
void RndLightDirectionalCom::Init() {
    InitComponentClass<RndLightDirectionalCom>();
}
