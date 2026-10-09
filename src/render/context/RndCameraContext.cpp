#include "render/context/RndCameraContext.h"

#include <cmath>
#include <cstring>

#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/geometry/Sphere.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"

namespace {

// Each render-target slice's camera constants: the view-projection matrix
// and the world and inverse-world transforms, as rows of transposed
// matrices. Names not in the reference map.
constexpr unsigned long kSliceFloats = 40;
constexpr unsigned long kSliceXfmsOffset = 16;
constexpr unsigned int kDefaultLodMask = 7;

float* CBufferFloats(RndShaderCBuffer& cbuffer, unsigned long memberOffset) {
    return static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(cbuffer, memberOffset));
}

// Stores the matrix's columns as rows.
void StoreTransposed(float* out, const Hmx::Matrix4& matrix) {
    const Vector4* rows[] = {&matrix.x, &matrix.y, &matrix.z, &matrix.w};
    for (int row = 0; row < 4; ++row) {
        out[row] = rows[row]->x;
        out[4 + row] = rows[row]->y;
        out[8 + row] = rows[row]->z;
        out[12 + row] = rows[row]->w;
    }
}

// Stores the transform as three rows of a transposed 3x4 matrix.
void StoreTransposed(float* out, const Transform& xfm) {
    const Vector3* rows[] = {&xfm.m.x, &xfm.m.y, &xfm.m.z, &xfm.v};
    for (int row = 0; row < 4; ++row) {
        out[row] = rows[row]->x;
        out[4 + row] = rows[row]->y;
        out[8 + row] = rows[row]->z;
    }
}

constexpr float kQuarterPi = 0.7853982F;
constexpr float kBehindCameraHeight = 1.0e30F;

// Target modes with their own projection rules. Names not in the reference
// map.
bool IsCubeFaceTargetMode(RndTargetMode mode) {
    const int value = mode;
    return value >= kTargetModeCubeFace && value < kTargetModeCubeFace + 6;
}

bool IsCubeTargetMode(RndTargetMode mode) {
    return mode == kTargetModeCube || IsCubeFaceTargetMode(mode);
}

bool IsStereoTargetMode(RndTargetMode mode) {
    return mode == kTargetModeStereo || mode == kTargetModeLeftEye ||
        mode == kTargetModeRightEye;
}

// Clamps a LOD distance into the camera's depth range. Name not in the
// reference map.
float ClampDistance(float distance, float nearPlane, float farPlane) {
    if (farPlane < distance) {
        return farPlane;
    }
    return nearPlane > distance ? nearPlane : distance;
}

// The side angles of a perspective view: a cube face's quarter turns, a
// stereo eye's own angles, or the camera's vertical angle widened by the
// aspect ratio. The slice is -1 for the whole target, which in stereo mode
// takes the left eye's vertical angles and the wider of the two eyes'
// horizontal ones. Inlined into _CalcProjectionMatrices and _CalcFrusta.
// Name not in the reference map.
Frustum::Fov CalcPerspectiveFov(const RndCameraContext& context, long slice) {
    const RndCameraSettings& settings = context.GetCameraSettings();
    const RndTargetMode mode = context.mTargetMode;
    if (IsCubeTargetMode(mode)) {
        return {kQuarterPi, kQuarterPi, kQuarterPi, kQuarterPi};
    }
    if (IsStereoTargetMode(mode) && settings.mUseEyes) {
        if (slice == -1 && mode == kTargetModeStereo) {
            const Frustum::Fov& left = settings.mEyes[0].mFov;
            const Frustum::Fov& right = settings.mEyes[1].mFov;
            return {
                left.mUp,
                left.mDown,
                right.mLeft > left.mLeft ? right.mLeft : left.mLeft,
                right.mRight > left.mRight ? right.mRight : left.mRight,
            };
        }
        long eye = -1;
        if (mode == kTargetModeLeftEye || mode == kTargetModeRightEye) {
            eye = mode - kTargetModeLeftEye;
        } else if (mode == kTargetModeStereo) {
            eye = slice;
        }
        return settings.mEyes[eye].mFov;
    }
    const float halfYFov = 0.5F * context._GetPerspectiveYFov();
    const float halfXFov = std::atan(std::tan(halfYFov) * context._CalcAspectRatio());
    return {halfYFov, halfYFov, halfXFov, halfXFov};
}

// The frustum moved into world space: the planes are transformed and the
// corners rebuilt from them. Inlined into _CalcFrusta. Name not in the
// reference map.
Frustum TransformFrustum(const Frustum& frustum, const Transform& xfm) {
    Frustum result;
    for (unsigned long i = 0; i < 6; ++i) {
        Multiply(frustum.mPlanes.mData[i], xfm, result.mPlanes.mData[i]);
    }
    result._DoUpdateHull();
    return result;
}

// Normalized device coordinates to a 2D coordinate system, through pixels.
// Inlined into Project and CalcProjectedHeight. Name not in the reference
// map.
Vector2 NDCTo2DCoord(const Vector2& ndc, const Vector2& viewport, Rnd2DCoord coord) {
    const Vector2 pixels = {
        (ndc.x + 1.0F) * 0.5F * viewport.x,
        (ndc.y + 1.0F) * 0.5F * viewport.y,
    };
    switch (coord) {
    case k2DCoordPixels:
        return pixels;
    case k2DCoordNormalized:
        return {pixels.x / viewport.x, pixels.y / viewport.y};
    case k2DCoordHeightNormalized:
        return {
            (pixels.x + (viewport.y - viewport.x) * 0.5F) / viewport.y,
            pixels.y / viewport.y,
        };
    case k2DCoordNDC:
        return {
            pixels.x * (2.0F / viewport.x) - 1.0F,
            pixels.y * (2.0F / viewport.y) - 1.0F,
        };
    default:
        return {0.0F, 0.0F};
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DD150.
RndCameraContext::View::View()
    : mWorldXfm(Transform::sID),
      mInvWorldXfm(Transform::sID),
      mWorldRotationTranspose(Hmx::Matrix3::sID),
      mViewXfm(Transform::sID),
      mProjection(),
      mViewProjection(),
      mInvViewProjection() {}

// Reconstructed from eboot.elf at 0x3D8170.
RndCameraContext::RndCameraContext()
    : mCamera(nullptr),
      mTargetMode(kTargetModeNone),
      mViewportSize(Vector2::sZero),
      mDepthRange(Vector2::sZero),
      mProjectionRect(0.0F, 0.0F, 1.0F, 1.0F),
      mValid(false),
      mLodMask(kDefaultLodMask),
      mLodSettingsSet(false) {}

// Reconstructed from eboot.elf at 0x3D8480.
RndCameraContext::RndCameraContext(const GameObject* camera, const Vector2& viewportSize)
    : mCamera(camera),
      mTargetMode(kTargetMode2D),
      mViewportSize(viewportSize),
      mDepthRange{0.0F, 1.0F},
      mProjectionRect(0.0F, 0.0F, 1.0F, 1.0F),
      mValid(true),
      mLodMask(kDefaultLodMask),
      mLodSettingsSet(false) {
    _SyncDerived();
}

// Reconstructed from eboot.elf at 0x3D88C0. The derived state is kept.
void RndCameraContext::Clear() {
    mValid = false;
    mCamera = nullptr;
    mTargetMode = kTargetModeNone;
    mViewportSize = Vector2::sZero;
    mDepthRange = Vector2::sZero;
    mProjectionRect = Hmx::Rect(0.0F, 0.0F, 1.0F, 1.0F);
    mLodMask = kDefaultLodMask;
    mLodSettingsSet = false;
}

// Reconstructed from eboot.elf at 0x3D8910.
bool RndCameraContext::SetCamera(const GameObject* camera) {
    if (mCamera == camera) {
        return false;
    }
    mCamera = camera;
    _SyncDerived();
    return true;
}

// Reconstructed from eboot.elf at 0x3D8930.
bool RndCameraContext::SetRenderTargetInfo(
    RndTargetMode mode,
    const Vector2& viewportSize,
    const Vector2& depthRange) {
    if (mTargetMode == mode && mViewportSize.x == viewportSize.x &&
        mViewportSize.y == viewportSize.y && mDepthRange.x == depthRange.x &&
        mDepthRange.y == depthRange.y) {
        return false;
    }
    mTargetMode = mode;
    mViewportSize = viewportSize;
    mDepthRange = depthRange;
    _SyncDerived();
    return true;
}

// Reconstructed from eboot.elf at 0x3D8990.
bool RndCameraContext::SetProjectionRect(const Hmx::Rect& rect) {
    if (rect.x == mProjectionRect.x && rect.y == mProjectionRect.y &&
        rect.w == mProjectionRect.w && rect.h == mProjectionRect.h) {
        return false;
    }
    mProjectionRect = rect;
    _SyncDerived();
    return true;
}

// Reconstructed from eboot.elf at 0x3D89E0. The binary inlines
// _SyncLodData here; _SyncDerived jumps into that copy at 0x3D8A20.
void RndCameraContext::SetLodSettings(unsigned int lodMask, const float* distances) {
    mLodSettingsSet = true;
    mLodMask = lodMask;
    for (unsigned long i = 0; i < 3; ++i) {
        mLodData[i].mDistance = distances[i];
    }
    if (mValid) {
        _SyncLodData();
    }
}

// Reconstructed from eboot.elf at 0x3D8A20, the tail of SetLodSettings.
// Each band runs from its own clamped distance to the next band's, the last
// to the far plane; a band the camera's range does not reach is inactive.
void RndCameraContext::_SyncLodData() {
    const RndCameraSettings& settings = GetCameraSettings();
    const float nearPlane = settings.mNearPlane;
    const float farPlane = settings.mFarPlane;
    float distances[3];
    for (unsigned long i = 0; i < 3; ++i) {
        distances[i] = ClampDistance(mLodData[i].mDistance, nearPlane, farPlane);
    }

    Frustum viewFrustum;
    Frustum worldFrustum;
    Frustum sliceViewFrusta[6];
    Frustum sliceWorldFrusta[6];
    const float ends[3] = {distances[1], distances[2], farPlane};
    for (unsigned long i = 0; i < 3; ++i) {
        mLodData[i].mActive = distances[i] < ends[i];
        if (mLodData[i].mActive) {
            _CalcFrusta(
                distances[i],
                ends[i],
                viewFrustum,
                worldFrustum,
                sliceViewFrusta,
                sliceWorldFrusta,
                mLodData[i].mFrustum);
        }
    }
}

// Reconstructed from eboot.elf at 0x3D87E0.
void RndCameraContext::_SyncDerived() {
    if (mCamera == nullptr || mViewportSize.x == 0.0F || mViewportSize.y == 0.0F ||
        mTargetMode == kTargetModeNone) {
        mValid = false;
        return;
    }
    mValid = true;
    mViews.resize(RndTargetModeSlices(mTargetMode));
    _CalcWorldXfms();
    _CalcViewXfms();
    _CalcProjectionMatrices();
    _CalcPrimaryFrusta();
    mDerivedCache.mSize = 0;
    if (mLodSettingsSet) {
        _SyncLodData();
    }
}

// Reconstructed from eboot.elf at 0x3D9DF0. The camera's transform comes
// from the transform component of the object that owns it. Stereo eyes and
// cube faces fill one view each; the whole target takes the camera's
// transform in stereo, the forward face for cubes, and the first slice
// otherwise.
void RndCameraContext::_CalcWorldXfms() {
    const RndCameraSettings& settings = GetCameraSettings();
    const Transform& xfm = settings.mOwner->GetCom<TransCom>()->mWorldXfm;
    View* views = mViews.mData;
    switch (mTargetMode) {
    case kTargetMode2D:
        views[0].mWorldXfm = xfm;
        mPrimaryView.mWorldXfm = views[0].mWorldXfm;
        break;
    case kTargetModeStereo:
        views[0].mWorldXfm = settings.mUseEyes ? settings.mEyes[0].mXfm : xfm;
        views[1].mWorldXfm = settings.mUseEyes ? settings.mEyes[1].mXfm : xfm;
        mPrimaryView.mWorldXfm = xfm;
        break;
    case kTargetModeCube:
        for (int face = 0; face < 6; ++face) {
            views[face].mWorldXfm =
                _CalcCubeFaceWorldXfm(xfm, static_cast<RndTextureCubeFace>(face));
        }
        mPrimaryView.mWorldXfm = views[4].mWorldXfm;
        break;
    case kTargetModeLeftEye:
    case kTargetModeRightEye:
        views[0].mWorldXfm =
            settings.mUseEyes ? settings.mEyes[mTargetMode - kTargetModeLeftEye].mXfm : xfm;
        mPrimaryView.mWorldXfm = views[0].mWorldXfm;
        break;
    default:
        if (IsCubeFaceTargetMode(mTargetMode)) {
            views[0].mWorldXfm = _CalcCubeFaceWorldXfm(
                xfm, static_cast<RndTextureCubeFace>(mTargetMode - kTargetModeCubeFace));
        }
        mPrimaryView.mWorldXfm = views[0].mWorldXfm;
        break;
    }

    for (unsigned long i = 0; i <= mViews.mSize; ++i) {
        auto& view = i == 0 ? mPrimaryView : mViews.mData[i - 1];
        Invert(view.mWorldXfm, view.mInvWorldXfm);
        Transpose(view.mWorldXfm.m, view.mWorldRotationTranspose);
    }
}

// Inlined into _CalcWorldXfms at 0x3DA010 and 0x3DA4F5. Faces 0 and 1 turn
// about the up axis, 2 and 3 tilt up and down, 4 keeps the camera's
// orientation and 5 turns around. The translation is kept; a face past the
// six gives the identity.
Transform RndCameraContext::_CalcCubeFaceWorldXfm(
    const Transform& xfm,
    RndTextureCubeFace face) {
    const Vector3& x = xfm.m.x;
    const Vector3& y = xfm.m.y;
    const Vector3& z = xfm.m.z;
    const Vector3 negX = {-x.x, -x.y, -x.z};
    const Vector3 negY = {-y.x, -y.y, -y.z};
    const Vector3 negZ = {-z.x, -z.y, -z.z};
    switch (static_cast<int>(face)) {
    case 0:
        return {{negY, x, z}, xfm.v};
    case 1:
        return {{y, negX, z}, xfm.v};
    case 2:
        return {{x, z, negY}, xfm.v};
    case 3:
        return {{x, negZ, y}, xfm.v};
    case 4:
        return {{x, y, z}, xfm.v};
    case 5:
        return {{negX, negY, z}, xfm.v};
    default:
        return Transform::sID;
    }
}

// Reconstructed from eboot.elf at 0x3DA8D0. The view transform turns the
// inverse world transform's z-up space into y-up view space; the binary
// builds the axis swap under a static guard.
void RndCameraContext::_CalcViewXfms() {
    static const Transform sViewAxes = {
        {{1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F, 0.0F}},
        {0.0F, 0.0F, 0.0F},
    };
    for (unsigned long i = 0; i <= mViews.mSize; ++i) {
        auto& view = i == 0 ? mPrimaryView : mViews.mData[i - 1];
        Transform viewXfm;
        Multiply(view.mInvWorldXfm, sViewAxes, viewXfm);
        view.mViewXfm = viewXfm;
    }
}

// Reconstructed from eboot.elf at 0x3D9750.
void RndCameraContext::SetViewProjectionShaderConstants(RndShaderCBuffer& cbuffer) const {
    if (mViews.mSize == 0) {
        return;
    }
    auto* slices = CBufferFloats(cbuffer, TheRndDevice()->mShaderMgr.mCameraRTSlicedData);
    for (unsigned long i = 0; i < mViews.mSize; ++i) {
        StoreTransposed(slices + i * kSliceFloats, mViews.mData[i].mViewProjection);
    }
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x3D9810. Perspective cameras write the
// far corners' extents in the second half of each element; orthographic
// cameras repeat them.
void RndCameraContext::SetOtherShaderConstants(RndShaderCBuffer& cbuffer) const {
    const auto& shaders = TheRndDevice()->mShaderMgr;
    const auto& settings = GetCameraSettings();
    auto* nearFar = CBufferFloats(cbuffer, shaders.mCameraNearFarParams);
    nearFar[0] = settings.mNearPlane;
    nearFar[1] = settings.mFarPlane;
    nearFar[2] = settings.mFarPlane * settings.mNearPlane;
    nearFar[3] = settings.mFarPlane - settings.mNearPlane;

    auto* misc = CBufferFloats(cbuffer, shaders.mCameraMiscParams);
    const float depthScale = 1.0F / (mDepthRange.y - mDepthRange.x);
    misc[0] = settings.mOrthographic ? 1.0F : 0.0F;
    misc[1] = 0.0F;
    misc[2] = depthScale;
    misc[3] = -mDepthRange.x * depthScale;
    cbuffer.mSyncPending = true;

    float extents[16] = {};
    const auto* corners = mPrimaryView.mViewFrustum.mCorners.mData;
    for (unsigned long i = 0; i < 4; ++i) {
        const auto& corner = corners[4 + i];
        auto* extent = extents + i * 4;
        if (settings.mOrthographic) {
            extent[0] = corner.x;
            extent[1] = corner.z;
        }
        extent[2] = corner.x;
        extent[3] = corner.z;
    }
    std::memcpy(CBufferFloats(cbuffer, shaders.mCameraViewExtents), extents, sizeof(extents));
    cbuffer.mSyncPending = true;

    if (mViews.mSize == 0) {
        return;
    }
    auto* slices = CBufferFloats(cbuffer, shaders.mCameraRTSlicedData);
    for (unsigned long i = 0; i < mViews.mSize; ++i) {
        auto* slice = slices + i * kSliceFloats + kSliceXfmsOffset;
        StoreTransposed(slice, mViews.mData[i].mWorldXfm);
        StoreTransposed(slice + 12, mViews.mData[i].mInvWorldXfm);
    }
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x3D9AE0.
void RndCameraContext::SetStereoViewProjectionShaderConstants(RndShaderCBuffer&) const {}

// Reconstructed from eboot.elf at 0x3D9AF0.
void RndCameraContext::SetStereoOtherShaderConstants(RndShaderCBuffer&) const {}

// Reconstructed from eboot.elf at 0x3D9B00.
void RndCameraContext::SetIdentityViewProjectionShaderConstants(
    RndTargetMode mode,
    RndShaderCBuffer& cbuffer) {
    const unsigned long numSlices = RndTargetModeSlices(mode);
    if (numSlices == 0) {
        return;
    }
    auto* slices = CBufferFloats(cbuffer, TheRndDevice()->mShaderMgr.mCameraRTSlicedData);
    for (unsigned long i = 0; i < numSlices; ++i) {
        StoreTransposed(slices + i * kSliceFloats, Hmx::Matrix4::sID);
    }
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x3D9BC0. Identity view-projection and
// transforms, a zero-to-one depth range, and unit extents.
void RndCameraContext::SetDefaultShaderConstants(RndTargetMode mode, RndShaderCBuffer& cbuffer) {
    const auto& shaders = TheRndDevice()->mShaderMgr;
    const unsigned long numSlices = RndTargetModeSlices(mode);
    auto* slices = CBufferFloats(cbuffer, shaders.mCameraRTSlicedData);
    if (numSlices != 0) {
        for (unsigned long i = 0; i < numSlices; ++i) {
            StoreTransposed(slices + i * kSliceFloats, Hmx::Matrix4::sID);
        }
        cbuffer.mSyncPending = true;
    }

    static const float kNearFar[] = {0.0F, 1.0F, 0.0F, 1.0F};
    static const float kViewExtents[] = {0.0F, 0.0F, 1.0F, 1.0F};
    static const float kMisc[] = {1.0F, 0.0F, 1.0F, 0.0F};
    std::memcpy(CBufferFloats(cbuffer, shaders.mCameraNearFarParams), kNearFar, sizeof(kNearFar));
    std::memcpy(
        CBufferFloats(cbuffer, shaders.mCameraViewExtents), kViewExtents, sizeof(kViewExtents));
    std::memcpy(CBufferFloats(cbuffer, shaders.mCameraMiscParams), kMisc, sizeof(kMisc));
    cbuffer.mSyncPending = true;

    if (numSlices != 0) {
        for (unsigned long i = 0; i < numSlices; ++i) {
            auto* slice = slices + i * kSliceFloats + kSliceXfmsOffset;
            StoreTransposed(slice, Transform::sID);
            StoreTransposed(slice + 12, Transform::sID);
        }
        cbuffer.mSyncPending = true;
    }
}

// Inlined. The projection rectangle's share of the viewport, width over
// height.
float RndCameraContext::_CalcAspectRatio() const {
    return (mProjectionRect.w * mViewportSize.x) / (mProjectionRect.h * mViewportSize.y);
}

// Inlined.
float RndCameraContext::_GetPerspectiveYFov() const {
    return GetCameraSettings().mYFov;
}

// Reconstructed from eboot.elf at 0x3DAA40. The projection rectangle maps to
// [left, right] x [bottom, top] in clip space, and depth runs from one at the
// near plane to zero at the far plane. Cube targets need a square aspect
// ratio and a perspective camera; the context is invalid otherwise.
void RndCameraContext::_CalcProjectionMatrices() {
    const RndCameraSettings& settings = GetCameraSettings();
    const RndTargetMode mode = mTargetMode;
    const float nearPlane = settings.mNearPlane;
    const float farPlane = settings.mFarPlane;
    const float aspect = _CalcAspectRatio();
    if (aspect != 1.0F && IsCubeTargetMode(mode)) {
        mValid = false;
    }

    const float invWidth = 1.0F / mViewportSize.x;
    const float invHeight = 1.0F / mViewportSize.y;
    const float left = mProjectionRect.x * (2.0F * mViewportSize.x) * invWidth - 1.0F;
    const float right =
        (mProjectionRect.w + mProjectionRect.x) * (2.0F * mViewportSize.x) * invWidth - 1.0F;
    const float bottom = mProjectionRect.y * (2.0F * mViewportSize.y) * invHeight - 1.0F;
    const float top =
        (mProjectionRect.h + mProjectionRect.y) * (2.0F * mViewportSize.y) * invHeight - 1.0F;
    const float width = right - left;
    const float height = top - bottom;
    const float halfWidth = width * 0.5F;
    const float halfHeight = height * 0.5F;
    const float centerX = halfWidth + left;
    const float centerY = halfHeight + bottom;
    const float invRange = 1.0F / (farPlane - nearPlane);

    for (unsigned long i = 0; i <= mViews.mSize; ++i) {
        auto& view = i == 0 ? mPrimaryView : mViews.mData[i - 1];
        Hmx::Matrix4& projection = view.mProjection;
        if (GetCameraSettings().mOrthographic) {
            if (IsCubeTargetMode(mode)) {
                mValid = false;
            }
            const float scaleY = height / GetCameraSettings().mOrthoHeight;
            projection.x = {scaleY / aspect, 0.0F, 0.0F, 0.0F};
            projection.y = {0.0F, scaleY, 0.0F, 0.0F};
            projection.z = {0.0F, 0.0F, -invRange, 0.0F};
            projection.w = {centerX, centerY, invRange * farPlane, 1.0F};
        } else {
            const Frustum::Fov fov = CalcPerspectiveFov(*this, static_cast<long>(i) - 1);
            const float tanLeft = std::tan(fov.mLeft);
            const float tanRight = std::tan(fov.mRight);
            const float tanUp = std::tan(fov.mUp);
            const float tanDown = std::tan(fov.mDown);
            const float invX = 1.0F / (tanRight + tanLeft);
            const float invY = 1.0F / (tanDown + tanUp);
            projection.x = {invX * width, 0.0F, 0.0F, 0.0F};
            projection.y = {0.0F, height * invY, 0.0F, 0.0F};
            projection.z = {
                centerX + (tanLeft - tanRight) * invX * halfWidth,
                centerY + (tanDown - tanUp) * invY * halfHeight,
                -(invRange * nearPlane),
                1.0F,
            };
            projection.w = {0.0F, 0.0F, nearPlane * farPlane * invRange, 0.0F};
        }
    }

    for (unsigned long i = 0; i <= mViews.mSize; ++i) {
        auto& view = i == 0 ? mPrimaryView : mViews.mData[i - 1];
        view.mViewProjection = view.mViewXfm * view.mProjection;
        const float det = std::fabs(Det(view.mViewProjection));
        Invert(view.mViewProjection, view.mInvViewProjection, det > 0.0F ? det * 0.5F : 0.0F);
    }
}

// Reconstructed from eboot.elf at 0x3DB020. The binary copies the slices'
// frusta for the target mode's slice count and skips modes past the table,
// including kTargetModeNone, which _SyncDerived never passes here.
void RndCameraContext::_CalcPrimaryFrusta() {
    Frustum sliceViewFrusta[6];
    Frustum sliceWorldFrusta[6];
    const RndCameraSettings& settings = GetCameraSettings();
    _CalcFrusta(
        settings.mNearPlane,
        settings.mFarPlane,
        mPrimaryView.mViewFrustum,
        mPrimaryView.mWorldFrustum,
        sliceViewFrusta,
        sliceWorldFrusta,
        mFrustum);
    const unsigned long numSlices = RndTargetModeSlices(mTargetMode);
    for (unsigned long i = 0; i < numSlices; ++i) {
        mViews.mData[i].mViewFrustum = sliceViewFrusta[i];
        mViews.mData[i].mWorldFrustum = sliceWorldFrusta[i];
    }
}

// Reconstructed from eboot.elf at 0x3DBFD0. A single slice shares the
// target's frusta. With several, a cube's bounding frustum runs from the
// back face's far corners to the front face's, and a stereo pair's takes the
// left eye's even corners and the right eye's odd ones.
void RndCameraContext::_CalcFrusta(
    float nearPlane,
    float farPlane,
    Frustum& viewFrustum,
    Frustum& worldFrustum,
    Frustum* sliceViewFrusta,
    Frustum* sliceWorldFrusta,
    Frustum& combinedFrustum) const {
    const float aspect = _CalcAspectRatio();
    if (GetCameraSettings().mOrthographic) {
        viewFrustum.SetOrtho(nearPlane, farPlane, GetCameraSettings().mOrthoHeight, aspect);
    } else {
        viewFrustum.SetPerspective(nearPlane, farPlane, CalcPerspectiveFov(*this, -1));
    }

    const unsigned long numSlices = mViews.mSize;
    if (numSlices == 1) {
        sliceViewFrusta[0] = viewFrustum;
    } else {
        for (unsigned long i = 0; i < numSlices; ++i) {
            if (GetCameraSettings().mOrthographic) {
                sliceViewFrusta[i].SetOrtho(
                    nearPlane, farPlane, GetCameraSettings().mOrthoHeight, aspect);
            } else {
                sliceViewFrusta[i].SetPerspective(
                    nearPlane, farPlane, CalcPerspectiveFov(*this, static_cast<long>(i)));
            }
        }
    }

    worldFrustum = TransformFrustum(viewFrustum, mPrimaryView.mWorldXfm);
    if (numSlices == 1) {
        sliceWorldFrusta[0] = worldFrustum;
    } else {
        for (unsigned long i = 0; i < numSlices; ++i) {
            sliceWorldFrusta[i] =
                TransformFrustum(sliceViewFrusta[i], mViews.mData[i].mWorldXfm);
        }
    }

    if (numSlices == 1) {
        combinedFrustum = worldFrustum;
    } else if (mTargetMode == kTargetModeCube) {
        const Vector3* back = sliceWorldFrusta[5].mCorners.mData;
        const Vector3* front = sliceWorldFrusta[4].mCorners.mData;
        Vector3 corners[8] = {};
        corners[0] = back[5];
        corners[1] = back[4];
        corners[2] = back[7];
        corners[3] = back[6];
        for (unsigned long i = 4; i < 8; ++i) {
            corners[i] = front[i];
        }
        combinedFrustum.SetCorners(corners);
    } else if (mTargetMode == kTargetModeStereo) {
        Vector3 corners[8] = {};
        for (unsigned long i = 0; i < 8; ++i) {
            corners[i] = sliceWorldFrusta[i & 1].mCorners.mData[i];
        }
        combinedFrustum.SetCorners(corners);
    }
}

// Reconstructed from eboot.elf at 0x3D9190.
Vector2 RndCameraContext::Project(const Vector3& point, Rnd2DCoord coord, float* depth) const {
    const Hmx::Matrix4& m = mPrimaryView.mViewProjection;
    const float z = point.x * m.x.z + point.y * m.y.z + point.z * m.z.z + m.w.z;
    const float w = point.x * m.x.w + point.y * m.y.w + point.z * m.z.w + m.w.w;
    Vector2 ndc = Vector2::sZero;
    if (w != 0.0F) {
        ndc.x = (point.x * m.x.x + point.y * m.y.x + (point.z * m.z.x + m.w.x)) / w;
        ndc.y = (point.x * m.x.y + point.y * m.y.y + (point.z * m.z.y + m.w.y)) / w;
    }
    if (depth != nullptr) {
        *depth = w != 0.0F ? z / w : 0.0F;
    }
    return NDCTo2DCoord(ndc, mViewportSize, coord);
}

// Reconstructed from eboot.elf at 0x3D9350. The depth is a view-space
// distance: the projection gives its clip-space z and w, and the inverse
// view-projection takes the clip point back to the world without a divide.
Vector3 RndCameraContext::Unproject(const Vector2& point, float depth, Rnd2DCoord coord) const {
    const Vector2& viewport = mViewportSize;
    Vector2 pixels;
    switch (coord) {
    case k2DCoordPixels:
        pixels = point;
        break;
    case k2DCoordNormalized:
        pixels = {point.x * viewport.x, point.y * viewport.y};
        break;
    case k2DCoordHeightNormalized:
        pixels = {
            viewport.y * point.x + (viewport.x - viewport.y) * 0.5F,
            viewport.y * point.y,
        };
        break;
    case k2DCoordNDC:
        pixels = {
            viewport.x * 0.5F * (point.x + 1.0F),
            viewport.y * 0.5F * (point.y + 1.0F),
        };
        break;
    default:
        pixels = {0.0F, 0.0F};
        break;
    }
    const float ndcX = (pixels.x + pixels.x) / viewport.x - 1.0F;
    const float ndcY = (pixels.y + pixels.y) / viewport.y - 1.0F;

    const Hmx::Matrix4& projection = mPrimaryView.mProjection;
    const float clipZ = depth * projection.z.z + projection.w.z;
    const float clipW = depth * projection.z.w + projection.w.w;
    const float clipX = clipW * ndcX;
    const float clipY = clipW * ndcY;
    const Hmx::Matrix4& inverse = mPrimaryView.mInvViewProjection;
    return {
        clipX * inverse.x.x + clipW * inverse.w.x + (clipY * inverse.y.x + clipZ * inverse.z.x),
        clipY * inverse.y.y + clipZ * inverse.z.y + (clipX * inverse.x.y + clipW * inverse.w.y),
        clipY * inverse.y.z + clipZ * inverse.z.z + (clipX * inverse.x.z + clipW * inverse.w.z),
    };
}

// Reconstructed from eboot.elf at 0x3D94D0.
Segment RndCameraContext::UnprojectSegment(const Vector2& point, Rnd2DCoord coord) const {
    const RndCameraSettings& settings = GetCameraSettings();
    Segment segment;
    segment.start = Unproject(point, settings.mNearPlane, coord);
    segment.end = Unproject(point, settings.mFarPlane, coord);
    return segment;
}

// Reconstructed from eboot.elf at 0x3D9560. A perspective sphere whose
// center is not in front of the camera projects 1e30 clip units high.
float RndCameraContext::CalcProjectedHeight(const Sphere& sphere, Rnd2DCoord coord) const {
    float height = mPrimaryView.mProjection.y.y * sphere.radius;
    if (!GetCameraSettings().mOrthographic) {
        const Transform& viewXfm = mPrimaryView.mViewXfm;
        const Vector3& center = sphere.center;
        const float z = center.y * viewXfm.m.y.z + center.x * viewXfm.m.x.z +
            center.z * viewXfm.m.z.z + viewXfm.v.z;
        if (z > 0.0F) {
            height = height / z;
        } else {
            height = kBehindCameraHeight;
        }
    }
    const Vector2 bottom = NDCTo2DCoord(Vector2::sZero, mViewportSize, coord);
    const Vector2 top = NDCTo2DCoord({0.0F, height}, mViewportSize, coord);
    return (top.y - bottom.y) * 2.0F;
}
