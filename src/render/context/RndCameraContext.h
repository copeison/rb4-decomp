#pragma once

#include <cstddef>

#include "math/geometry/Frustum.h"
#include "math/geometry/Rect.h"
#include "math/geometry/Segment.h"
#include "math/matrix/Matrix3.h"
#include "math/matrix/Matrix4.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"
#include "math/vector/Vector4.h"
#include "utl/containers/FixedVector.h"

class GameObject;
class ObjPtr;
class RndShaderCBuffer;
class Sphere;

enum RndTextureCubeFace : int;

// What kind of texture a render-target binding draws into. The name is the
// map's; the enumerators are not in the reference map.
enum RndTargetMode : int {
    kTargetModeNone = -1,
    kTargetMode2D = 0,        // 2D textures and slices of 2D arrays.
    kTargetModeStereo = 1,    // Both eyes, one slice each.
    kTargetModeCube = 2,      // Cube textures.
    kTargetModeLeftEye = 3,   // One eye of a stereo pair.
    kTargetModeRightEye = 4,
    // 5 to 10 draw one face of a cube, face (mode - 5).
    kTargetModeCubeFace = 5,
};

// The 2D coordinate systems that Project and Unproject convert between.
// Enumerator names are not in the reference map.
enum Rnd2DCoord : int {
    k2DCoordPixels = 0,      // Viewport pixels.
    k2DCoordNormalized = 1,  // Zero to one across the viewport.
    // Viewport heights, with the square of the viewport's height centered.
    k2DCoordHeightNormalized = 2,
    k2DCoordNDC = 3,  // Minus one to one.
};

// Render-target slices of a target mode: one for none and 2D, two for
// stereo, six for cubes, none past the table. Every user inlines its own
// copy of the table (0x127E100 here, 0x12B6AD0 in RndContext.o). Name not
// in the reference map.
inline unsigned long RndTargetModeSlices(RndTargetMode mode) {
    static constexpr unsigned long kSlices[] = {1, 2, 6, 1, 1, 1, 1, 1, 1, 1, 1};
    const unsigned int index = mode == kTargetModeNone ? 0U : static_cast<unsigned int>(mode);
    return index < sizeof(kSlices) / sizeof(*kSlices) ? kSlices[index] : 0;
}

// The projection settings a camera context reads from the camera it is
// given. The binary reads them at fixed offsets from the pointer that the
// map's signatures type as GameObject const*; the object behaves like a
// camera component, whose +8 is the owning object that _CalcWorldXfms asks
// for its transform. Name and field names not in the reference map.
struct RndCameraSettings {
    // A stereo eye's view: its world transform and its side angles.
    struct Eye {
        Transform mXfm;
        Frustum::Fov mFov;
    };

    // The component's vtable pointer; never read here.
    unsigned char mVtable[8];
    GameObject* mOwner;
    // The rest of the Component base (its flags) and its tail padding;
    // never read here.
    unsigned char mComponentFlags[8];
    float mNearPlane;
    float mFarPlane;
    bool mOrthographic;
    float mYFov;          // Full vertical angle, in radians.
    float mOrthoHeight;   // Height of the orthographic view volume.
    // The stereo modes take each eye's transform and angles from mEyes.
    bool mUseEyes;
    Eye mEyes[2];  // Left, then right.
};

static_assert(sizeof(RndCameraSettings::Eye) == 64);
static_assert(offsetof(RndCameraSettings, mOwner) == 8);
static_assert(offsetof(RndCameraSettings, mNearPlane) == 24);
static_assert(offsetof(RndCameraSettings, mFarPlane) == 28);
static_assert(offsetof(RndCameraSettings, mOrthographic) == 32);
static_assert(offsetof(RndCameraSettings, mYFov) == 36);
static_assert(offsetof(RndCameraSettings, mOrthoHeight) == 40);
static_assert(offsetof(RndCameraSettings, mUseEyes) == 44);
static_assert(offsetof(RndCameraSettings, mEyes) == 48);

// Camera and projection state for one render target: the camera, the target
// it draws into, the transforms and frusta derived from both, and the
// level-of-detail frusta. RndContext embeds two.
class RndCameraContext {
public:
    // A level-of-detail band: its far distance, whether the camera's range
    // reaches it, and its frustum. The constructor is inlined into the
    // context's. Field names are not in the reference map.
    struct LodData {
        LodData() : mDistance(0.0F), mActive(false) {}

        float mDistance;
        bool mActive;
        Frustum mFrustum;
    };

    // Transforms and frusta of one render-target slice; the context keeps
    // one for the whole target and one per slice. Name and field names not
    // in the reference map.
    struct View {
        View();  // 0x3DD150

        Transform mWorldXfm;  // Camera to world.
        Transform mInvWorldXfm;
        Hmx::Matrix3 mWorldRotationTranspose;
        // World to view space: mInvWorldXfm with the y and z axes swapped.
        Transform mViewXfm;
        Hmx::Matrix4 mProjection;
        Hmx::Matrix4 mViewProjection;
        Hmx::Matrix4 mInvViewProjection;
        Frustum mViewFrustum;   // In view space.
        Frustum mWorldFrustum;  // In world space.
    };

    RndCameraContext();  // 0x3D8170
    RndCameraContext(const GameObject* camera, const Vector2& viewportSize);  // 0x3D8480
    // Not located in this build.
    RndCameraContext(const ObjPtr& camera, const Vector2& viewportSize);

    void Clear();  // 0x3D88C0
    // Returns whether the camera changed.
    bool SetCamera(const GameObject* camera);  // 0x3D8910
    // Not located in this build.
    bool SetCamera(const ObjPtr& camera);
    // Returns whether the target changed.
    bool SetRenderTargetInfo(
        RndTargetMode mode,
        const Vector2& viewportSize,
        const Vector2& depthRange);  // 0x3D8930
    // Returns whether the rectangle changed.
    bool SetProjectionRect(const Hmx::Rect& rect);  // 0x3D8990
    // The mask is stored as is; the three distances end the LOD bands.
    void SetLodSettings(unsigned int lodMask, const float* distances);  // 0x3D89E0

    // Projects a world point through the whole target's view-projection,
    // and stores its depth, z over w, when depth is not null. A point with
    // a zero w projects to the origin with zero depth.
    Vector2 Project(const Vector3& point, Rnd2DCoord coord, float* depth) const;  // 0x3D9190
    // The world point at the given view-space depth under the 2D point.
    Vector3 Unproject(const Vector2& point, float depth, Rnd2DCoord coord) const;  // 0x3D9350
    // Unprojects the point at the camera's near and far planes. Name not in
    // the reference map.
    Segment UnprojectSegment(const Vector2& point, Rnd2DCoord coord) const;  // 0x3D94D0
    // The sphere's projected diameter, in the coordinate system's units.
    float CalcProjectedHeight(const Sphere& sphere, Rnd2DCoord coord) const;  // 0x3D9560

    // Writes each slice's view-projection matrix.
    void SetViewProjectionShaderConstants(RndShaderCBuffer& cbuffer) const;  // 0x3D9750
    // Writes the near and far planes, the depth range, the view extents and
    // each slice's world transforms.
    void SetOtherShaderConstants(RndShaderCBuffer& cbuffer) const;  // 0x3D9810
    // Called on the context's second camera after the first camera's
    // constants; empty in this build. Names not in the reference map.
    void SetStereoViewProjectionShaderConstants(RndShaderCBuffer& cbuffer) const;  // 0x3D9AE0
    void SetStereoOtherShaderConstants(RndShaderCBuffer& cbuffer) const;           // 0x3D9AF0
    static void SetIdentityViewProjectionShaderConstants(
        RndTargetMode mode,
        RndShaderCBuffer& cbuffer);  // 0x3D9B00
    // Writes the constants of a context without a camera.
    static void SetDefaultShaderConstants(
        RndTargetMode mode,
        RndShaderCBuffer& cbuffer);  // 0x3D9BC0

    // Keeps one view per slice of the target and rederives everything from
    // the camera, or marks the context invalid without a camera or target.
    void _SyncDerived();  // 0x3D87E0
    // Rebuilds the frusta of the LOD bands the camera's range reaches.
    // SetLodSettings (0x3D89E0) carries a copy that _SyncDerived jumps into
    // at 0x3D8A20; the binary has no separate entry point.
    void _SyncLodData();
    // Takes each slice's camera-to-world transform from the camera's
    // transform component, the stereo eyes or the cube faces, and derives
    // the inverse and the rotation's transpose.
    void _CalcWorldXfms();  // 0x3D9DF0
    void _CalcViewXfms();   // 0x3DA8D0
    // Builds each view's projection from the projection rectangle and the
    // camera, then its view-projection and inverse.
    void _CalcProjectionMatrices();  // 0x3DAA40
    // Builds the view frusta of the whole target and of each slice, and the
    // combined world frustum.
    void _CalcPrimaryFrusta();  // 0x3DB020
    // Inlined into _CalcWorldXfms. Rotates the transform to face one side
    // of the cube. The map gives no return type.
    Transform _CalcCubeFaceWorldXfm(const Transform& xfm, RndTextureCubeFace face);
    // Sets the view frustum between the two depths, each slice's view
    // frustum, their world frusta, and the frustum that bounds every slice.
    // The map has _CalcFrusta(float, float, Frustum&, Frustum*, Frustum&)
    // const; this build also takes the world frustum and the slices' world
    // frusta.
    void _CalcFrusta(
        float nearPlane,
        float farPlane,
        Frustum& viewFrustum,
        Frustum& worldFrustum,
        Frustum* sliceViewFrusta,
        Frustum* sliceWorldFrusta,
        Frustum& combinedFrustum) const;  // 0x3DBFD0
    // Inlined into its callers.
    float _CalcAspectRatio() const;
    // Inlined into its callers.
    float _GetPerspectiveYFov() const;

    // The camera's projection settings. Name not in the reference map.
    const RndCameraSettings& GetCameraSettings() const {
        return *reinterpret_cast<const RndCameraSettings*>(mCamera);
    }

    // Field names are not in the reference map.
    const GameObject* mCamera;
    RndTargetMode mTargetMode;
    Vector2 mViewportSize;  // In pixels.
    Vector2 mDepthRange;    // Minimum and maximum depth.
    Hmx::Rect mProjectionRect;
    Frustum mFrustum;
    // A cache of up to four 16-byte entries derived from the camera,
    // emptied whenever the derived state is rebuilt. Nothing in this build
    // adds to it or reads it, so the element type is not recovered and the
    // name is a guess. Name not in the reference map.
    FixedVector<Vector4, 4> mDerivedCache;
    bool mValid;  // Has a camera and a target.
    unsigned int mLodMask;
    bool mLodSettingsSet;
    LodData mLodData[3];
    View mPrimaryView;  // The whole target.
    FixedVector<View, 6> mViews;  // One per slice.
};

static_assert(offsetof(RndCameraContext::LodData, mFrustum) == 8);
static_assert(sizeof(RndCameraContext::LodData) == 368);
static_assert(offsetof(RndCameraContext::View, mInvWorldXfm) == 48);
static_assert(offsetof(RndCameraContext::View, mWorldRotationTranspose) == 96);
static_assert(offsetof(RndCameraContext::View, mViewXfm) == 132);
static_assert(offsetof(RndCameraContext::View, mProjection) == 180);
static_assert(offsetof(RndCameraContext::View, mViewProjection) == 244);
static_assert(offsetof(RndCameraContext::View, mInvViewProjection) == 308);
static_assert(offsetof(RndCameraContext::View, mViewFrustum) == 376);
static_assert(offsetof(RndCameraContext::View, mWorldFrustum) == 736);
static_assert(sizeof(RndCameraContext::View) == 1096);
static_assert(offsetof(RndCameraContext, mTargetMode) == 8);
static_assert(offsetof(RndCameraContext, mViewportSize) == 12);
static_assert(offsetof(RndCameraContext, mDepthRange) == 20);
static_assert(offsetof(RndCameraContext, mProjectionRect) == 28);
static_assert(offsetof(RndCameraContext, mFrustum) == 48);
static_assert(offsetof(RndCameraContext, mDerivedCache) == 408);
static_assert(offsetof(RndCameraContext, mValid) == 496);
static_assert(offsetof(RndCameraContext, mLodMask) == 500);
static_assert(offsetof(RndCameraContext, mLodSettingsSet) == 504);
static_assert(offsetof(RndCameraContext, mLodData) == 512);
static_assert(offsetof(RndCameraContext, mPrimaryView) == 1616);
static_assert(offsetof(RndCameraContext, mViews) == 2712);
static_assert(sizeof(RndCameraContext) == 9312);
