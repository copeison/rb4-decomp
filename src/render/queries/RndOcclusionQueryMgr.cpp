// render/RndOcclusionQueryMgr.o (0x5F7EC0 to 0x5F8D60).
#include "render/queries/RndOcclusionQueryMgr.h"

#include <cmath>
#include <cstring>

#include "math/matrix/Matrix3.h"
#include "math/scalar/Trig.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector4.h"
#include "render/buffers/RndCShaderClearBuffer.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/materials/RndMaterialCom.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/queries/RndShaderDisplayOcclusionQueryCoverage.h"
#include "render/queries/RndShaderOcclusionQuery.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "utl/containers/FixedVector.h"

namespace {

// Names not in the reference map.
RndShaderDisplayOcclusionQueryCoverage* gDisplayCoverageShader = nullptr;  // 0x1AA7C20
RndMesh* gOcclusionSphere = nullptr;                                       // 0x1AA7C28
RndShaderOcclusionQuery* gOcclusionQueryShader = nullptr;                  // 0x1AA7C30

// The occlusion sphere's segments, as Init builds it: the area of its
// octagonal silhouette is this factor times the square of the projected
// radius. Name not in the reference map.
const float kOctagonSegmentAngle = 0.39269909F;   // pi / 8
const float kOctagonCornerAngle = 1.9634955F;     // 5 pi / 8
const float kOctagonSegments = 8.0F;

// Creates a shader and registers it with the shader manager. Name not in
// the reference map.
template <class T>
T* CreateShader() {
    auto* shader = new T;
    shader->_Register();
    return shader;
}

// Name not in the reference map.
template <class T>
void SafeDelete(T*& object) {
    delete object;
    object = nullptr;
}

// Unbinds the source textures of the stages and forgets their slot
// limits, as the binary inlines it. Name not in the reference map.
void DeselectSourceTextures(RndContext& context, unsigned int stages) {
    context._DeselectAllSourceTexturesImpl(stages);
    for (unsigned int stage = 0; stage < kNumShaderProgramTypes; ++stage) {
        if ((stages & (1U << stage)) != 0) {
            context.mOutputSlotLimits[stage] = 0;
        }
    }
}

// The smallest per-draw constant buffer (16, 32 or 64 elements) that holds
// the given number of elements. Name not in the reference map.
RndShaderCBuffer* TransientCBuffer(RndContext& context, unsigned long size) {
    unsigned long index = 0;
    if (size > 16) {
        for (unsigned long capacity = 16; capacity < size; capacity *= 2) {
            ++index;
        }
    }
    return context.mCBuffers[6 + index];
}

}  // namespace

// Reconstructed from eboot.elf at 0x5F7ED0.
void RndOcclusionQueryMgr::Init() {
    RndMeshUtl::CreateSphereParams params;
    params.mName = "Occlusion Query Sphere";
    params.mVertexType = kVertexPosOnly;
    params.mNumSegments = 8;
    params.mRadius = 1.0F;
    params.mNumRings = 4;
    gOcclusionSphere = RndMeshUtl::CreateSphere(params);
    gOcclusionQueryShader = CreateShader<RndShaderOcclusionQuery>();
    gDisplayCoverageShader = CreateShader<RndShaderDisplayOcclusionQueryCoverage>();
}

// Reconstructed from eboot.elf at 0x5F7FB0.
void RndOcclusionQueryMgr::Terminate() {
    SafeDelete(gOcclusionSphere);
    SafeDelete(gOcclusionQueryShader);
    SafeDelete(gDisplayCoverageShader);
}

// Reconstructed from eboot.elf at 0x5F7EC0.
RndShaderDisplayOcclusionQueryCoverage* RndOcclusionQueryMgr::GetDisplayCoverageShader() {
    return gDisplayCoverageShader;
}

// Reconstructed from eboot.elf at 0x5F8010.
RndOcclusionQueryMgr::RndOcclusionQueryMgr()
    : mNumQueries(0), mQueries(nullptr), mQueryList(nullptr) {
    mQueryList = new RndOcclusionQuery::Link;
    mQueryList->mPrev = mQueryList;
    mQueryList->mNext = mQueryList;
}

// Reconstructed from eboot.elf at 0x5F8080. The queries stay linked to
// each other; their owners delete them.
RndOcclusionQueryMgr::~RndOcclusionQueryMgr() {
    delete mQueries;
    mQueries = nullptr;
    RndOcclusionQuery::Link* list = mQueryList;
    if (list != nullptr) {
        list->mNext->mPrev = list->mPrev;
        list->mPrev->mNext = list->mNext;
        delete list;
    }
    mQueryList = nullptr;
}

// Reconstructed from eboot.elf at 0x5F8110. The query goes at the end of
// the list.
void RndOcclusionQueryMgr::RegisterQuery(RndOcclusionQuery& query) {
    query.mIndex = mNumQueries++;
    RndOcclusionQuery::Link* list = mQueryList;
    RndOcclusionQuery::Link* last = list->mPrev;
    query.mLink.mPrev = last;
    query.mLink.mNext = list;
    last->mNext = &query.mLink;
    list->mPrev = &query.mLink;
}

// Reconstructed from eboot.elf at 0x5F8140.
void RndOcclusionQueryMgr::PostEnter() {
    if (mQueryList->mNext == mQueryList) {
        return;
    }
    if (mQueries == nullptr || mQueries->mDesc.mNumElements != mNumQueries) {
        _AllocCoverageBuffer();
    }
}

// Reconstructed from eboot.elf at 0x5F81E0; PostEnter and Poll inline it.
void RndOcclusionQueryMgr::_AllocCoverageBuffer() {
    delete mQueries;
    mQueries = nullptr;
    RndComputeBuffer::Description desc{};
    desc.mElementSize = 4;
    desc.mNumElements = mNumQueries;
    desc.mFlags = 1;
    desc.mName = "Occlusion Query Coverage";
    mQueries = RndComputeBuffer::New(desc);
}

// Reconstructed from eboot.elf at 0x5F8270.
void RndOcclusionQueryMgr::Poll() {
    if (mQueryList->mNext == mQueryList) {
        return;
    }
    if (mQueries == nullptr || mQueries->mDesc.mNumElements < mNumQueries) {
        _AllocCoverageBuffer();
    }
}

// Reconstructed from eboot.elf at 0x5F8310. A query outside the frustum is
// marked culled for the view; a visible one expects the pixels of the
// sphere's octagonal silhouette, rounded up.
void RndOcclusionQueryMgr::Submit(const RndCameraContext& camera) {
    const float octagonArea = Sine(kOctagonSegmentAngle) * kOctagonSegments *
        Sine(kOctagonCornerAngle);
    const long view = GetViewIndex(camera.mTargetMode);
    eastl::vector<RndOcclusionQuery*>& visible = mViewQueries[view];
    visible.clear();
    for (RndOcclusionQuery::Link* link = mQueryList->mNext; link != mQueryList;
         link = link->mNext) {
        RndOcclusionQuery* query = reinterpret_cast<RndOcclusionQuery*>(
            reinterpret_cast<char*>(link) - offsetof(RndOcclusionQuery, mLink));
        if (query->mEnabled &&
            camera.mFrustum.ClassifySphere(query->mSphere) !=
                ConvexHullBase::kOutside) {
            const float radius =
                camera.CalcProjectedHeight(query->mSphere, k2DCoordPixels) *
                0.5F;
            query->mPixelCount = static_cast<unsigned int>(
                std::ceil(radius * radius * octagonArea));
            query->mCulled[view] = false;
            visible.push_back(query);
        } else {
            query->mCulled[view] = true;
        }
    }
}

// Reconstructed from eboot.elf at 0x5F8590.
long RndOcclusionQueryMgr::GetViewIndex(RndTargetMode mode) {
    if (mode == kTargetMode2D) {
        return 0;
    }
    if (static_cast<unsigned int>(mode - kTargetModeLeftEye) <= 1) {
        return mode - kTargetModeLeftEye;
    }
    if (static_cast<unsigned int>(mode - kTargetModeCubeFace) <= 5) {
        return 0;
    }
    return -1;
}

// Reconstructed from eboot.elf at 0x5F85C0. The coverage buffer is cleared
// as an unordered-access buffer, written by the pixel shader of each
// query's sphere inside the query, and handed back to the shaders. The
// depth target's +160 field (RndTextureBase::mResourceIndex) is read as its
// target mode.
void RndOcclusionQueryMgr::DrawQueries(
    RndContext& context,
    RndTextureBase& depth) {
    const long view =
        GetViewIndex(static_cast<RndTargetMode>(depth.mResourceIndex));
    eastl::vector<RndOcclusionQuery*>& visible = mViewQueries[view];
    if (visible.mpBegin == visible.mpEnd) {
        return;
    }
    static Symbol sName;
    if (sName == Symbol()) {
        sName = Symbol("Occlusion Queries");
    }
    RndScopedGpuStatBlock statBlock(context, sName.Str());

    FixedVector<RndResourceBarrier, 1> barriers;
    RndResourceBarrier barrier;
    barrier.mResource = mQueries;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = RndResourceState::kPixelShaderResource;
    barrier.mAfter = RndResourceState::kUnorderedAccess;
    barriers.push_back(barrier);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndCShaderClearBuffer::Params clear;
    clear.mBuffer = mQueries;
    clear.mNumericType = kShaderNumericUInt;
    gRndDevice->mShaderMgr.mClearBufferCShader->Dispatch(context, clear);
    DeselectSourceTextures(context, (1U << kNumShaderProgramTypes) - 1);

    barriers[0].mType = RndResourceBarrierType::kUnorderedAccess;
    barriers[0].mPhase = RndResourceBarrierPhase::kImmediate;
    barriers[0].mResource = mQueries;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    context.SetRenderTargets(nullptr, &depth);
    context.mBlendMode = RndBlendMode::kDestination;
    context._SetBlendModeImpl(
        RndBlendMode::kDestination, Hmx::Color::GetWhite());
    context._SetDepthModeImpl(3);
    context._SetStencilModeImpl(0, 0, 0, 0);
    context._SetCullModeImpl(kCullBack);
    gOcclusionQueryShader->Select(context, *mQueries);

    RndShaderCBuffer* buffer =
        TransientCBuffer(context, gOcclusionQueryShader->mCBufferSize);
    for (RndOcclusionQuery* query : visible) {
        auto* id = static_cast<float*>(buffer->mData) +
            gOcclusionQueryShader->mCoverageId * 4;
        *id = static_cast<float>(query->mIndex);
        buffer->mSyncPending = true;
        buffer->_SyncImpl(context, 0, gOcclusionQueryShader->mCBufferSize);
        buffer->mSyncPending = false;
        buffer->_SelectImpl(context);
        query->_BeginQueryImpl(context);
        DrawOcclusionSphere(context, query->mSphere);
        query->_EndQueryImpl(context);
    }

    DeselectSourceTextures(context, 1U << kShaderProgramPixel);
    barriers[0].mType = RndResourceBarrierType::kTransition;
    barriers[0].mPhase = RndResourceBarrierPhase::kImmediate;
    barriers[0].mResource = mQueries;
    barriers[0].mSubresource = ~0UL;
    barriers[0].mBefore = RndResourceState::kUnorderedAccess;
    barriers[0].mAfter = RndResourceState::kPixelShaderResource;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
}

// Reconstructed from eboot.elf at 0x5F8BF0. The sphere's transform is the
// camera's rotation scaled by the radius, so the mesh's silhouette faces
// the camera; the packed state is left unset.
void RndOcclusionQueryMgr::DrawOcclusionSphere(
    RndContext& context,
    const Sphere& sphere) {
    const Hmx::Matrix3& rotation = context.mCameras[0].mViews[0].mWorldXfm.m;
    const float* axes = &rotation.x.x;
    const float* center = &sphere.center.x;
    const float* identity = &Hmx::Matrix3::sID.x.x;
    RndInstanceData instance;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            instance.mXfm[i][j] = sphere.radius * axes[j * 3 + i];
            instance.mNormalXfm[i][j] = identity[j * 3 + i];
        }
        instance.mXfm[i][3] = center[i];
    }
    for (float* params : instance.mParams) {
        std::memcpy(params, &Vector4::sZero, sizeof(Vector4));
    }
    const VectorAdapter<RndInstanceData> instances = {&instance, 1};
    const RndDrawable::DrawRange range = {
        0, RndDrawable::DrawRange::kAllFaces};
    gOcclusionSphere->_DrawBatchImpl(context, instances, range);
}
