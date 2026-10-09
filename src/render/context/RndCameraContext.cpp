#include "render/context/RndCameraContext.h"

#include <cstring>

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
    mUnknown408.mSize = 0;
    if (mLodSettingsSet) {
        _SyncLodData();
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
