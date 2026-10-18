// render/RndCameraCom.o (0x6B72B0 to 0x6B88BB).
#include "render/context/RndCameraCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "math/geometry/Rect.h"
#include "render/context/RndCameraContext.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

namespace {

// Gives a pixel-accurate camera the content height as its orthographic
// height. Inlined into the "pixel_accurate" callback and the revision
// handler. Name not in the reference map.
void SyncPixelAccurateHeight(RndCameraCom& camera) {
    if (camera.mPixelAccurate) {
        camera.mOrthoHeight = static_cast<float>(TheRndDevice()->mSettings->mContentResolution.y);
    }
}

// The property callbacks _Init installs (0x6B8310 to 0x6B83A0). Names not
// in the reference map.
void OnNearDistanceChanged(const PropAccessorArgs& args);
void OnFarDistanceChanged(const PropAccessorArgs& args);
void OnPixelAccurateChanged(const PropAccessorArgs& args);
float GetPerspectiveFovDegrees(const PropAccessorArgs& args);
void SetPerspectiveFovDegrees(const PropAccessorArgs& args);

}  // namespace


// The camera context reads the component through RndCameraSettings.
static_assert(offsetof(RndCameraCom, mNearDistance) == offsetof(RndCameraSettings, mNearPlane));
static_assert(offsetof(RndCameraCom, mFarDistance) == offsetof(RndCameraSettings, mFarPlane));
static_assert(offsetof(RndCameraCom, mOrthographic) == offsetof(RndCameraSettings, mOrthographic));
static_assert(offsetof(RndCameraCom, mPerspectiveFov) == offsetof(RndCameraSettings, mYFov));
static_assert(offsetof(RndCameraCom, mOrthoHeight) == offsetof(RndCameraSettings, mOrthoHeight));
static_assert(offsetof(RndCameraCom, mStereo) == offsetof(RndCameraSettings, mUseEyes));
static_assert(
    offsetof(RndCameraCom, mStereo) + offsetof(RndCameraCom::Stereo, mEyes) ==
    offsetof(RndCameraSettings, mEyes));

// The object's statics, in the order of its static initializer (0x6B87F0).
// The three ints it first sets (0x1AAF8E4) come from a shared header and are
// not modelled.
Symbol RndCameraCom::sId("Camera");
Symbol RndCameraCom::sClassName("Camera");
PropRegistry RndCameraCom::sPropRegistry;
ComMetaData RndCameraCom::sMetaData;

// Reconstructed from eboot.elf at 0x6B72B0.
float RndCameraCom::GetDefaultNearDistance() {
    return 0.1F;
}

// Reconstructed from eboot.elf at 0x6B72C0.
float RndCameraCom::GetDefaultFarDistance() {
    return 1000.0F;
}

// Reconstructed from eboot.elf at 0x6B72D0. About 34.5 degrees.
float RndCameraCom::GetDefaultYFov() {
    return 0.6024178F;
}

// Reconstructed from eboot.elf at 0x6B72E0.
float RndCameraCom::GetMaxFarNearRatio() {
    return 10000.0F;
}

// Reconstructed from eboot.elf at 0x6B72F0. The defaults are the values
// the static getters return.
RndCameraCom::RndCameraCom()
    : mNearDistance(0.1F),
      mFarDistance(1000.0F),
      mOrthographic(false),
      mPixelAccurate(false),
      mPerspectiveFov(0.6024178F),
      mOrthoHeight(10.0F) {}

// Reconstructed from eboot.elf at 0x6B7380. The class's constructor
// inlines it.
RndCameraCom::Eye::Eye() : mXfm(Transform::sID), mFov{0.0F, 0.0F, 0.0F, 0.0F} {}

// Reconstructed from eboot.elf at 0x6B73B0. The class's constructor
// inlines it.
RndCameraCom::Stereo::Stereo() : mUseEyes(false) {}

// Reconstructed from eboot.elf at 0x6B73F0. The deleting destructor is at
// 0x6B7400.
RndCameraCom::~RndCameraCom() {}

// Reconstructed from eboot.elf at 0x6B7420. The clip distances keep the far
// plane beyond the near one; the field of view is edited in degrees and
// kept in radians. Revision 1 sizes a pixel-accurate camera saved before it
// to the content height.
void RndCameraCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Used for sweet sweet rendering";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryCamera;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    metadata.mLightweight = true;
    metadata.mRequiredComponents.push_back(TransCom::sClassName);
    metadata.mAliases.push_back(Symbol("RndCameraCom"));
    RndCameraCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev <= 0) {
                SyncPixelAccurateHeight(*static_cast<RndCameraCom*>(args.mComponent));
            }
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    FloatMetadata& nearDistance = TypeSpecificMetadata(
        registry.RegisterProp("near_distance", offsetof(RndCameraCom, mNearDistance), kPropertyFloat, 4),
        prototype.mNearDistance);
    nearDistance.mDefault = prototype.mNearDistance;
    static_cast<void>(Symbol("prop"));
    nearDistance.mMin = 0.01F;
    nearDistance.mOnChanged = OnNearDistanceChanged;
    nearDistance.mHelp = String("Distance to the near clip plane");

    FloatMetadata& farDistance = TypeSpecificMetadata(
        registry.RegisterProp("far_distance", offsetof(RndCameraCom, mFarDistance), kPropertyFloat, 4),
        prototype.mFarDistance);
    farDistance.mDefault = prototype.mFarDistance;
    static_cast<void>(Symbol("prop"));
    farDistance.mMin = 0.02F;
    farDistance.mOnChanged = OnFarDistanceChanged;
    farDistance.mHelp = String("Distance to the far clip plane");

    BoolMetadata& orthographic = TypeSpecificMetadata(
        registry.RegisterProp("orthographic", offsetof(RndCameraCom, mOrthographic), kPropertyBool, 1),
        prototype.mOrthographic);
    orthographic.mDefault = prototype.mOrthographic;
    static_cast<void>(Symbol("prop"));
    orthographic.mHelp = String("Switch between perspective and orthographic projection");

    BoolMetadata& pixelAccurate = TypeSpecificMetadata(
        registry.RegisterProp("pixel_accurate", offsetof(RndCameraCom, mPixelAccurate), kPropertyBool, 1),
        prototype.mPixelAccurate);
    pixelAccurate.mDefault = prototype.mPixelAccurate;
    static_cast<void>(Symbol("prop"));
    pixelAccurate.mHelp = String(
        "For orthographic cameras, forces Y height to match the project's content resolution, yielding "
        "pixel-accurate coordinates");
    pixelAccurate.mOnChanged = OnPixelAccurateChanged;
    pixelAccurate.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndCameraCom&>(args.mComponent).mOrthographic;
    });

    FloatMetadata& fov = TypeSpecificMetadata(
        registry.RegisterProp("perspective_fov", offsetof(RndCameraCom, mPerspectiveFov), kPropertyFloat, 4),
        prototype.mPerspectiveFov);
    fov.mDefault = prototype.mPerspectiveFov;
    static_cast<void>(Symbol("prop"));
    fov.mHelp = String("Vertical field-of-view for perspective cameras (in degrees)");
    fov.mGetter = GetPerspectiveFovDegrees;
    fov.mSetter = SetPerspectiveFovDegrees;
    fov.mDefault = 34.516F;
    fov.mMin = 1.0F;
    fov.mMax = 100.0F;
    fov.mUnits = Symbol(" degrees");
    fov.mSlider = true;
    fov.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndCameraCom&>(args.mComponent).mOrthographic;
    });

    FloatMetadata& orthoHeight = TypeSpecificMetadata(
        registry.RegisterProp("ortho_height", offsetof(RndCameraCom, mOrthoHeight), kPropertyFloat, 4),
        prototype.mOrthoHeight);
    orthoHeight.mDefault = prototype.mOrthoHeight;
    static_cast<void>(Symbol("prop"));
    orthoHeight.mHelp =
        String("Vertical size of the camera viewport for orthographic cameras (in world units)");
    orthoHeight.mMin = 0.01F;
    orthoHeight.mReadOnly._Set([](const TagArgs& args) {
        const RndCameraCom& camera = static_cast<const RndCameraCom&>(args.mComponent);
        if (!camera.mOrthographic) {
            return true;
        }
        return camera.mPixelAccurate;
    });
}

// Reconstructed from eboot.elf at 0x6B7F70.
bool RndCameraCom::_OnResourcesLoaded() {
    if (mOrthographic && mPixelAccurate) {
        mOrthoHeight = static_cast<float>(TheRndDevice()->mSettings->mContentResolution.y);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x6B7FA0. Without an instancing scene the
// camera projects into the whole target.
void RndCameraCom::_Poll() {
    RndSceneCom* scene = mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    if (scene == nullptr || scene->GetCamera(0) != this) {
        return;
    }
    Hmx::Rect projectionRect(0.0F, 0.0F, 1.0F, 1.0F);
    const GameObject* parent = mObject->mEntity->mParentObject;
    if (parent != nullptr) {
        const RndSceneInstanceCom* instance = parent->GetCom<RndSceneInstanceCom>();
        if (instance != nullptr) {
            projectionRect = instance->CalcProjectionRect(
                *TheRndDevice()->mCapabilities[kPlatformPS4].mResolutions.begin());
        }
    }
    scene->mSceneDrawer->SetSceneCamera(this, projectionRect);
}

// Reconstructed from eboot.elf at 0x6B80F0.
Symbol RndCameraCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6B8100.
Symbol RndCameraCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6B8110.
int RndCameraCom::CurrentRev() const {
    return const_cast<RndCameraCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6B8130.
bool RndCameraCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6B8160.
Component* RndCameraCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6B8170. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndCameraCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndCameraCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndCameraCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6B82F0.
PropRegistry& RndCameraCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6B8300.
ComMetaData& RndCameraCom::_GetMetaData() {
    return sMetaData;
}

namespace {

// Reconstructed from eboot.elf at 0x6B8310. Keeps the far plane at least
// twice the near distance.
void OnNearDistanceChanged(const PropAccessorArgs& args) {
    RndCameraCom* const camera = static_cast<RndCameraCom*>(args.mComponent);
    const float minFar = camera->mNearDistance + camera->mNearDistance;
    camera->mFarDistance = minFar > camera->mFarDistance ? minFar : camera->mFarDistance;
}

// Reconstructed from eboot.elf at 0x6B8330. Keeps the near plane at most
// half the far distance.
void OnFarDistanceChanged(const PropAccessorArgs& args) {
    RndCameraCom* const camera = static_cast<RndCameraCom*>(args.mComponent);
    const float maxNear = camera->mFarDistance * 0.5F;
    camera->mNearDistance = maxNear < camera->mNearDistance ? maxNear : camera->mNearDistance;
}

// Reconstructed from eboot.elf at 0x6B8350.
void OnPixelAccurateChanged(const PropAccessorArgs& args) {
    SyncPixelAccurateHeight(*static_cast<RndCameraCom*>(args.mComponent));
}

// Reconstructed from eboot.elf at 0x6B8380.
float GetPerspectiveFovDegrees(const PropAccessorArgs& args) {
    return static_cast<RndCameraCom*>(args.mComponent)->mPerspectiveFov * 57.29578F;
}

// Reconstructed from eboot.elf at 0x6B83A0.
void SetPerspectiveFovDegrees(const PropAccessorArgs& args) {
    static_cast<RndCameraCom*>(args.mComponent)->mPerspectiveFov =
        *static_cast<const float*>(args.mValue) * 0.017453292F;
}

}  // namespace

// Reconstructed from eboot.elf at 0x404160. The binary emits the factory in
// render/RndInit.o with the class's Init.
Component* RndCameraCom::_Create() {
    return new RndCameraCom();
}
