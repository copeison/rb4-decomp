// render/RndParticleCom.o (0x601880 to 0x61E1F5). The object also emits the
// extension list's insert and erase (0x601E50, 0x601F60), the copies of the
// parameter structures (0x619580-0x61A66B), the per-chunk waveform helpers
// (0x614AF0, 0x615970), Waveform<float>::ValueRange (0x6178D0),
// WaveformResource<float>::ValueRange (0x61DD30) and the resize of the
// new-particle data (0x61DE40); they are not reconstructed. The registry's
// accessors (0x618540-0x61912F) and lambdas (0x61A670-0x61DD2F) are.
#include "render/particles/RndParticleCom.h"

#include <cfloat>
#include <cmath>
#include <functional>
#include <new>
#include <utility>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "math/geometry/Sphere.h"
#include "math/random/Rand.h"
#include "math/rotation/Rot.h"
#include "os/memory/MemMgr.h"
#include "render/buffers/RndParticleBuffer.h"
#include "render/defaults/RndDefaults.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshResource.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/particles/RndParticleAffectorCom.h"
#include "render/particles/RndParticleColliderCom.h"
#include "render/particles/RndParticleEmitterCom.h"
#include "render/particles/RndParticleExtCom.h"
#include "render/particles/RndParticleVelocityAlignCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/system/RndDevice.h"

// The object's statics, in the order of its static initializer (0x61DF50).
Symbol RndParticleCom::sId("PSys");
Symbol RndParticleCom::sClassName("PSys");
PropRegistry RndParticleCom::sPropRegistry;
ComMetaData RndParticleCom::sMetaData;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultPitch;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultYaw;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultEmitRate;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultSpeedMultiplierOverLife;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultLifespan;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultSpeed;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultInitialSize;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultPivot;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultZPivot;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultScaleOverLife;
ResourcePtr<WaveformResource<Hmx::Color>> RndParticleCom::sDefaultColor;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultAlpha;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultXRotationAngle;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultYRotationAngle;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultRotationAngle;
ResourcePtr<WaveformResource<float>> RndParticleCom::sDefaultExtraData;
RndMesh* RndParticleCom::sDefaultMesh;

namespace {

// The invalid GameObjectId, the shared header's -1 (0x1AA8B58). Name not in
// the reference map.
constexpr unsigned int kInvalidObjectId = 0xFFFFFFFF;

// The component with base class T on the object with the id, or null. The
// lookup is inlined into each loop over "affectors". Name not in the
// reference map.
template <typename T>
T* FindBaseCom(const Entity* entity, GameObjectId id) {
    if (id.mId == kInvalidObjectId) {
        return nullptr;
    }
    const GameObject* object = entity->GetObject(id);
    if (object == nullptr) {
        return nullptr;
    }
    return object->GetBaseCom<T>();
}

// The degree conversions of the orientation accessors.
constexpr float kRadiansToDegrees = 57.295776F;
constexpr float kDegreesToRadians = 0.017453292F;
constexpr float kPi = 3.1415927F;
constexpr float kTwoPi = 6.2831855F;

// The alignment a revision 0 save's value stands for: the camera XY
// alignment came later (0x12A4E70). Name not in the reference map.
constexpr int kRev0Alignments[4] = {
    RndParticleCom::kCameraAligned,
    RndParticleCom::kXAxisAligned,
    RndParticleCom::kYAxisAligned,
    RndParticleCom::kZAxisAligned};

// What the revision handler does for a saved property it cannot read: it
// builds the component's error name and the path's text, whose report the
// release build strips. Inlined three times. Name not in the reference map.
void ReportMissingSavedProp(const Component& saved, const PropPath& path) {
    saved.MakeErrorName();
    char buffer[512] = {};
    path.ToString(buffer, sizeof(buffer));
}

// Registers a vector struct's "x", "y" and "z" with the prototype's
// components as their defaults. Inlined into _Init for each vector struct.
// Name not in the reference map.
void RegisterVector3Members(PropRegistry& registry, const Vector3& prototype) {
    FloatMetadata& x = TypeSpecificMetadata(registry.RegisterProp("x", 0, kPropertyFloat, 4), prototype.x);
    x.mDefault = prototype.x;
    FloatMetadata& y = TypeSpecificMetadata(registry.RegisterProp("y", 4, kPropertyFloat, 4), prototype.y);
    y.mDefault = prototype.y;
    FloatMetadata& z = TypeSpecificMetadata(registry.RegisterProp("z", 8, kPropertyFloat, 4), prototype.z);
    z.mDefault = prototype.z;
}

// The property accessors _Init installs (0x618540 to 0x61912F). Names not
// in the reference map.
void OnParticleTypeChanged(const PropAccessorArgs& args);
void OnMaxCountChanged(const PropAccessorArgs& args);
Waveform<float> GetInitialZRotationAngle(const PropAccessorArgs& args);
void SetInitialZRotationAngle(const PropAccessorArgs& args);
float GetXOrientation(const PropAccessorArgs& args);
void SetXOrientation(const PropAccessorArgs& args);
float GetYOrientation(const PropAccessorArgs& args);
void SetYOrientation(const PropAccessorArgs& args);
float GetZOrientation(const PropAccessorArgs& args);
void SetZOrientation(const PropAccessorArgs& args);
Waveform<float> GetZRpmOverLife(const PropAccessorArgs& args);
void SetZRpmOverLife(const PropAccessorArgs& args);
float GetZRpmDrag(const PropAccessorArgs& args);
void SetZRpmDrag(const PropAccessorArgs& args);

}  // namespace

// Reconstructed from eboot.elf at 0x601880. A unit box on the axes.
void RndParticleCom::StaticInit() {
    RndMeshUtl::CreateBoxParams params;
    params.mAxisX = {1.0F, 0.0F, 0.0F};
    params.mAxisY = {0.0F, 1.0F, 0.0F};
    params.mAxisZ = {0.0F, 0.0F, 1.0F};
    params.mNumSegmentsX = 1;
    params.mNumSegmentsY = 1;
    params.mNumSegmentsZ = 1;
    sDefaultMesh = RndMeshUtl::CreateBox(params);
}

// Reconstructed from eboot.elf at 0x601910.
void RndParticleCom::StaticTerminate() {
    delete sDefaultMesh;
    sDefaultMesh = nullptr;
}

// Reconstructed from eboot.elf at 0x601940.
RndParticleCom::RndParticleCom()
    : RndDrawInstanceCom(),
      mEmitter(),
      mParticle(),
      mColor(),
      mSpin(),
      mExtraData(),
      mRender(),
      mTimeUnits(kTimeUnitsSeconds),
      mSimWhenHidden(false),
      mCollision(),
      mAffectors(),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x601B90. The members release their
// resources; the base leaves the scene drawer's list.
RndParticleCom::~RndParticleCom() {}

// Reconstructed from eboot.elf at 0x601FC0. The waveforms are built with
// the waveform default and then take the class's.
RndParticleCom::EmitterParams::EmitterParams()
    : mDuration(30.0F),
      mEmitCanKillOldest(false),
      mNeverKill(false),
      mMinBoxExtents(Vector3::sZero),
      mMaxBoxExtents(Vector3::sZero),
      mPitch(),
      mYaw(),
      mBirthMomentum(false),
      mBirthMomentumAmount(1.0F),
      mSubsamples(0),
      mLocalSpace(false) {
    mPitch.SetResource(sDefaultPitch.Get());
    mYaw.SetResource(sDefaultYaw.Get());
}

// Inlined into ~RndParticleCom (0x601B90).
RndParticleCom::EmitterParams::~EmitterParams() {}

// Reconstructed from eboot.elf at 0x602200. The three scale waveforms share
// sDefaultScaleOverLife, and the x and y pivots sDefaultPivot.
RndParticleCom::ParticleParams::ParticleParams()
    : mParticleType(kSprite),
      mMeshResourcePath(),
      mMaxCount(100),
      mEmitRate(),
      mEmitRateMultiplier(1.0F),
      mEmitDistancePerParticle(0.0F),
      mLifespan(),
      mLifespanMultiplier(1.0F),
      mSpeed(),
      mSpeedMultiplierOverLife(),
      mDrag(0.0F),
      mInitialSize(),
      mXScaleOverLife(),
      mYScaleOverLife(),
      mZScaleOverLife(),
      mUniformScale(true),
      mXPivotOverLife(),
      mYPivotOverLife(),
      mZPivotOverLife(),
      mOffsetThenScale(true),
      mForceDirection(Vector3::sZero) {
    mEmitRate.SetResource(sDefaultEmitRate.Get());
    mLifespan.SetResource(sDefaultLifespan.Get());
    mSpeed.SetResource(sDefaultSpeed.Get());
    mSpeedMultiplierOverLife.SetResource(sDefaultSpeedMultiplierOverLife.Get());
    mInitialSize.SetResource(sDefaultInitialSize.Get());
    mXScaleOverLife.SetResource(sDefaultScaleOverLife.Get());
    mYScaleOverLife.SetResource(sDefaultScaleOverLife.Get());
    mZScaleOverLife.SetResource(sDefaultScaleOverLife.Get());
    mXPivotOverLife.SetResource(sDefaultPivot.Get());
    mYPivotOverLife.SetResource(sDefaultPivot.Get());
    mZPivotOverLife.SetResource(sDefaultZPivot.Get());
}

// Reconstructed from eboot.elf at 0x602D70.
RndParticleCom::ColorParams::ColorParams()
    : mColor(sDefaultColor.Get()), mAlpha(sDefaultAlpha.Get()) {}

// Inlined into ~RndParticleCom (0x601B90).
RndParticleCom::ColorParams::~ColorParams() {}

// Reconstructed from eboot.elf at 0x602EC0.
RndParticleCom::SpinParams::SpinParams()
    : mInitialRotation(kUnrotated),
      mInitialXRotationAngle(),
      mInitialYRotationAngle(),
      mInitialRotationAngle(),
      mOrientation(Hmx::Matrix3::sID),
      mXRpmOverLife(),
      mYRpmOverLife(),
      mRpmOverLife(),
      mXRpmDrag(0.0F),
      mYRpmDrag(0.0F),
      mRpmDrag(0.0F) {
    mInitialXRotationAngle.SetResource(sDefaultXRotationAngle.Get());
    mInitialYRotationAngle.SetResource(sDefaultYRotationAngle.Get());
    mInitialRotationAngle.SetResource(sDefaultRotationAngle.Get());
}

// Reconstructed from eboot.elf at 0x6033F0. The channels start disabled.
RndParticleCom::ExtraDataParams::ExtraDataParams()
    : mExtraDataType0(kExtraDataDisabled),
      mExtraDataType1(kExtraDataDisabled),
      mExtraDataType2(kExtraDataDisabled),
      mExtraData0(),
      mExtraData1(),
      mExtraData2() {
    mExtraData0.SetResource(sDefaultExtraData.Get());
    mExtraData1.SetResource(sDefaultExtraData.Get());
    mExtraData2.SetResource(sDefaultExtraData.Get());
}

// Inlined into RndParticleCom's copy constructor (0x619130).
RndParticleCom::ExtraDataParams::ExtraDataParams(const ExtraDataParams& other) = default;

// Inlined into ~RndParticleCom (0x601B90).
RndParticleCom::ExtraDataParams::~ExtraDataParams() {}

// Reconstructed from eboot.elf at 0x6036F0. The cached components and the
// mesh are left for _OnResourcesLoaded to set.
RndParticleCom::RuntimeData::RuntimeData()
    : mParticleTypeChanged(false),
      mExtensions(),
      mMeshResource(),
      mClock(nullptr),
      mTime(0.0F),
      mLastPosition(Vector3::sZero),
      mNumActive(0),
      mRateRemainder(0.0F),
      mDistanceTravelled(0.0F),
      mBurstCountMin(0),
      mBurstCountMax(0),
      mParticles(),
      mRand(nullptr),
      mBuffer(nullptr),
      mVelocityAligned(false),
      mNewParticleXfms(),
      mNewParticleData() {
    mLastXfm.m.x = {1.0F, 0.0F, 0.0F};
    mLastXfm.m.y = {0.0F, 1.0F, 0.0F};
    mLastXfm.m.z = {0.0F, 0.0F, 1.0F};
    mLastXfm.v = {0.0F, 0.0F, 0.0F};
    mParticleXfm.m.x = {1.0F, 0.0F, 0.0F};
    mParticleXfm.m.y = {0.0F, 1.0F, 0.0F};
    mParticleXfm.m.z = {0.0F, 0.0F, 1.0F};
    mParticleXfm.v = {0.0F, 0.0F, 0.0F};
    // The seed is the structure's address, truncated to an int.
    mRand = new Rand(static_cast<long>(static_cast<int>(reinterpret_cast<long>(this))));
}

// Inlined into ~RndParticleCom (0x601B90).
RndParticleCom::RuntimeData::~RuntimeData() {
    delete mRand;
    mRand = nullptr;
    delete mBuffer;
    mBuffer = nullptr;
}

// Reconstructed from eboot.elf at 0x603850. Builds the default waveforms the
// parameter constructors take, hides the draw instance's extra data (the
// system has its own), and registers the properties by group. Most
// properties of one particle type are hidden for the other; the spin's
// x/y/z variants are the mesh's, which the "z_*" accessors and the type
// change handler (0x618540) map onto the sprite's single rotation. Revision
// 5's handler converts the properties of older saves. Every property
// registration builds a "prop" symbol it does not use, and the struct,
// array, item and action registrations "struct", "array", "item" and
// "action". The lambdas' bodies are at 0x61A6F0 to 0x61DCB0.
void RndParticleCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndDrawInstanceCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "A basic particle system. To achieve more sophisticated particle behavior add additional components.";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryParticleSystem;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mAliases.push_back(Symbol("ParticleSystem"));
    metadata.mLightweight = true;

    sDefaultPitch = new WaveformResource<float>();
    sDefaultPitch->SetType(1);
    sDefaultPitch->mBlendMode = 2;
    sDefaultPitch->SetValue(0, 0.0F);
    sDefaultPitch->SetValue(1, 90.0F);
    sDefaultYaw = new WaveformResource<float>();
    sDefaultYaw->SetType(1);
    sDefaultYaw->mBlendMode = 2;
    sDefaultYaw->SetValue(0, 0.0F);
    sDefaultYaw->SetValue(1, 360.0F);
    sDefaultEmitRate = new WaveformResource<float>();
    sDefaultEmitRate->SetType(0);
    sDefaultEmitRate->SetValue(0, 5.0F);
    sDefaultLifespan = new WaveformResource<float>();
    sDefaultLifespan->SetType(0);
    sDefaultLifespan->SetValue(0, 1.0F);
    sDefaultSpeed = new WaveformResource<float>();
    sDefaultSpeed->SetType(0);
    sDefaultSpeed->SetValue(0, 2.0F);
    sDefaultSpeedMultiplierOverLife = new WaveformResource<float>();
    sDefaultSpeedMultiplierOverLife->SetType(0);
    sDefaultSpeedMultiplierOverLife->SetValue(0, 1.0F);
    sDefaultInitialSize = new WaveformResource<float>();
    sDefaultInitialSize->SetType(0);
    sDefaultInitialSize->SetValue(0, 2.0F);
    sDefaultScaleOverLife = new WaveformResource<float>();
    sDefaultScaleOverLife->SetType(0);
    sDefaultScaleOverLife->SetValue(0, 1.0F);
    sDefaultPivot = new WaveformResource<float>();
    sDefaultPivot->SetType(0);
    sDefaultPivot->SetValue(0, 0.5F);
    sDefaultZPivot = new WaveformResource<float>();
    sDefaultZPivot->SetType(0);
    sDefaultZPivot->SetValue(0, 0.0F);
    sDefaultColor = new WaveformResource<Hmx::Color>();
    sDefaultColor->SetType(0);
    sDefaultColor->SetValue(0, Hmx::Color::GetWhite());
    sDefaultAlpha = new WaveformResource<float>();
    sDefaultAlpha->SetType(0);
    sDefaultAlpha->SetValue(0, 1.0F);
    sDefaultXRotationAngle = new WaveformResource<float>();
    sDefaultXRotationAngle->SetType(1);
    sDefaultXRotationAngle->mBlendMode = 2;
    sDefaultXRotationAngle->SetValue(0, 0.0F);
    sDefaultXRotationAngle->SetValue(1, 360.0F);
    sDefaultYRotationAngle = new WaveformResource<float>();
    sDefaultYRotationAngle->SetType(1);
    sDefaultYRotationAngle->mBlendMode = 2;
    sDefaultYRotationAngle->SetValue(0, 0.0F);
    sDefaultYRotationAngle->SetValue(1, 360.0F);
    sDefaultRotationAngle = new WaveformResource<float>();
    sDefaultRotationAngle->SetType(1);
    sDefaultRotationAngle->mBlendMode = 2;
    sDefaultRotationAngle->SetValue(0, 0.0F);
    sDefaultRotationAngle->SetValue(1, 360.0F);
    sDefaultExtraData = new WaveformResource<float>();
    sDefaultExtraData->SetType(0);
    sDefaultExtraData->SetValue(0, 0.0F);

    RndParticleCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&own != &registry) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    // The draw instance's extra data; the system's own replaces it.
    static_cast<void>(registry.FindProp(PropPath(Symbol("extra_data")), nullptr));
    PropMetadata& extraDataStruct =
        *registry.FindProp(PropPath(Symbol("extra_data")), nullptr)->mMetadata.Get();
    static_cast<void>(Symbol("struct"));
    extraDataStruct.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
    });
    static_cast<void>(registry.FindProp(PropPath(Symbol("extra_data_1")), nullptr));
    PropMetadata& extraData1Struct =
        *registry.FindProp(PropPath(Symbol("extra_data_1")), nullptr)->mMetadata.Get();
    static_cast<void>(Symbol("struct"));
    extraData1Struct.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
    });
    static_cast<void>(registry.FindProp(PropPath(Symbol("extra_data_as_color")), nullptr));
    const PropInfo* const extraDataColorInfo =
        registry.FindProp(PropPath(Symbol("extra_data_as_color")), nullptr);
    const Hmx::Color extraDataColorType(0.0F, 0.0F, 0.0F, 1.0F);
    ColorMetadata& extraDataColor =
        TypeSpecificMetadata(const_cast<PropInfo&>(*extraDataColorInfo), extraDataColorType);
    static_cast<void>(Symbol("prop"));
    extraDataColor.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
    });
    static_cast<void>(registry.FindProp(PropPath(Symbol("extra_data_1_as_color")), nullptr));
    const PropInfo* const extraData1ColorInfo =
        registry.FindProp(PropPath(Symbol("extra_data_1_as_color")), nullptr);
    const Hmx::Color extraData1ColorType(0.0F, 0.0F, 0.0F, 1.0F);
    ColorMetadata& extraData1Color =
        TypeSpecificMetadata(const_cast<PropInfo&>(*extraData1ColorInfo), extraData1ColorType);
    static_cast<void>(Symbol("prop"));
    extraData1Color.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
    });

    // Revision 0 numbered the alignments without the camera XY one; revision
    // 1 kept the affectors in "attractors"; revision 2 had one extra data
    // switch, "extra_data" "enabled"; revision 3 had no elasticity; revision
    // 4 chose the initial rotation with two switches in "spin". The old
    // values are read from the component holding the saved properties.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            RndParticleCom& com = *static_cast<RndParticleCom*>(args.mComponent);
            Component* const saved = args.mSavedProps;
            if (args.mSavedRev <= 0) {
                com.mRender.mParticleAlignment = kRev0Alignments[com.mRender.mParticleAlignment];
            }
            if (args.mSavedRev <= 1) {
                PropPath::Node structNode;
                PropPath::Node arrayNode;
                const PropPath path(structNode = Symbol("attractors"), arrayNode = Symbol("attractors"));
                if (saved->_GetPropRegistry().FindProp(path, nullptr) != nullptr) {
                    const int count = saved->Size(path);
                    com.mAffectors.Resize(count);
                    for (int index = 0; index < count; ++index) {
                        PropPath element = path;
                        PropPath::Node indexNode;
                        element.PushNode(indexNode = static_cast<unsigned long>(index));
                        GameObjectId id = {0xFFFFFFFFU};
                        void* storage = saved;
                        const PropInfo* const info = saved->_GetPropRegistry().FindProp(element, &storage);
                        if (info == nullptr
                            || !_GetProp(*saved, element, *info, storage, &id, kPropertyGameObjectId)) {
                            ReportMissingSavedProp(*saved, element);
                        }
                        com.mAffectors.data()[index] = id;
                    }
                }
            }
            if (args.mSavedRev <= 2) {
                PropPath::Node structNode;
                PropPath::Node enabledNode;
                const PropPath path(structNode = Symbol("extra_data"), enabledNode = Symbol("enabled"));
                bool enabled = false;
                void* storage = saved;
                const PropInfo* const info = saved->_GetPropRegistry().FindProp(path, &storage);
                if (info != nullptr && _GetProp(*saved, path, *info, storage, &enabled, kPropertyBool)
                    && enabled) {
                    com.mExtraData.mExtraDataType0 = kExtraDataOverLife;
                    com.mExtraData.mExtraDataType1 = kExtraDataOverLife;
                    com.mExtraData.mExtraDataType2 = kExtraDataOverLife;
                }
            }
            if (args.mSavedRev <= 3) {
                com.mCollision.mElasticity = 1.0F;
            } else if (args.mSavedRev != 4) {
                return;
            }
            PropPath::Node spinNode;
            PropPath::Node initialSpinNode;
            const PropPath initialSpinPath(spinNode = Symbol("spin"), initialSpinNode = Symbol("initial_spin"));
            PropPath::Node inheritSpinNode;
            PropPath::Node inheritNode;
            const PropPath inheritPath(
                inheritSpinNode = Symbol("spin"), inheritNode = Symbol("inherit_orientation"));
            bool initialSpin = false;
            bool inherit = false;
            void* storage = saved;
            const PropInfo* info = saved->_GetPropRegistry().FindProp(initialSpinPath, &storage);
            if (info == nullptr
                || !_GetProp(*saved, initialSpinPath, *info, storage, &initialSpin, kPropertyBool)) {
                ReportMissingSavedProp(*saved, initialSpinPath);
            }
            storage = saved;
            info = saved->_GetPropRegistry().FindProp(inheritPath, &storage);
            if (info != nullptr) {
                _GetProp(*saved, inheritPath, *info, storage, &inherit, kPropertyBool);
            }
            com.mSpin.mInitialRotation = initialSpin ? kRandomRotation : inherit;
        };
    if (&own == &registry) {
        own.SetCurrentRev(5);
        own.mRevisionFunc = onRevision;
    }

    IntMetadata<int>& timeUnits = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "time_units", offsetof(RndParticleCom, mTimeUnits), kPropertyInt, 4).mMetadata.Get());
    timeUnits.mDefault = prototype.mTimeUnits;
    static_cast<void>(Symbol("prop"));
    timeUnits.mHelp = String("Timeline that the particle system updates on");
    timeUnits.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kTimeUnitsSeconds, String("Seconds"), String("Update based on game seconds")},
        AllowedValue<int>{1, String("Beats"), String("Update based on beats")},
        AllowedValue<int>{2, String("UI Seconds"), String("Update based on ui seconds")},
        AllowedValue<int>{3, String("Tutorial Seconds"), String("Update based on tutorial seconds")}};

    BoolMetadata& simWhenHidden = TypeSpecificMetadata(
        registry.RegisterProp("sim_when_hidden", offsetof(RndParticleCom, mSimWhenHidden), kPropertyBool, 1),
        prototype.mSimWhenHidden);
    simWhenHidden.mDefault = prototype.mSimWhenHidden;
    static_cast<void>(Symbol("prop"));
    simWhenHidden.mHelp = String("Should the particle system continue to simulate when hidden?");

    IntMetadata<int>& numActive = TypeSpecificMetadata(
        registry.RegisterProp(
            "num_active",
            offsetof(RndParticleCom, mRuntime) + offsetof(RuntimeData, mNumActive),
            kPropertyInt,
            4),
        prototype.mRuntime.mNumActive);
    numActive.mDefault = prototype.mRuntime.mNumActive;
    static_cast<void>(Symbol("prop"));
    numActive.mReadOnly = true;
    numActive.mSaves = false;

    GroupMetadata& burstCount = static_cast<GroupMetadata&>(
        *registry.RegisterProp("burst_count", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    burstCount.mHelp = String(
        "A number of particles to fire this frame. Will reset to 0 as soon as the particles are spawned.");
    burstCount.mSaves = false;
    burstCount.mKind = 1;
    PropRegistry& burstRegistry = burstCount.mRegistry;
    IntMetadata<int>& burstMin = TypeSpecificMetadata(
        burstRegistry.RegisterProp(
            "min", offsetof(RndParticleCom, mRuntime) + offsetof(RuntimeData, mBurstCountMin), kPropertyInt, 4),
        prototype.mRuntime.mBurstCountMin);
    burstMin.mDefault = prototype.mRuntime.mBurstCountMin;
    IntMetadata<int>& burstMax = TypeSpecificMetadata(
        burstRegistry.RegisterProp(
            "max", offsetof(RndParticleCom, mRuntime) + offsetof(RuntimeData, mBurstCountMax), kPropertyInt, 4),
        prototype.mRuntime.mBurstCountMax);
    burstMax.mDefault = prototype.mRuntime.mBurstCountMax;

    ActionMetadata& killAll = static_cast<ActionMetadata&>(*registry.RegisterAction("kill_all").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    killAll.mAction = [](const TagArgs& args) {
        const_cast<RndParticleCom&>(static_cast<const RndParticleCom&>(args.mComponent))._KillAllParticles();
    };

    {
        PropRegistry& emitter = static_cast<GroupMetadata&>(
            *registry.RegisterProp("emitter", offsetof(RndParticleCom, mEmitter), kPropertyStruct, sizeof(EmitterParams))
                 .mMetadata.Get()).mRegistry;
        static_cast<void>(Symbol("struct"));
        EmitterParams emitterPrototype;

        BoolMetadata& killOldest = TypeSpecificMetadata(
            emitter.RegisterProp(
                "emit_can_kill_oldest", offsetof(EmitterParams, mEmitCanKillOldest), kPropertyBool, 1),
            emitterPrototype.mEmitCanKillOldest);
        killOldest.mDefault = emitterPrototype.mEmitCanKillOldest;
        static_cast<void>(Symbol("prop"));
        killOldest.mHelp = String(
            "If necessary, can the particle system kill the oldest particle in order to emit a new one?");

        BoolMetadata& neverKill = TypeSpecificMetadata(
            emitter.RegisterProp("never_kill", offsetof(EmitterParams, mNeverKill), kPropertyBool, 1),
            emitterPrototype.mNeverKill);
        neverKill.mDefault = emitterPrototype.mNeverKill;
        static_cast<void>(Symbol("prop"));
        neverKill.mHelp = String("Should particles never die?");

        GroupMetadata& minExtents = static_cast<GroupMetadata&>(
            *emitter.RegisterProp("min_box_extents", offsetof(EmitterParams, mMinBoxExtents), kPropertyStruct, 12)
                 .mMetadata.Get());
        static_cast<void>(Symbol("struct"));
        minExtents.mKind = 1;
        minExtents.mHelp = String("Min extents of the box region which emits particles");
        RegisterVector3Members(minExtents.mRegistry, emitterPrototype.mMinBoxExtents);
        minExtents.mReadOnly._Set([](const TagArgs& args) {
            return args.mComponent.mObject->GetBaseCom<RndParticleEmitterCom>() != nullptr;
        });

        GroupMetadata& maxExtents = static_cast<GroupMetadata&>(
            *emitter.RegisterProp("max_box_extents", offsetof(EmitterParams, mMaxBoxExtents), kPropertyStruct, 12)
                 .mMetadata.Get());
        static_cast<void>(Symbol("struct"));
        maxExtents.mKind = 1;
        maxExtents.mHelp = String("Max extents of the box region which emits particles");
        RegisterVector3Members(maxExtents.mRegistry, emitterPrototype.mMaxBoxExtents);
        maxExtents.mReadOnly._Set([](const TagArgs& args) {
            return args.mComponent.mObject->GetBaseCom<RndParticleEmitterCom>() != nullptr;
        });

        FloatWaveformMetadata& pitch = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
            emitter.RegisterProp("pitch", offsetof(EmitterParams, mPitch), kPropertyWaveformFloat, 40),
            emitterPrototype.mPitch));
        pitch.mDefault = Waveform<float>(emitterPrototype.mPitch);
        static_cast<void>(Symbol("prop"));
        pitch.mHelp = String(
            "Pitch offset for the direction of the emitter over the duration of the system (angle from the XY "
            "plane to the Z-axis; 0 sits in the XY plane, +90 points along +Z, -90 points along -Z)");
        pitch.mMin = -90.0F;
        pitch.mMax = 90.0F;
        pitch.mSlider = true;

        FloatWaveformMetadata& yaw = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
            emitter.RegisterProp("yaw", offsetof(EmitterParams, mYaw), kPropertyWaveformFloat, 40),
            emitterPrototype.mYaw));
        yaw.mDefault = Waveform<float>(emitterPrototype.mYaw);
        static_cast<void>(Symbol("prop"));
        yaw.mHelp = String(
            "Yaw offset for the direction of the emitter over the duration of the system (angle about the "
            "Z-axis; 0 points along +Y, +90 points along -X, +180 points along -Y, +270 points along +X)");
        yaw.mMin = 0.0F;
        yaw.mMax = 360.0F;
        yaw.mSlider = true;

        BoolMetadata& birthMomentum = TypeSpecificMetadata(
            emitter.RegisterProp("birth_momentum", offsetof(EmitterParams, mBirthMomentum), kPropertyBool, 1),
            emitterPrototype.mBirthMomentum);
        birthMomentum.mDefault = emitterPrototype.mBirthMomentum;
        static_cast<void>(Symbol("prop"));
        birthMomentum.mHelp = String("Do new particles inherit the emitter's speed?");

        FloatMetadata& momentumAmount = TypeSpecificMetadata(
            emitter.RegisterProp(
                "birth_momentum_amount", offsetof(EmitterParams, mBirthMomentumAmount), kPropertyFloat, 4),
            emitterPrototype.mBirthMomentumAmount);
        momentumAmount.mDefault = emitterPrototype.mBirthMomentumAmount;
        static_cast<void>(Symbol("prop"));
        momentumAmount.mHelp = String("Multiplier on the amount of the emitter's speed that is inherited");

        IntMetadata<int>& subsamples = TypeSpecificMetadata(
            emitter.RegisterProp("subsamples", offsetof(EmitterParams, mSubsamples), kPropertyInt, 4),
            emitterPrototype.mSubsamples);
        subsamples.mDefault = emitterPrototype.mSubsamples;
        static_cast<void>(Symbol("prop"));
        subsamples.mHelp =
            String("Number of subsamples to alleviate clumps of emitted particles with fast moving emitters");

        FloatMetadata& duration = TypeSpecificMetadata(
            emitter.RegisterProp("duration", offsetof(EmitterParams, mDuration), kPropertyFloat, 4),
            emitterPrototype.mDuration);
        duration.mDefault = emitterPrototype.mDuration;
        static_cast<void>(Symbol("prop"));
        duration.mHelp = String("Duration of the effect. This is used for over life of the emitter waveforms.");

        BoolMetadata& localSpace = TypeSpecificMetadata(
            emitter.RegisterProp("local_space", offsetof(EmitterParams, mLocalSpace), kPropertyBool, 1),
            emitterPrototype.mLocalSpace);
        localSpace.mDefault = emitterPrototype.mLocalSpace;
        static_cast<void>(Symbol("prop"));
        localSpace.mHelp = String("Is the particle system in local space?");
    }

    {
        PropRegistry& particle = static_cast<GroupMetadata&>(
            *registry.RegisterProp("particle", offsetof(RndParticleCom, mParticle), kPropertyStruct, sizeof(ParticleParams))
                 .mMetadata.Get()).mRegistry;
        static_cast<void>(Symbol("struct"));
        ParticleParams particlePrototype;

        IntMetadata<int>& particleType = static_cast<IntMetadata<int>&>(*particle.RegisterProp(
            "particle_type", offsetof(ParticleParams, mParticleType), kPropertyInt, 4).mMetadata.Get());
        particleType.mDefault = particlePrototype.mParticleType;
        static_cast<void>(Symbol("prop"));
        particleType.mAllowedValues = eastl::vector<AllowedValue<int>>{
            AllowedValue<int>{kSprite, String("Sprite"), String("Emits sprite particles")},
            AllowedValue<int>{kMesh, String("Mesh"), String("Emits mesh particles")}};
        particleType.mOnChanged = OnParticleTypeChanged;
        particleType.mAnimatable = false;

        ResourcePathMetadata& meshPath = TypeSpecificMetadata(
            particle.RegisterProp(
                "mesh_resource_path", offsetof(ParticleParams, mMeshResourcePath), kPropertyResourcePath, 8),
            particlePrototype.mMeshResourcePath);
        meshPath.mDefault = particlePrototype.mMeshResourcePath;
        static_cast<void>(Symbol("prop"));
        meshPath.mResourceId = RndMeshResource::Id();
        meshPath.mNoTest = true;
        meshPath.mHelp = String("The mesh resource that this particle system should emit instances of");
        meshPath.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType != kMesh;
        });
        meshPath.mAnimatable = false;

        IntMetadata<int>& maxCount = TypeSpecificMetadata(
            particle.RegisterProp("max_count", offsetof(ParticleParams, mMaxCount), kPropertyInt, 4),
            particlePrototype.mMaxCount);
        maxCount.mDefault = particlePrototype.mMaxCount;
        static_cast<void>(Symbol("prop"));
        maxCount.mHelp = String("The maximum number of particles which can be alive");
        maxCount.mOnChanged = OnMaxCountChanged;

        WaveformMetadata<float>& emitRate = TypeSpecificMetadata(
            particle.RegisterProp("emit_rate", offsetof(ParticleParams, mEmitRate), kPropertyWaveformFloat, 40),
            particlePrototype.mEmitRate);
        emitRate.mDefault = Waveform<float>(particlePrototype.mEmitRate);
        static_cast<void>(Symbol("prop"));
        emitRate.mHelp = String("Particle emission rate over the duration of the system");

        FloatMetadata& emitRateMultiplier = TypeSpecificMetadata(
            particle.RegisterProp(
                "emit_rate_multiplier", offsetof(ParticleParams, mEmitRateMultiplier), kPropertyFloat, 4),
            particlePrototype.mEmitRateMultiplier);
        emitRateMultiplier.mDefault = particlePrototype.mEmitRateMultiplier;
        static_cast<void>(Symbol("prop"));
        emitRateMultiplier.mHelp = String("Multiplier on particle emission rate");

        FloatMetadata& emitDistance = TypeSpecificMetadata(
            particle.RegisterProp(
                "emit_distance_per_particle",
                offsetof(ParticleParams, mEmitDistancePerParticle),
                kPropertyFloat,
                4),
            particlePrototype.mEmitDistancePerParticle);
        emitDistance.mDefault = particlePrototype.mEmitDistancePerParticle;
        static_cast<void>(Symbol("prop"));
        emitDistance.mHelp = String(
            "Distance the system must travel before a particle is emitted (0 disables distance based emission)");

        WaveformMetadata<float>& lifespan = TypeSpecificMetadata(
            particle.RegisterProp("lifespan", offsetof(ParticleParams, mLifespan), kPropertyWaveformFloat, 40),
            particlePrototype.mLifespan);
        lifespan.mDefault = Waveform<float>(particlePrototype.mLifespan);
        static_cast<void>(Symbol("prop"));
        lifespan.mHelp = String("Lifespan of a particle over the duration of the system");

        FloatMetadata& lifespanMultiplier = TypeSpecificMetadata(
            particle.RegisterProp(
                "lifespan_multiplier", offsetof(ParticleParams, mLifespanMultiplier), kPropertyFloat, 4),
            particlePrototype.mLifespanMultiplier);
        lifespanMultiplier.mDefault = particlePrototype.mLifespanMultiplier;
        static_cast<void>(Symbol("prop"));
        lifespanMultiplier.mHelp = String("Multiplier on particle lifespan");

        WaveformMetadata<float>& speed = TypeSpecificMetadata(
            particle.RegisterProp("speed", offsetof(ParticleParams, mSpeed), kPropertyWaveformFloat, 40),
            particlePrototype.mSpeed);
        speed.mDefault = Waveform<float>(particlePrototype.mSpeed);
        static_cast<void>(Symbol("prop"));
        speed.mHelp = String("Speed at emission time of a particle over the duration of the system");

        WaveformMetadata<float>& speedOverLife = TypeSpecificMetadata(
            particle.RegisterProp(
                "speed_multiplier_over_life",
                offsetof(ParticleParams, mSpeedMultiplierOverLife),
                kPropertyWaveformFloat,
                40),
            particlePrototype.mSpeedMultiplierOverLife);
        speedOverLife.mDefault = Waveform<float>(particlePrototype.mSpeedMultiplierOverLife);
        static_cast<void>(Symbol("prop"));
        speedOverLife.mHelp = String("Speed multiplier over the life of a particle");

        FloatMetadata& drag = TypeSpecificMetadata(
            particle.RegisterProp("drag", offsetof(ParticleParams, mDrag), kPropertyFloat, 4),
            particlePrototype.mDrag);
        drag.mDefault = particlePrototype.mDrag;
        static_cast<void>(Symbol("prop"));
        drag.mHelp = String("Amount of drag which affects a particle's speed");
        drag.mMin = 0.0F;
        drag.mMax = 1.0F;
        drag.mSlider = true;

        WaveformMetadata<float>& initialSize = TypeSpecificMetadata(
            particle.RegisterProp(
                "initial_size", offsetof(ParticleParams, mInitialSize), kPropertyWaveformFloat, 40),
            particlePrototype.mInitialSize);
        initialSize.mDefault = Waveform<float>(particlePrototype.mInitialSize);
        static_cast<void>(Symbol("prop"));
        initialSize.mHelp = String("Size of a particle at emission time over the duration of the system");

        BoolMetadata& uniformScale = TypeSpecificMetadata(
            particle.RegisterProp("uniform_scale", offsetof(ParticleParams, mUniformScale), kPropertyBool, 1),
            particlePrototype.mUniformScale);
        uniformScale.mDefault = particlePrototype.mUniformScale;
        static_cast<void>(Symbol("prop"));
        uniformScale.mHelp = String("If enabled x scale drives both the x and y scale of the particle");

        WaveformMetadata<float>& xScale = TypeSpecificMetadata(
            particle.RegisterProp(
                "x_scale_over_life", offsetof(ParticleParams, mXScaleOverLife), kPropertyWaveformFloat, 40),
            particlePrototype.mXScaleOverLife);
        xScale.mDefault = Waveform<float>(particlePrototype.mXScaleOverLife);
        static_cast<void>(Symbol("prop"));
        xScale.mHelp = String("X scale over the life of a particle");

        WaveformMetadata<float>& yScale = TypeSpecificMetadata(
            particle.RegisterProp(
                "y_scale_over_life", offsetof(ParticleParams, mYScaleOverLife), kPropertyWaveformFloat, 40),
            particlePrototype.mYScaleOverLife);
        yScale.mDefault = Waveform<float>(particlePrototype.mYScaleOverLife);
        static_cast<void>(Symbol("prop"));
        yScale.mHelp = String("Y scale over the life of a particle");
        yScale.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mUniformScale;
        });

        WaveformMetadata<float>& zScale = TypeSpecificMetadata(
            particle.RegisterProp(
                "z_scale_over_life", offsetof(ParticleParams, mZScaleOverLife), kPropertyWaveformFloat, 40),
            particlePrototype.mZScaleOverLife);
        zScale.mDefault = Waveform<float>(particlePrototype.mZScaleOverLife);
        static_cast<void>(Symbol("prop"));
        zScale.mHelp = String("Z scale over the life of a particle");
        zScale.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });
        zScale.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mUniformScale;
        });

        BoolMetadata& offsetThenScale = TypeSpecificMetadata(
            particle.RegisterProp(
                "offset_then_scale", offsetof(ParticleParams, mOffsetThenScale), kPropertyBool, 1),
            particlePrototype.mOffsetThenScale);
        offsetThenScale.mDefault = particlePrototype.mOffsetThenScale;
        static_cast<void>(Symbol("prop"));
        offsetThenScale.mHelp = String("Should offset be applied before scale?");

        WaveformMetadata<float>& xPivot = TypeSpecificMetadata(
            particle.RegisterProp(
                "x_pivot_over_life", offsetof(ParticleParams, mXPivotOverLife), kPropertyWaveformFloat, 40),
            particlePrototype.mXPivotOverLife);
        xPivot.mDefault = Waveform<float>(particlePrototype.mXPivotOverLife);
        static_cast<void>(Symbol("prop"));
        xPivot.mHelp = String("X pivot point (0.5 means the center of the sprite)");

        WaveformMetadata<float>& yPivot = TypeSpecificMetadata(
            particle.RegisterProp(
                "y_pivot_over_life", offsetof(ParticleParams, mYPivotOverLife), kPropertyWaveformFloat, 40),
            particlePrototype.mYPivotOverLife);
        yPivot.mDefault = Waveform<float>(particlePrototype.mYPivotOverLife);
        static_cast<void>(Symbol("prop"));
        yPivot.mHelp = String("Y pivot point (0.5 means the center of the sprite)");

        WaveformMetadata<float>& zPivot = TypeSpecificMetadata(
            particle.RegisterProp(
                "z_pivot_over_life", offsetof(ParticleParams, mZPivotOverLife), kPropertyWaveformFloat, 40),
            particlePrototype.mZPivotOverLife);
        zPivot.mDefault = Waveform<float>(particlePrototype.mZPivotOverLife);
        static_cast<void>(Symbol("prop"));
        zPivot.mHelp = String("Z pivot point (0.5 means the center of the sprite)");
        zPivot.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });

        GroupMetadata& force = static_cast<GroupMetadata&>(
            *particle.RegisterProp("force_direction", offsetof(ParticleParams, mForceDirection), kPropertyStruct, 12)
                 .mMetadata.Get());
        static_cast<void>(Symbol("struct"));
        force.mKind = 1;
        force.mHelp = String("Direction and magnitude of force to be applied to a particle");
        RegisterVector3Members(force.mRegistry, particlePrototype.mForceDirection);

        ActionMetadata& gravity =
            static_cast<ActionMetadata&>(*particle.RegisterAction("set_force_to_gravity").mMetadata.Get());
        static_cast<void>(Symbol("action"));
        gravity.mAction = [](const TagArgs& args) {
            RndParticleCom& com = const_cast<RndParticleCom&>(static_cast<const RndParticleCom&>(args.mComponent));
            com.mParticle.mForceDirection.x = 0.0F;
            com.mParticle.mForceDirection.y = 0.0F;
            com.mParticle.mForceDirection.z = -9.8F;
        };
    }

    {
        PropRegistry& color = static_cast<GroupMetadata&>(
            *registry.RegisterProp("color", offsetof(RndParticleCom, mColor), kPropertyStruct, sizeof(ColorParams))
                 .mMetadata.Get()).mRegistry;
        static_cast<void>(Symbol("struct"));
        ColorParams colorPrototype;

        WaveformMetadata<Hmx::Color>& colorWaveform = TypeSpecificMetadata(
            color.RegisterProp("color", offsetof(ColorParams, mColor), kPropertyWaveformColor, 40),
            colorPrototype.mColor);
        colorWaveform.mDefault = Waveform<Hmx::Color>(colorPrototype.mColor);
        static_cast<void>(Symbol("prop"));
        colorWaveform.mHelp._Set([](const TagArgs& args) {
            return String(
                static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh
                    ? "Color over the life of each particle (access in the shader via the ObjectExtra shader node, "
                      "using index 0)"
                    : "Color over the life of each particle (access in the shader via the VertexColor shader node)");
        });

        WaveformMetadata<float>& alpha = TypeSpecificMetadata(
            color.RegisterProp("alpha", offsetof(ColorParams, mAlpha), kPropertyWaveformFloat, 40),
            colorPrototype.mAlpha);
        alpha.mDefault = Waveform<float>(colorPrototype.mAlpha);
        static_cast<void>(Symbol("prop"));
        alpha.mHelp._Set([](const TagArgs& args) {
            return String(
                static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh
                    ? "Alpha over the life of each particle (access in the shader via the 'w' output of the "
                      "ObjectExtra shader node, using index 0)"
                    : "Alpha over the life of each particle (access in the shader via the alpha channel of the "
                      "VertexColor shader node)");
        });
    }

    {
        GroupMetadata& spinStruct = static_cast<GroupMetadata&>(
            *registry.RegisterProp("spin", offsetof(RndParticleCom, mSpin), kPropertyStruct, sizeof(SpinParams))
                 .mMetadata.Get());
        static_cast<void>(Symbol("struct"));
        PropRegistry& spin = spinStruct.mRegistry;
        SpinParams spinPrototype;

        IntMetadata<int>& initialRotation = static_cast<IntMetadata<int>&>(*spin.RegisterProp(
            "initial_rotation", offsetof(SpinParams, mInitialRotation), kPropertyInt, 4).mMetadata.Get());
        initialRotation.mDefault = spinPrototype.mInitialRotation;
        static_cast<void>(Symbol("prop"));
        initialRotation.mHelp = String("What is the particle's initial rotation?");
        initialRotation.mAllowedValues = eastl::vector<AllowedValue<int>>{
            AllowedValue<int>{kUnrotated, String("Unrotated"), String("No rotation is applied.")},
            AllowedValue<int>{
                kInheritFromEmitter,
                String("Inherit from emitter"),
                String("The particles get the rotation of the parent emitter.")},
            AllowedValue<int>{
                kRandomRotation,
                String("Random"),
                String("The particles get a random initial rotation angle, within a specifiable range.")}};

        // The orientation matrix's elements, edited through the angles below.
        FloatMetadata& mxx = TypeSpecificMetadata(
            spin.RegisterProp(
                "mxx", offsetof(SpinParams, mOrientation) + 0, kPropertyFloat, 4),
            spinPrototype.mOrientation.x.x);
        mxx.mDefault = spinPrototype.mOrientation.x.x;
        static_cast<void>(Symbol("prop"));
        mxx.mIsHidden = true;
        FloatMetadata& mxy = TypeSpecificMetadata(
            spin.RegisterProp(
                "mxy", offsetof(SpinParams, mOrientation) + 4, kPropertyFloat, 4),
            spinPrototype.mOrientation.x.y);
        mxy.mDefault = spinPrototype.mOrientation.x.y;
        static_cast<void>(Symbol("prop"));
        mxy.mIsHidden = true;
        FloatMetadata& mxz = TypeSpecificMetadata(
            spin.RegisterProp(
                "mxz", offsetof(SpinParams, mOrientation) + 8, kPropertyFloat, 4),
            spinPrototype.mOrientation.x.z);
        mxz.mDefault = spinPrototype.mOrientation.x.z;
        static_cast<void>(Symbol("prop"));
        mxz.mIsHidden = true;
        FloatMetadata& myx = TypeSpecificMetadata(
            spin.RegisterProp(
                "myx", offsetof(SpinParams, mOrientation) + 12, kPropertyFloat, 4),
            spinPrototype.mOrientation.y.x);
        myx.mDefault = spinPrototype.mOrientation.y.x;
        static_cast<void>(Symbol("prop"));
        myx.mIsHidden = true;
        FloatMetadata& myy = TypeSpecificMetadata(
            spin.RegisterProp(
                "myy", offsetof(SpinParams, mOrientation) + 16, kPropertyFloat, 4),
            spinPrototype.mOrientation.y.y);
        myy.mDefault = spinPrototype.mOrientation.y.y;
        static_cast<void>(Symbol("prop"));
        myy.mIsHidden = true;
        FloatMetadata& myz = TypeSpecificMetadata(
            spin.RegisterProp(
                "myz", offsetof(SpinParams, mOrientation) + 20, kPropertyFloat, 4),
            spinPrototype.mOrientation.y.z);
        myz.mDefault = spinPrototype.mOrientation.y.z;
        static_cast<void>(Symbol("prop"));
        myz.mIsHidden = true;
        FloatMetadata& mzx = TypeSpecificMetadata(
            spin.RegisterProp(
                "mzx", offsetof(SpinParams, mOrientation) + 24, kPropertyFloat, 4),
            spinPrototype.mOrientation.z.x);
        mzx.mDefault = spinPrototype.mOrientation.z.x;
        static_cast<void>(Symbol("prop"));
        mzx.mIsHidden = true;
        FloatMetadata& mzy = TypeSpecificMetadata(
            spin.RegisterProp(
                "mzy", offsetof(SpinParams, mOrientation) + 28, kPropertyFloat, 4),
            spinPrototype.mOrientation.z.y);
        mzy.mDefault = spinPrototype.mOrientation.z.y;
        static_cast<void>(Symbol("prop"));
        mzy.mIsHidden = true;
        FloatMetadata& mzz = TypeSpecificMetadata(
            spin.RegisterProp(
                "mzz", offsetof(SpinParams, mOrientation) + 32, kPropertyFloat, 4),
            spinPrototype.mOrientation.z.z);
        mzz.mDefault = spinPrototype.mOrientation.z.z;
        static_cast<void>(Symbol("prop"));
        mzz.mIsHidden = true;

        FloatWaveformMetadata& rotationAngle = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
            spin.RegisterProp(
                "initial_rotation_angle",
                offsetof(SpinParams, mInitialRotationAngle),
                kPropertyWaveformFloat,
                40),
            spinPrototype.mInitialRotationAngle));
        rotationAngle.mDefault = Waveform<float>(spinPrototype.mInitialRotationAngle);
        static_cast<void>(Symbol("prop"));
        rotationAngle.mHelp = String("The particle's initial counter-clockwise rotation, in degrees");
        rotationAngle.mIsHidden._Set([](const TagArgs& args) {
            const RndParticleCom& com = static_cast<const RndParticleCom&>(args.mComponent);
            if (com.mParticle.mParticleType != kMesh) {
                return com.mSpin.mInitialRotation != kRandomRotation;
            }
            return true;
        });
        rotationAngle.mAllowedTypes = {0, 1};
        rotationAngle.mAllowedBlendModes = {2};
        rotationAngle.mMin = 0.0F;
        rotationAngle.mMax = 360.0F;

        WaveformMetadata<float>& rpm = TypeSpecificMetadata(
            spin.RegisterProp("rpm_over_life", offsetof(SpinParams, mRpmOverLife), kPropertyWaveformFloat, 40),
            spinPrototype.mRpmOverLife);
        rpm.mDefault = Waveform<float>(spinPrototype.mRpmOverLife);
        static_cast<void>(Symbol("prop"));
        rpm.mHelp = String("Counter-clockwise RPM over the life of a particle");
        rpm.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
        });

        FloatMetadata& rpmDrag = TypeSpecificMetadata(
            spin.RegisterProp("rpm_drag", offsetof(SpinParams, mRpmDrag), kPropertyFloat, 4),
            spinPrototype.mRpmDrag);
        rpmDrag.mDefault = spinPrototype.mRpmDrag;
        static_cast<void>(Symbol("prop"));
        rpmDrag.mHelp = String("Multiplier on the rpm");
        rpmDrag.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
        });

        FloatWaveformMetadata& xRotationAngle = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
            spin.RegisterProp(
                "initial_x_rotation_angle",
                offsetof(SpinParams, mInitialXRotationAngle),
                kPropertyWaveformFloat,
                40),
            spinPrototype.mInitialXRotationAngle));
        xRotationAngle.mDefault = Waveform<float>(spinPrototype.mInitialXRotationAngle);
        static_cast<void>(Symbol("prop"));
        xRotationAngle.mHelp =
            String("The particle's initial counter-clockwise rotation about the X-axis, in degrees");
        xRotationAngle.mIsHidden._Set([](const TagArgs& args) {
            const RndParticleCom& com = static_cast<const RndParticleCom&>(args.mComponent);
            return com.mParticle.mParticleType == kSprite || com.mSpin.mInitialRotation != kRandomRotation;
        });
        xRotationAngle.mAllowedTypes = {0, 1};
        xRotationAngle.mAllowedBlendModes = {2};
        xRotationAngle.mMin = 0.0F;
        xRotationAngle.mMax = 360.0F;

        FloatWaveformMetadata& yRotationAngle = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
            spin.RegisterProp(
                "initial_y_rotation_angle",
                offsetof(SpinParams, mInitialYRotationAngle),
                kPropertyWaveformFloat,
                40),
            spinPrototype.mInitialYRotationAngle));
        yRotationAngle.mDefault = Waveform<float>(spinPrototype.mInitialYRotationAngle);
        static_cast<void>(Symbol("prop"));
        yRotationAngle.mHelp =
            String("The particle's initial counter-clockwise rotation about the Y-axis, in degrees");
        yRotationAngle.mIsHidden._Set([](const TagArgs& args) {
            const RndParticleCom& com = static_cast<const RndParticleCom&>(args.mComponent);
            return com.mParticle.mParticleType == kSprite || com.mSpin.mInitialRotation != kRandomRotation;
        });
        yRotationAngle.mAllowedTypes = {0, 1};
        yRotationAngle.mAllowedBlendModes = {2};
        yRotationAngle.mMin = 0.0F;
        yRotationAngle.mMax = 360.0F;

        FloatWaveformMetadata& zRotationAngle = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
            spin.RegisterProp("initial_z_rotation_angle", -1, kPropertyWaveformFloat, 0), Waveform<float>()));
        static_cast<void>(Symbol("prop"));
        zRotationAngle.mHelp =
            String("The particle's initial counter-clockwise rotation about the Z-axis, in degrees");
        zRotationAngle.mGetter = GetInitialZRotationAngle;
        zRotationAngle.mSetter = SetInitialZRotationAngle;
        zRotationAngle.mIsHidden._Set([](const TagArgs& args) {
            const RndParticleCom& com = static_cast<const RndParticleCom&>(args.mComponent);
            return com.mParticle.mParticleType == kSprite || com.mSpin.mInitialRotation != kRandomRotation;
        });
        zRotationAngle.mAllowedTypes = {0, 1};
        zRotationAngle.mAllowedBlendModes = {2};
        zRotationAngle.mMin = 0.0F;
        zRotationAngle.mMax = 360.0F;

        const float xOrientationType = 0.0F;
        FloatMetadata& xOrientation =
            TypeSpecificMetadata(spin.RegisterProp("x_orientation", -1, kPropertyFloat, 0), xOrientationType);
        static_cast<void>(Symbol("prop"));
        xOrientation.mHelp = String("Mesh orientation angle about the X-axis (applied pre-scale)");
        xOrientation.mGetter = GetXOrientation;
        xOrientation.mSetter = SetXOrientation;
        xOrientation.mDefault = 0.0F;
        xOrientation.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });

        const float yOrientationType = 0.0F;
        FloatMetadata& yOrientation =
            TypeSpecificMetadata(spin.RegisterProp("y_orientation", -1, kPropertyFloat, 0), yOrientationType);
        static_cast<void>(Symbol("prop"));
        yOrientation.mHelp = String("Mesh orientation angle about the Y-axis (applied pre-scale)");
        yOrientation.mGetter = GetYOrientation;
        yOrientation.mSetter = SetYOrientation;
        yOrientation.mDefault = 0.0F;
        yOrientation.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });

        const float zOrientationType = 0.0F;
        FloatMetadata& zOrientation =
            TypeSpecificMetadata(spin.RegisterProp("z_orientation", -1, kPropertyFloat, 0), zOrientationType);
        static_cast<void>(Symbol("prop"));
        zOrientation.mHelp = String("Mesh orientation angle about the Z-axis (applied pre-scale)");
        zOrientation.mGetter = GetZOrientation;
        zOrientation.mSetter = SetZOrientation;
        zOrientation.mDefault = 0.0F;
        zOrientation.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });

        WaveformMetadata<float>& xRpm = TypeSpecificMetadata(
            spin.RegisterProp("x_rpm_over_life", offsetof(SpinParams, mXRpmOverLife), kPropertyWaveformFloat, 40),
            spinPrototype.mXRpmOverLife);
        xRpm.mDefault = Waveform<float>(spinPrototype.mXRpmOverLife);
        static_cast<void>(Symbol("prop"));
        xRpm.mHelp = String("Counter-clockwise RPM about the X-axis over the life of a particle");
        xRpm.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });
        xRpm.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mRuntime.mVelocityAligned;
        });

        WaveformMetadata<float>& yRpm = TypeSpecificMetadata(
            spin.RegisterProp("y_rpm_over_life", offsetof(SpinParams, mYRpmOverLife), kPropertyWaveformFloat, 40),
            spinPrototype.mYRpmOverLife);
        yRpm.mDefault = Waveform<float>(spinPrototype.mYRpmOverLife);
        static_cast<void>(Symbol("prop"));
        yRpm.mHelp = String("Counter-clockwise RPM about the Y-axis over the life of a particle");
        yRpm.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });

        WaveformMetadata<float>& zRpm = TypeSpecificMetadata(
            spin.RegisterProp("z_rpm_over_life", -1, kPropertyWaveformFloat, 0), Waveform<float>());
        static_cast<void>(Symbol("prop"));
        zRpm.mGetter = GetZRpmOverLife;
        zRpm.mSetter = SetZRpmOverLife;
        zRpm.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });
        zRpm.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mRuntime.mVelocityAligned;
        });

        FloatMetadata& xRpmDrag = TypeSpecificMetadata(
            spin.RegisterProp("x_rpm_drag", offsetof(SpinParams, mXRpmDrag), kPropertyFloat, 4),
            spinPrototype.mXRpmDrag);
        xRpmDrag.mDefault = spinPrototype.mXRpmDrag;
        static_cast<void>(Symbol("prop"));
        xRpmDrag.mHelp = String("Multiplier on the X rpm");
        xRpmDrag.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });
        xRpmDrag.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mRuntime.mVelocityAligned;
        });

        FloatMetadata& yRpmDrag = TypeSpecificMetadata(
            spin.RegisterProp("y_rpm_drag", offsetof(SpinParams, mYRpmDrag), kPropertyFloat, 4),
            spinPrototype.mYRpmDrag);
        yRpmDrag.mDefault = spinPrototype.mYRpmDrag;
        static_cast<void>(Symbol("prop"));
        yRpmDrag.mHelp = String("Multiplier on the Y rpm");
        yRpmDrag.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });

        const float zRpmDragType = 0.0F;
        FloatMetadata& zRpmDrag =
            TypeSpecificMetadata(spin.RegisterProp("z_rpm_drag", -1, kPropertyFloat, 0), zRpmDragType);
        static_cast<void>(Symbol("prop"));
        zRpmDrag.mHelp = String("Multiplier on the Z rpm");
        zRpmDrag.mGetter = GetZRpmDrag;
        zRpmDrag.mSetter = SetZRpmDrag;
        zRpmDrag.mIsHidden._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kSprite;
        });
        zRpmDrag.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mRuntime.mVelocityAligned;
        });

        spinStruct.mReadOnly._Set([](const TagArgs& args) {
            const RndParticleCom& com = static_cast<const RndParticleCom&>(args.mComponent);
            return com.mParticle.mParticleType == kSprite && com.mRuntime.mVelocityAligned;
        });
    }

    {
        PropRegistry& collision = static_cast<GroupMetadata&>(
            *registry.RegisterProp(
                "collision", offsetof(RndParticleCom, mCollision), kPropertyStruct, sizeof(CollisionParams))
                 .mMetadata.Get()).mRegistry;
        static_cast<void>(Symbol("struct"));
        CollisionParams collisionPrototype;

        IntMetadata<int>& shape = static_cast<IntMetadata<int>&>(*collision.RegisterProp(
            "per_particle_collision_shape",
            offsetof(CollisionParams, mPerParticleCollisionShape),
            kPropertyInt,
            4).mMetadata.Get());
        shape.mDefault = collisionPrototype.mPerParticleCollisionShape;
        static_cast<void>(Symbol("prop"));
        shape.mHelp = String("The shape used for each particle in resolving collisions with collision surfaces");
        shape.mAllowedValues = eastl::vector<AllowedValue<int>>{
            AllowedValue<int>{
                kCollidePoint,
                String("Point"),
                String("Each particle is treated as a point for the purposes of resolving collision")},
            AllowedValue<int>{
                kCollideSphere,
                String("Sphere"),
                String("Each particle is treated as a sphere for the purposes of resolving collision")}};

        FloatMetadata& elasticity = TypeSpecificMetadata(
            collision.RegisterProp("elasticity", offsetof(CollisionParams, mElasticity), kPropertyFloat, 4),
            collisionPrototype.mElasticity);
        elasticity.mDefault = collisionPrototype.mElasticity;
        static_cast<void>(Symbol("prop"));
        elasticity.mHelp = String(
            "Determines how elastic collisions with my particles (1 for a full bounce, 0 to completely deaden "
            "the bounce)");
        elasticity.mMin = 0.0F;
        elasticity.mMax = 1.0F;
        elasticity.mSlider = true;

        PropInfo& colliders = collision.RegisterPropArray(
            "colliders", offsetof(CollisionParams, mColliders), sizeof(PropArray<GameObjectId>));
        ArrayMetadata& collidersMetadata = static_cast<ArrayMetadata&>(*colliders.mMetadata.Get());
        static_cast<void>(Symbol("array"));
        collidersMetadata.mHelp = String("The set of colliders which affect particles in this particle system");
        PropRegistry::RegisterPropArrayItem(colliders, kPropertyGameObjectId, sizeof(GameObjectId));
        const GameObjectId colliderType = {0xFFFFFFFFU};
        GameObjectIdMetadata& collider = TypeSpecificMetadata(collidersMetadata.mItemInfo, colliderType);
        static_cast<void>(Symbol("item"));
        collider.mRequiredComs = {RndParticleColliderCom::sId};
        collider.mAllowsDuplicates = false;

        ActionMetadata& gatherColliders =
            static_cast<ActionMetadata&>(*collision.RegisterAction("gather_all").mMetadata.Get());
        static_cast<void>(Symbol("action"));
        gatherColliders.mHelp = String("Gathers all colliders in the same entity as this particle system");
        gatherColliders.mAction = [](const TagArgs& args) {
            const_cast<RndParticleCom&>(static_cast<const RndParticleCom&>(args.mComponent)).GatherColliders();
        };
    }

    {
        PropRegistry& affectors = static_cast<GroupMetadata&>(
            *registry.RegisterProp("affectors", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
        static_cast<void>(Symbol("struct"));
        PropInfo& affectorArray = affectors.RegisterPropArray(
            "affectors", offsetof(RndParticleCom, mAffectors), sizeof(PropArray<GameObjectId>));
        ArrayMetadata& affectorArrayMetadata = static_cast<ArrayMetadata&>(*affectorArray.mMetadata.Get());
        static_cast<void>(Symbol("array"));
        affectorArrayMetadata.mHelp = String("The set of affectors which affect particles in this particle system");
        PropRegistry::RegisterPropArrayItem(affectorArray, kPropertyGameObjectId, sizeof(GameObjectId));
        const GameObjectId affectorType = {0xFFFFFFFFU};
        GameObjectIdMetadata& affector = TypeSpecificMetadata(affectorArrayMetadata.mItemInfo, affectorType);
        static_cast<void>(Symbol("item"));
        affector.mRequiredComs = {RndParticleAffectorCom::sId};
        affector.mAllowsDuplicates = false;

        ActionMetadata& gatherAffectors =
            static_cast<ActionMetadata&>(*affectors.RegisterAction("gather_all").mMetadata.Get());
        static_cast<void>(Symbol("action"));
        gatherAffectors.mHelp = String("Gathers all affectors in the same entity as this particle system");
        gatherAffectors.mAction = [](const TagArgs& args) {
            const_cast<RndParticleCom&>(static_cast<const RndParticleCom&>(args.mComponent)).GatherAffectors();
        };
    }

    // The channel types, built in the temporary heap.
    eastl::vector<AllowedValue<int>> extraDataTypes;
    unsigned int tempHeap;
    MemPushTemp(tempHeap, true, true);
    extraDataTypes.push_back(AllowedValue<int>{
        kExtraDataDisabled, String("Disabled"), String("This extra data channel is not used")});
    extraDataTypes.push_back(AllowedValue<int>{
        kExtraDataOverLife,
        String("Over Life"),
        String("Each particle will evaluate this extra data channel over the life of the particle")});
    extraDataTypes.push_back(AllowedValue<int>{
        kExtraDataOnEmission,
        String("On Emission"),
        String(
            "Each particle will evaluate this extra data waveform once on emission (based on duration), keeping "
            "the same value for the entire life of the particle")});
    MemPopTemp(tempHeap);

    {
        GroupMetadata& extraDataGroup = static_cast<GroupMetadata&>(*registry.RegisterProp(
            "particle_extra_data", offsetof(RndParticleCom, mExtraData), kPropertyStruct, sizeof(ExtraDataParams))
                 .mMetadata.Get());
        static_cast<void>(Symbol("struct"));
        PropRegistry& extraData = extraDataGroup.mRegistry;
        ExtraDataParams extraDataPrototype;
        extraDataGroup.mHelp._Set([](const TagArgs& args) {
            return String(
                static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh
                    ? "Extra per-particle data passed to the shader (access in the shader via the ObjectExtra "
                      "shader node, using index 1)"
                    : "Extra per-particle data passed to the shader (access in the shader via the ParticleData "
                      "shader node)");
        });

        IntMetadata<int>& type0 = static_cast<IntMetadata<int>&>(*extraData.RegisterProp(
            "extra_data_type_0", offsetof(ExtraDataParams, mExtraDataType0), kPropertyInt, 4).mMetadata.Get());
        type0.mDefault = extraDataPrototype.mExtraDataType0;
        static_cast<void>(Symbol("prop"));
        type0.mAllowedValues = extraDataTypes;
        WaveformMetadata<float>& data0 = TypeSpecificMetadata(
            extraData.RegisterProp("extra_data_0", offsetof(ExtraDataParams, mExtraData0), kPropertyWaveformFloat, 40),
            extraDataPrototype.mExtraData0);
        data0.mDefault = Waveform<float>(extraDataPrototype.mExtraData0);
        static_cast<void>(Symbol("prop"));
        data0.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mExtraData.mExtraDataType0 ==
                kExtraDataDisabled;
        });

        IntMetadata<int>& type1 = static_cast<IntMetadata<int>&>(*extraData.RegisterProp(
            "extra_data_type_1", offsetof(ExtraDataParams, mExtraDataType1), kPropertyInt, 4).mMetadata.Get());
        type1.mDefault = extraDataPrototype.mExtraDataType1;
        static_cast<void>(Symbol("prop"));
        type1.mAllowedValues = extraDataTypes;
        WaveformMetadata<float>& data1 = TypeSpecificMetadata(
            extraData.RegisterProp("extra_data_1", offsetof(ExtraDataParams, mExtraData1), kPropertyWaveformFloat, 40),
            extraDataPrototype.mExtraData1);
        data1.mDefault = Waveform<float>(extraDataPrototype.mExtraData1);
        static_cast<void>(Symbol("prop"));
        data1.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mExtraData.mExtraDataType1 ==
                kExtraDataDisabled;
        });

        IntMetadata<int>& type2 = static_cast<IntMetadata<int>&>(*extraData.RegisterProp(
            "extra_data_type_2", offsetof(ExtraDataParams, mExtraDataType2), kPropertyInt, 4).mMetadata.Get());
        type2.mDefault = extraDataPrototype.mExtraDataType2;
        static_cast<void>(Symbol("prop"));
        type2.mAllowedValues = extraDataTypes;
        WaveformMetadata<float>& data2 = TypeSpecificMetadata(
            extraData.RegisterProp("extra_data_2", offsetof(ExtraDataParams, mExtraData2), kPropertyWaveformFloat, 40),
            extraDataPrototype.mExtraData2);
        data2.mDefault = Waveform<float>(extraDataPrototype.mExtraData2);
        static_cast<void>(Symbol("prop"));
        data2.mReadOnly._Set([](const TagArgs& args) {
            return static_cast<const RndParticleCom&>(args.mComponent).mExtraData.mExtraDataType2 ==
                kExtraDataDisabled;
        });
    }

    GroupMetadata& renderGroup = static_cast<GroupMetadata&>(
        *registry.RegisterProp("render", offsetof(RndParticleCom, mRender), kPropertyStruct, sizeof(RenderParams))
             .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& render = renderGroup.mRegistry;
    IntMetadata<int>& alignment = static_cast<IntMetadata<int>&>(*render.RegisterProp(
        "particle_alignment", offsetof(RenderParams, mParticleAlignment), kPropertyInt, 4).mMetadata.Get());
    alignment.mDefault = RenderParams().mParticleAlignment;
    static_cast<void>(Symbol("prop"));
    alignment.mHelp = String("Alignment of the particles in world space");
    alignment.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kCameraAligned, String("Camera Aligned"), String("Align particles to face the camera")},
        AllowedValue<int>{
            kCameraXYAligned,
            String("Camera XY-Aligned"),
            String("Align particles to face the camera in XY, aligning Z straight-up")},
        AllowedValue<int>{
            kXAxisAligned, String("X-Axis Aligned"), String("Align particles to face the worldspace +X axis")},
        AllowedValue<int>{
            kYAxisAligned, String("Y-Axis Aligned"), String("Align particles to face the worldspace -Y axis")},
        AllowedValue<int>{
            kZAxisAligned, String("Z-Axis Aligned"), String("Align particles to face the worldspace +Z axis")}};
    IntMetadata<int>& sortMethod = static_cast<IntMetadata<int>&>(*render.RegisterProp(
        "sort_method", offsetof(RenderParams, mSortMethod), kPropertyInt, 4).mMetadata.Get());
    sortMethod.mDefault = RenderParams().mSortMethod;
    static_cast<void>(Symbol("prop"));
    sortMethod.mHelp = String("What order should we render the particles in?");
    sortMethod.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            0, String("Back to Front"), String("Sort by depth, drawing the particles furthest from the camera first")},
        AllowedValue<int>{1, String("Oldest to Newest"), String("Sort by age, drawing the oldest particles first")},
        AllowedValue<int>{2, String("Newest to Oldest"), String("Sort by age, drawing the newest particles first")}};
    renderGroup.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const RndParticleCom&>(args.mComponent).mParticle.mParticleType == kMesh;
    });
}

// Reconstructed from eboot.elf at 0x60D530.
void RndParticleCom::_GetPollDeps(
    eastl::vector<GameObjectId>& before,
    eastl::vector<GameObjectId>& after) {
    static_cast<void>(after);
    const PropArray<GameObjectId>& colliders = mCollision.mColliders;
    before.reserve(before.size() + colliders.mSize);
    for (const GameObjectId& id : colliders) {
        before.push_back(id);
    }
}

// Reconstructed from eboot.elf at 0x60D6F0.
void RndParticleCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    RndDrawInstanceCom::_GetComponentOrderDeps(follows, precedes);
    follows.push_back(RndParticleColliderCom::sClassName);
}

// Reconstructed from eboot.elf at 0x60D7C0. A mesh path is looked up first
// among the resources inlined into the entity's layers, then loaded. The
// binary inlines _AllocateBuffers and _InstallRand.
bool RndParticleCom::_OnResourcesLoaded() {
    if (!RndDrawInstanceCom::_OnResourcesLoaded()) {
        return false;
    }
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    mRuntime.mClock = TheTimeMgr->GetClock(mObject->mEntity);

    RndMesh* mesh = nullptr;
    if (mParticle.mParticleType != kMesh) {
        mRuntime.mMeshResource = nullptr;
    } else if (mParticle.mMeshResourcePath == ResourcePath()) {
        mRuntime.mMeshResource = nullptr;
        mesh = sDefaultMesh;
    } else {
        const ResourcePath& path = mParticle.mMeshResourcePath;
        Resource* inlined = nullptr;
        for (const EntityResource::LayerInfo& layer : mObject->mEntity->mResource->mLayers) {
            for (Resource* resource : layer.mInlineResources) {
                if (resource->mPath == path) {
                    inlined = resource;
                    break;
                }
            }
            if (inlined != nullptr) {
                break;
            }
        }
        if (inlined != nullptr) {
            mRuntime.mMeshResource =
                ResourcePtr<RndMeshResource>(static_cast<RndMeshResource*>(inlined));
        } else {
            mRuntime.mMeshResource = Resource::GetOrLoad<RndMeshResource>(path, false);
        }
        RndMeshResource* resource = mRuntime.mMeshResource;
        if (resource != nullptr && !resource->Fail()) {
            mesh = resource->mMesh;
        }
    }
    mRuntime.mMesh = mesh;

    _AllocateBuffers(mObject->mName.Str());
    _InstallRand();
    mRuntime.mNewParticleXfms.reserve(mParticle.mMaxCount);
    mRuntime.mNewParticleData.reserve(mParticle.mMaxCount);
    return true;
}

// Reconstructed from eboot.elf at 0x60DCD0.
void RndParticleCom::_AllocateBuffers(const char* name) {
    delete mRuntime.mBuffer;
    mRuntime.mBuffer = nullptr;
    mRuntime.mParticles.SetMaxCount(mParticle.mMaxCount);
    if (mParticle.mParticleType == kSprite) {
        mRuntime.mBuffer = RndParticleBuffer::New(mParticle.mMaxCount, name);
    }
}

// Reconstructed from eboot.elf at 0x60DE00.
unsigned long RndParticleCom::_GetNumDrawInstancesImpl(RndSceneLod lod) const {
    if ((mLods >> (lod & 31) & 1) == 0) {
        return 0;
    }
    if (mParticle.mParticleType == kMesh) {
        return mRuntime.mParticles.mMaxCount;
    }
    return 1;
}

// Reconstructed from eboot.elf at 0x60DE30.
RndMaterialCom* RndParticleCom::_GetDefaultMaterial() const {
    const RndDefaults& defaults = gRndDevice->mDefaults;
    return mParticle.mParticleType == kMesh ? defaults.mLitMaterial : defaults.mParticleMaterial;
}

// Reconstructed from eboot.elf at 0x60DE60.
void RndParticleCom::_InitDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) {
    _SyncInstanceStateFlags(instances);
}

// Reconstructed from eboot.elf at 0x60F540.
bool RndParticleCom::_DrawsGeometry() const {
    return false;
}

// Reconstructed from eboot.elf at 0x60F550. The binary inlines
// _KillAllParticles.
void RndParticleCom::_Enter() {
    _KillAllParticles();
    RndDrawInstanceCom::_Enter();
}

// Reconstructed from eboot.elf at 0x60F5A0.
void RndParticleCom::_KillAllParticles() {
    while (mRuntime.mParticles.mNumParticles != 0) {
        mRuntime.mParticles.DeleteParticle(0);
        --mRuntime.mNumActive;
    }
}

// Reconstructed from eboot.elf at 0x60F5E0. A hidden system that does not
// "sim_when_hidden" empties its sphere and its particles. Otherwise the
// system advances its time and runs in the object's space ("local_space")
// or in world space, then hands the buffer what it draws and polls its
// extensions. The buffer gets last frame's velocity alignment.
void RndParticleCom::_Poll() {
    if (!mSimWhenHidden && (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) != 0) {
        mRuntime.mDrawNode->SetLocalSphere(Sphere::sZero);
        if (mRuntime.mParticles.mNumChunks * 16 != 0) {
            _DeallocAllParticles();
        }
        RndDrawInstanceCom::_Poll();
        return;
    }

    mRuntime.mTime += mRuntime.mClock->DeltaTime(mTimeUnits);
    Transform emitXfm;
    Transform simXfm;
    if (mEmitter.mLocalSpace) {
        emitXfm = Transform::sID;
        simXfm = RndDrawInstanceCom::mRuntime.mDrawNode->mRuntime.mInvWorldXfm;
        mRuntime.mParticleXfm = mRuntime.mTrans->mWorldXfm;
    } else {
        emitXfm = mRuntime.mTrans->mWorldXfm;
        simXfm = Transform::sID;
        mRuntime.mParticleXfm = Transform::sID;
    }
    _UpdateParticles(emitXfm, simXfm);
    _UpdateBoundingSphere();

    RndParticleBuffer* buffer = mRuntime.mBuffer;
    if (buffer != nullptr) {
        buffer->mParticles = &mRuntime.mParticles;
        buffer->mXfm = &mRuntime.mParticleXfm;
        buffer->mAlignment = mRender.mParticleAlignment;
        buffer->mSortMode = mRender.mSortMethod;
        buffer->mVelocityAligned = mRuntime.mVelocityAligned;
        buffer->mWorldSpace = mParticle.mOffsetThenScale;
        buffer->mHasRotation = mExtraData.mExtraDataType0 != kExtraDataDisabled ||
                               mExtraData.mExtraDataType1 != kExtraDataDisabled ||
                               mExtraData.mExtraDataType2 != kExtraDataDisabled;
    }
    mRuntime.mVelocityAligned = mObject->GetCom<RndParticleVelocityAlignCom>() != nullptr;
    mRuntime.mLastPosition = emitXfm.v;
    for (RndParticleExtCom* extension : mRuntime.mExtensions) {
        extension->_PollExtension();
    }
    RndDrawInstanceCom::_Poll();
}

// Reconstructed from eboot.elf at 0x60FA20.
void RndParticleCom::_DeallocAllParticles() {
    _KillAllParticles();
    mRuntime.mParticles.DeleteAllParticles();
}

// Reconstructed from eboot.elf at 0x60FA70. Nothing happens on a frame
// without time. Particles die past their death time, or when the clock went
// back before their birth, unless "never_kill"; the binary inlines
// _UpdateDeath and _UpdateAffectors without their chunk frees. The life
// fraction is computed four particles at a time with a refined reciprocal.
// Sprites aligned to their velocity keep their rotation.
void RndParticleCom::_UpdateParticles(const Transform& emitXfm, const Transform& simXfm) {
    const float dt = mRuntime.mClock->DeltaTime(mTimeUnits);
    if (!(std::fabs(dt) > 1.0e-4F)) {
        return;
    }
    RndParticleCollection& particles = mRuntime.mParticles;
    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        while (!mEmitter.mNeverKill) {
            const float time = mRuntime.mTime;
            if (!(time >= particles.mDeathTimes[i]) &&
                time >= particles.mBirthTimes[i]) {
                break;
            }
            particles.DeleteParticle(i);
            --mRuntime.mNumActive;
            if (i >= particles.mNumParticles) {
                break;
            }
        }
    }
    particles.FreeUnusedChunks();

    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        const float birth = particles.mBirthTimes[i];
        const float death = particles.mDeathTimes[i];
        float fraction = (mRuntime.mTime - birth) / (death - birth);
        fraction = fraction < 1.0F ? fraction : 1.0F;
        fraction = fraction > 0.0F ? fraction : 0.0F;
        particles.mLifeFractions[i] = fraction;
    }

    _UpdateForce(simXfm);
    _UpdatePositions(simXfm);
    const Entity* entity = mObject->mEntity;
    for (const GameObjectId& id : mAffectors) {
        RndParticleAffectorCom* affector = FindBaseCom<RndParticleAffectorCom>(entity, id);
        if (affector != nullptr && affector->mEnabled) {
            affector->_PostAffectParticles(*this, simXfm);
        }
    }
    particles.FreeUnusedChunks();

    if (!mRuntime.mVelocityAligned || mParticle.mParticleType == kMesh) {
        _UpdateSpin();
    }
    _UpdatePivot();
    _UpdateColorSize();
    _UpdateExtraData();
    _SpawnParticles(emitXfm);
}

// Reconstructed from eboot.elf at 0x60FDF0. The sphere encloses each live
// particle's box of its largest size; without particles it is a point at
// the particle transform's position. A world-space system's sphere moves
// into the object's space.
void RndParticleCom::_UpdateBoundingSphere() {
    Sphere sphere;
    const int numActive = mRuntime.mNumActive;
    if (numActive <= 0) {
        sphere.center = mRuntime.mParticleXfm.v;
        sphere.radius = 0.0F;
    } else {
        const RndParticleCollection& particles = mRuntime.mParticles;
        Vector3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        for (unsigned long i = 0; i < static_cast<unsigned long>(numActive); ++i) {
            float extent = particles.mSizeY[i];
            extent = std::fmax(extent, particles.mSizeX[i]);
            extent = std::fmax(extent, particles.mSizeZ[i]);
            const float x = particles.mPosX[i];
            const float y = particles.mPosY[i];
            const float z = particles.mPosZ[i];
            min.x = std::fmin(x - extent, min.x);
            min.y = std::fmin(y - extent, min.y);
            min.z = std::fmin(z - extent, min.z);
            max.x = std::fmax(extent + x, max.x);
            max.y = std::fmax(extent + y, max.y);
            max.z = std::fmax(extent + z, max.z);
        }
        sphere.SetFromBox(min, max);
    }
    if (!mEmitter.mLocalSpace) {
        mRuntime.mDrawNode->SetLocalSphereFromWorld(sphere);
    } else {
        mRuntime.mDrawNode->SetLocalSphere(sphere);
    }
}

// Reconstructed from eboot.elf at 0x610070.
void RndParticleCom::_UpdateDeath() {
    RndParticleCollection& particles = mRuntime.mParticles;
    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        while (!mEmitter.mNeverKill) {
            const float time = mRuntime.mTime;
            if (!(time >= particles.mDeathTimes[i]) &&
                time >= particles.mBirthTimes[i]) {
                break;
            }
            particles.DeleteParticle(i);
            --mRuntime.mNumActive;
            if (i >= particles.mNumParticles) {
                break;
            }
        }
    }
    particles.FreeUnusedChunks();
}

// Reconstructed from eboot.elf at 0x610230. The binary adds the force to
// four particles at a time. Affectors whose strength is within 1e-4 of zero
// are skipped.
void RndParticleCom::_UpdateForce(const Transform& simXfm) {
    const float dt = mRuntime.mClock->DeltaTime(mTimeUnits);
    RndParticleCollection& particles = mRuntime.mParticles;
    const Vector3& force = mParticle.mForceDirection;
    const float dx = dt * force.x;
    const float dy = dt * force.y;
    const float dz = dt * force.z;
    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        particles.mVelX[i] += dx;
        particles.mVelY[i] += dy;
        particles.mVelZ[i] += dz;
    }
    const Entity* entity = mObject->mEntity;
    for (const GameObjectId& id : mAffectors) {
        RndParticleAffectorCom* affector = FindBaseCom<RndParticleAffectorCom>(entity, id);
        if (affector != nullptr && affector->mEnabled &&
            std::fabs(affector->mForceStrength) > 1.0e-4F) {
            affector->_AffectParticles(particles, simXfm, dt);
        }
    }
}

// Reconstructed from eboot.elf at 0x6108A0.
void RndParticleCom::_UpdateAffectors(const Transform& simXfm) {
    const Entity* entity = mObject->mEntity;
    for (const GameObjectId& id : mAffectors) {
        RndParticleAffectorCom* affector = FindBaseCom<RndParticleAffectorCom>(entity, id);
        if (affector != nullptr && affector->mEnabled) {
            affector->_PostAffectParticles(*this, simXfm);
        }
    }
    mRuntime.mParticles.FreeUnusedChunks();
}

// Reconstructed from eboot.elf at 0x611F50. When the oldest may die, the
// oldest live particles make room for the whole queue. A queue the
// collection cannot hold is cut where creation failed.
void RndParticleCom::_SpawnParticles(const Transform& emitXfm) {
    _UpdateDistanceEmission(emitXfm);
    _UpdateRateEmission(emitXfm);
    RndParticleCollection& particles = mRuntime.mParticles;
    eastl::vector<Transform>& births = mRuntime.mNewParticleXfms;
    if (mEmitter.mEmitCanKillOldest) {
        int excess = static_cast<int>(births.size()) + mRuntime.mNumActive - mParticle.mMaxCount;
        for (; excess > 0; --excess) {
            particles.DeleteParticle(particles.mFirstParticle);
            --mRuntime.mNumActive;
        }
    }
    for (Transform* birth = births.begin(); birth != births.end(); ++birth) {
        if (particles.CreateParticle() == static_cast<unsigned long>(-1)) {
            births.erase(birth, births.end());
            break;
        }
        ++mRuntime.mNumActive;
    }
    _InitParticles();
}

// Reconstructed from eboot.elf at 0x612080. Births are spaced
// "emit_distance_per_particle" apart along the straight path from the last
// position; a jump of more than 128 spacings emits along it in 128 steps,
// and an absurd one (a step over 1e6) is dropped. The binary measures the
// path with a refined reciprocal square root.
void RndParticleCom::_UpdateDistanceEmission(const Transform& emitXfm) {
    const float spacing = mParticle.mEmitDistancePerParticle;
    if (!(spacing > 0.0F)) {
        return;
    }
    const float rateMultiplier = mParticle.mEmitRateMultiplier;
    if (!(std::fabs(rateMultiplier) > 1.0e-4F)) {
        return;
    }
    const Vector3& last = mRuntime.mLastPosition;
    const Vector3 delta = {emitXfm.v.x - last.x, emitXfm.v.y - last.y, emitXfm.v.z - last.z};
    const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (!std::isfinite(distance)) {
        return;
    }
    const float previous = mRuntime.mDistanceTravelled;
    float travelled = previous + distance;
    mRuntime.mDistanceTravelled = travelled;
    float step;
    if ((travelled - spacing) / spacing > 128.0F) {
        step = distance * (1.0F / 128.0F);
        if (step > 1.0e6F) {
            mRuntime.mDistanceTravelled = 0.0F;
            return;
        }
    } else {
        step = spacing / rateMultiplier;
    }

    Transform birth = emitXfm;
    float target = spacing;
    if (!(target > travelled)) {
        do {
            const float span = travelled - previous;
            if (span == 0.0F) {
                break;
            }
            const float t = (target - previous) / span;
            birth.v.x = delta.x * t + last.x;
            birth.v.y = delta.y * t + last.y;
            birth.v.z = delta.z * t + last.z;
            if (!_QueueNewParticle(birth)) {
                break;
            }
            target += step;
            travelled -= step;
        } while (target <= mRuntime.mDistanceTravelled);
    }
    mRuntime.mDistanceTravelled = travelled;
}

// Reconstructed from eboot.elf at 0x612890.
void RndParticleCom::_KillParticle(unsigned long particle) {
    mRuntime.mParticles.DeleteParticle(particle);
    --mRuntime.mNumActive;
}

// Reconstructed from eboot.elf at 0x617630. A full queue that may kill the
// oldest drops its first birth.
bool RndParticleCom::_QueueNewParticle(const Transform& xfm) {
    eastl::vector<Transform>& births = mRuntime.mNewParticleXfms;
    const long maxCount = mParticle.mMaxCount;
    if (mEmitter.mEmitCanKillOldest) {
        if (static_cast<long>(births.size()) == maxCount) {
            births.erase(births.begin());
        }
    } else if (births.size() >= static_cast<unsigned long>(maxCount - mRuntime.mNumActive)) {
        return false;
    }
    births.push_back(xfm);
    return true;
}

// Reconstructed from eboot.elf at 0x617BC0. The queue grows once for the
// whole count.
unsigned long RndParticleCom::_QueueNewParticles(unsigned long count, const Transform& xfm) {
    int room = mParticle.mMaxCount;
    if (!mEmitter.mEmitCanKillOldest) {
        room -= mRuntime.mNumActive;
    }
    if (static_cast<unsigned long>(static_cast<long>(room)) < count) {
        count = room;
    }
    eastl::vector<Transform>& births = mRuntime.mNewParticleXfms;
    births.reserve(count + births.size());
    unsigned long queued = 0;
    for (; queued < count; ++queued) {
        if (!_QueueNewParticle(xfm)) {
            break;
        }
    }
    return queued;
}

// Reconstructed from eboot.elf at 0x617D20. Each queued birth moves by a
// random point of the box, in the birth transform's axes.
void RndParticleCom::_PlaceNewParticles() {
    RndParticleEmitterCom* emitter = mObject->GetBaseCom<RndParticleEmitterCom>();
    if (emitter != nullptr) {
        emitter->_PlaceNewParticles(mRuntime.mRand, mRuntime.mNewParticleXfms);
        return;
    }
    Rand& rand = *mRuntime.mRand;
    const Vector3& min = mEmitter.mMinBoxExtents;
    const Vector3& max = mEmitter.mMaxBoxExtents;
    for (Transform& birth : mRuntime.mNewParticleXfms) {
        const float x = rand.Float() * (max.x - min.x) + min.x;
        const float y = rand.Float() * (max.y - min.y) + min.y;
        const float z = rand.Float() * (max.z - min.z) + min.z;
        const Hmx::Matrix3& m = birth.m;
        birth.v.x = x * m.x.x + y * m.y.x + birth.v.x + z * m.z.x;
        birth.v.y = x * m.x.y + y * m.y.y + birth.v.y + z * m.z.y;
        birth.v.z = x * m.x.z + y * m.y.z + birth.v.z + z * m.z.z;
    }
}

// Reconstructed from eboot.elf at 0x617F30.
void RndParticleCom::GatherColliders() {
    PropArray<GameObjectId>& colliders = mCollision.mColliders;
    colliders.Resize(0);
    const Entity* entity = mObject->mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object->GetCom<RndParticleColliderCom>() != nullptr) {
            const GameObjectId id = object->mId;
            colliders._Insert(colliders.mSize, &id);
        }
    }
}

// Reconstructed from eboot.elf at 0x618040.
void RndParticleCom::GatherAffectors() {
    mAffectors.Resize(0);
    const Entity* entity = mObject->mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object->GetBaseCom<RndParticleAffectorCom>() != nullptr) {
            const GameObjectId id = object->mId;
            mAffectors._Insert(mAffectors.mSize, &id);
        }
    }
}

// Reconstructed from eboot.elf at 0x618140.
Symbol RndParticleCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x618150.
Symbol RndParticleCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x618160.
int RndParticleCom::CurrentRev() const {
    return const_cast<RndParticleCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x618180.
bool RndParticleCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6181B0.
Component* RndParticleCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6181C0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndParticleCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndParticleCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndParticleCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6182D0.
PropRegistry& RndParticleCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6182E0.
ComMetaData& RndParticleCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x6182F0. The waveforms release their
// resources.
RndParticleCom::SpinParams::~SpinParams() {}

// Reconstructed from eboot.elf at 0x6183C0.
RndParticleCom::ParticleParams::~ParticleParams() {}

namespace {

// Reconstructed from eboot.elf at 0x618540. A sprite turns about z and
// scales in x and y, a mesh turns about each axis and scales in z too:
// the type change trades the sprite's single rotation and z scale with the
// mesh's y ones (through the waveform swap at 0x618720), and the x and y
// pivots go back to the new type's default.
void OnParticleTypeChanged(const PropAccessorArgs& args) {
    RndParticleCom& com = *static_cast<RndParticleCom*>(args.mComponent);
    com.mRuntime.mParticleTypeChanged = true;
    std::swap(com.mParticle.mYScaleOverLife, com.mParticle.mZScaleOverLife);
    std::swap(com.mSpin.mInitialYRotationAngle, com.mSpin.mInitialRotationAngle);
    std::swap(com.mSpin.mYRpmOverLife, com.mSpin.mRpmOverLife);
    std::swap(com.mSpin.mYRpmDrag, com.mSpin.mRpmDrag);
    if (*static_cast<const int*>(args.mRequestedValue) == RndParticleCom::kMesh) {
        com.mParticle.mXPivotOverLife.SetResource(RndParticleCom::sDefaultZPivot);
        com.mParticle.mYPivotOverLife.SetResource(RndParticleCom::sDefaultZPivot);
    } else {
        com.mParticle.mXPivotOverLife.SetResource(RndParticleCom::sDefaultPivot);
        com.mParticle.mYPivotOverLife.SetResource(RndParticleCom::sDefaultPivot);
    }
}

// Reconstructed from eboot.elf at 0x618890. At least one particle; the
// buffers are rebuilt for the count.
void OnMaxCountChanged(const PropAccessorArgs& args) {
    RndParticleCom& com = *static_cast<RndParticleCom*>(args.mComponent);
    com.mParticle.mMaxCount = com.mParticle.mMaxCount > 0 ? com.mParticle.mMaxCount : 1;
    com._AllocateBuffers(nullptr);
}

// Reconstructed from eboot.elf at 0x618910. A mesh's z rotation is the
// sprite's rotation.
Waveform<float> GetInitialZRotationAngle(const PropAccessorArgs& args) {
    return static_cast<RndParticleCom*>(args.mComponent)->mSpin.mInitialRotationAngle;
}

// Reconstructed from eboot.elf at 0x6189F0.
void SetInitialZRotationAngle(const PropAccessorArgs& args) {
    static_cast<RndParticleCom*>(args.mComponent)->mSpin.mInitialRotationAngle =
        *static_cast<const Waveform<float>*>(args.mValue);
}

// The angle in degrees as radians in [-pi, pi). Inlined into the orientation
// setters. Name not in the reference map.
float WrapOrientationAngle(float degrees) {
    const float angle = fmodf(degrees * kDegreesToRadians + kPi, kTwoPi);
    return angle + (angle < 0.0F ? kPi : -kPi);
}

// Whether two angles differ by more than a turn's rounding. Inlined into the
// orientation setters. Name not in the reference map.
bool OrientationAngleChanged(float from, float to) {
    const float difference = from - to;
    return fabsf(difference) > 1.0e-4F && fabsf(fabsf(difference) - kTwoPi) > 1.0e-4F;
}

// Rebuilds the orientation from the angles, keeping each row's length.
// Inlined into the orientation setters. Name not in the reference map.
void SetOrientation(Hmx::Matrix3& orientation, const Vector3& euler, const Vector3& scale) {
    MakeRotMatrix(euler, orientation, false);
    orientation.x.x *= scale.x;
    orientation.x.y *= scale.x;
    orientation.x.z *= scale.x;
    orientation.y.x *= scale.y;
    orientation.y.y *= scale.y;
    orientation.y.z *= scale.y;
    orientation.z.x *= scale.z;
    orientation.z.y *= scale.z;
    orientation.z.z *= scale.z;
}

// Reconstructed from eboot.elf at 0x618A70.
float GetXOrientation(const PropAccessorArgs& args) {
    Vector3 euler = {0.0F, 0.0F, 0.0F};
    Vector3 scale = {0.0F, 0.0F, 0.0F};
    MakeEulerScale(static_cast<RndParticleCom*>(args.mComponent)->mSpin.mOrientation, euler, scale);
    return euler.x * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x618AE0. The matrix is rebuilt only
// when the angle changes.
void SetXOrientation(const PropAccessorArgs& args) {
    Hmx::Matrix3& orientation = static_cast<RndParticleCom*>(args.mComponent)->mSpin.mOrientation;
    Vector3 euler = {0.0F, 0.0F, 0.0F};
    Vector3 scale = {0.0F, 0.0F, 0.0F};
    MakeEulerScale(orientation, euler, scale);
    const float angle = WrapOrientationAngle(*static_cast<const float*>(args.mValue));
    if (OrientationAngleChanged(euler.x, angle)) {
        euler.x = angle;
        SetOrientation(orientation, euler, scale);
    }
}

// Reconstructed from eboot.elf at 0x618C30.
float GetYOrientation(const PropAccessorArgs& args) {
    Vector3 euler = {0.0F, 0.0F, 0.0F};
    Vector3 scale = {0.0F, 0.0F, 0.0F};
    MakeEulerScale(static_cast<RndParticleCom*>(args.mComponent)->mSpin.mOrientation, euler, scale);
    return euler.y * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x618CA0.
void SetYOrientation(const PropAccessorArgs& args) {
    Hmx::Matrix3& orientation = static_cast<RndParticleCom*>(args.mComponent)->mSpin.mOrientation;
    Vector3 euler = {0.0F, 0.0F, 0.0F};
    Vector3 scale = {0.0F, 0.0F, 0.0F};
    MakeEulerScale(orientation, euler, scale);
    const float angle = WrapOrientationAngle(*static_cast<const float*>(args.mValue));
    if (OrientationAngleChanged(euler.y, angle)) {
        euler.y = angle;
        SetOrientation(orientation, euler, scale);
    }
}

// Reconstructed from eboot.elf at 0x618DF0.
float GetZOrientation(const PropAccessorArgs& args) {
    Vector3 euler = {0.0F, 0.0F, 0.0F};
    Vector3 scale = {0.0F, 0.0F, 0.0F};
    MakeEulerScale(static_cast<RndParticleCom*>(args.mComponent)->mSpin.mOrientation, euler, scale);
    return euler.z * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x618E60.
void SetZOrientation(const PropAccessorArgs& args) {
    Hmx::Matrix3& orientation = static_cast<RndParticleCom*>(args.mComponent)->mSpin.mOrientation;
    Vector3 euler = {0.0F, 0.0F, 0.0F};
    Vector3 scale = {0.0F, 0.0F, 0.0F};
    MakeEulerScale(orientation, euler, scale);
    const float angle = WrapOrientationAngle(*static_cast<const float*>(args.mValue));
    if (OrientationAngleChanged(euler.z, angle)) {
        euler.z = angle;
        SetOrientation(orientation, euler, scale);
    }
}

// Reconstructed from eboot.elf at 0x618FB0. A mesh's z spin is the
// sprite's spin.
Waveform<float> GetZRpmOverLife(const PropAccessorArgs& args) {
    return static_cast<RndParticleCom*>(args.mComponent)->mSpin.mRpmOverLife;
}

// Reconstructed from eboot.elf at 0x619090.
void SetZRpmOverLife(const PropAccessorArgs& args) {
    static_cast<RndParticleCom*>(args.mComponent)->mSpin.mRpmOverLife =
        *static_cast<const Waveform<float>*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x619110.
float GetZRpmDrag(const PropAccessorArgs& args) {
    return static_cast<RndParticleCom*>(args.mComponent)->mSpin.mRpmDrag;
}

// Reconstructed from eboot.elf at 0x619120.
void SetZRpmDrag(const PropAccessorArgs& args) {
    static_cast<RndParticleCom*>(args.mComponent)->mSpin.mRpmDrag = *static_cast<const float*>(args.mValue);
}

}  // namespace

// Reconstructed from eboot.elf at 0x619580. Each waveform shares the
// other's resources and draws a new evaluation state.
RndParticleCom::EmitterParams::EmitterParams(const EmitterParams& other) = default;

// Reconstructed from eboot.elf at 0x619720.
RndParticleCom::ParticleParams::ParticleParams(const ParticleParams& other) = default;

// Reconstructed from eboot.elf at 0x61A020.
RndParticleCom::ColorParams::ColorParams(const ColorParams& other) = default;

// Reconstructed from eboot.elf at 0x61A1A0.
RndParticleCom::SpinParams::SpinParams(const SpinParams& other) = default;

// Reconstructed from eboot.elf at 0x405B30. The binary emits the factory
// with the class's Init (0x3FA5F0), before the renderer's components; the
// placement factory at 0x405B60 constructs in given storage.
Component* RndParticleCom::_Create() {
    return new RndParticleCom();
}

// Reconstructed from eboot.elf at 0x3FA5F0. The binary emits the registration
// in render/RndInit.o.
void RndParticleCom::Init() {
    InitComponentClass<RndParticleCom>();
}
