// render/RndCameraCom.o (0x6B72B0 to 0x6B88BB).
#include "render/context/RndCameraCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "math/geometry/Rect.h"
#include "render/context/RndCameraContext.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

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

// Reconstructed from eboot.elf at 0x404160. The binary emits the factory in
// render/RndInit.o with the class's Init.
Component* RndCameraCom::_Create() {
    return new RndCameraCom();
}
