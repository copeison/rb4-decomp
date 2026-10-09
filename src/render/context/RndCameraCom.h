#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "math/geometry/Frustum.h"
#include "math/transform/Transform.h"
#include "utl/text/Symbol.h"

// The camera component (render/RndCameraCom.o, 0x6B72B0-0x6B88BB). Its
// class id is "Camera". A camera context reads its projection settings
// (RndCameraSettings mirrors them); the scene's camera hands itself to its
// scene drawer every poll. The vtable at 0x19370E8 has 41 slots. The object
// is 176 bytes.
class RndCameraCom : public Component {
public:
    // A stereo eye's view: its world transform and its side angles. Inlined
    // into the class's constructor and _Imprint. Name and field names not in
    // the reference map.
    struct Eye {
        Eye();  // 0x6B7380

        Transform mXfm;
        Frustum::Fov mFov;
    };

    // The stereo eyes, which the camera context uses instead of the camera's
    // own transform and angles when mUseEyes is set. Nothing in this build
    // sets them; they are reset rather than copied by _Imprint. Name and
    // field names not in the reference map.
    struct Stereo {
        Stereo();  // 0x6B73B0

        bool mUseEyes;
        Eye mEyes[2];  // Left, then right.
    };

    RndCameraCom();            // 0x6B72F0
    // Copies the projection settings for an imprint and resets the stereo
    // eyes. Inlined into _Imprint (0x6B8170). Not in the reference map.
    RndCameraCom(const RndCameraCom& other)
        : Component(other),
          mNearDistance(other.mNearDistance),
          mFarDistance(other.mFarDistance),
          mOrthographic(other.mOrthographic),
          mPixelAccurate(other.mPixelAccurate),
          mPerspectiveFov(other.mPerspectiveFov),
          mOrthoHeight(other.mOrthoHeight),
          mStereo() {}
    ~RndCameraCom() override;  // slots 0-1: 0x6B73F0, 0x6B7400

    Symbol GetId() const override;         // slot 4: 0x6B80F0
    Symbol GetClassName() const override;  // slot 5: 0x6B8100
    int CurrentRev() const override;       // slot 7: 0x6B8110
    bool IsA(Symbol type) const override;  // slot 8: 0x6B8130
    Component* AsComponent() override;     // slot 9: 0x6B8160
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // projection settings and resets the stereo eyes.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6B8170
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6B82F0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6B8300
    // Slot 29: a pixel-accurate orthographic camera takes the content
    // height as its height.
    bool _OnResourcesLoaded() override;  // 0x6B7F70
    // Slot 33: the scene's main camera points its scene drawer at itself,
    // projecting into the instancing scene's rectangle when there is one.
    // The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0x6B7FA0

    // The defaults the properties start from, and the largest ratio of the
    // far to the near distance.
    static float GetDefaultNearDistance();  // 0x6B72B0
    static float GetDefaultFarDistance();   // 0x6B72C0
    static float GetDefaultYFov();          // 0x6B72D0
    static float GetMaxFarNearRatio();      // 0x6B72E0

    // The class factory. Emitted in render/RndInit.o at 0x404160.
    static Component* _Create();
    // Describes the class ("Used for sweet sweet rendering"): a camera
    // class allowed in scene resources, also found as "RndCameraCom", that
    // requires a TransCom, with the properties near_distance,
    // far_distance, orthographic, pixel_accurate, perspective_fov (edited
    // in degrees) and ortho_height. Their callbacks keep the far distance
    // at least twice the near one and the near distance at most half the
    // far one, take the content height for a pixel-accurate camera, and
    // hide the settings of the other projection. Not reconstructed: the
    // property metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6B7420

    static Symbol sId;  // 0x1AAF8F0, "Camera"
    // Also "Camera"; the component factory is given it. Name not in the
    // reference map.
    static Symbol sClassName;           // 0x1AAF8F8
    static PropRegistry sPropRegistry;  // 0x1AAF900
    static ComMetaData sMetaData;       // 0x1AAF9A0

    // Field names are not in the reference map; they follow the
    // properties.
    float mNearDistance;    // "near_distance"
    float mFarDistance;     // "far_distance"
    bool mOrthographic;     // "orthographic"
    // "pixel_accurate": an orthographic camera's height follows the
    // content resolution.
    bool mPixelAccurate;
    // "perspective_fov": the full vertical angle, in radians; the property
    // shows degrees.
    float mPerspectiveFov;
    float mOrthoHeight;  // "ortho_height", in world units.
    Stereo mStereo;
};

static_assert(sizeof(RndCameraCom::Eye) == 64);
static_assert(sizeof(RndCameraCom::Stereo) == 132);
static_assert(offsetof(RndCameraCom, mNearDistance) == 0x18);
static_assert(offsetof(RndCameraCom, mFarDistance) == 0x1C);
static_assert(offsetof(RndCameraCom, mOrthographic) == 0x20);
static_assert(offsetof(RndCameraCom, mPixelAccurate) == 0x21);
static_assert(offsetof(RndCameraCom, mPerspectiveFov) == 0x24);
static_assert(offsetof(RndCameraCom, mOrthoHeight) == 0x28);
static_assert(offsetof(RndCameraCom, mStereo) == 0x2C);
static_assert(sizeof(RndCameraCom) == 0xB0);
