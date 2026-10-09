// render/RndMaterialRuntimeData.o (0x4F71E0 to 0x4F9A00).
#include "render/materials/RndMaterialRuntimeData.h"

#include "render/buffers/RndShaderCBuffer.h"
#include "render/shaders/RndShaderError.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/shadergraph/RndShaderGraph.h"
#include "render/shadergraph/RndShaderGraphResource.h"
#include "render/system/RndDevice.h"

// Reconstructed from eboot.elf at 0x4F71E0.
RndMaterialRuntimeData* RndMaterialRuntimeData::New(RndShaderGraphResource* graph) {
    return new RndMaterialRuntimeData(graph);
}

// Reconstructed from eboot.elf at 0x4F7210. Data the GPU may still read
// this frame is freed by the device at the frame boundary.
void RndMaterialRuntimeData::Delete(RndMaterialRuntimeData* data) {
    if (gRndDevice != nullptr) {
        if (data->mFrameStamp == gRndDevice->mFrameCount) {
            gRndDevice->SyncFreeMaterialData(data);
            return;
        }
    } else if (data == nullptr) {
        return;
    }
    delete data;
}

// Reconstructed from eboot.elf at 0x4F72A0. mBucket is left unset.
RndMaterialRuntimeData::RndMaterialRuntimeData(RndShaderGraphResource* graph)
    : mFrameStamp(~0UL),
      mShaderGraph(graph),
      mGraph(graph != nullptr ? graph->GetShaderGraph() : nullptr),
      mRootFlags{false, false, false},
      mBlendMode(-1),
      mBlendFactor(Hmx::Color::GetWhite()),
      mDepthPrepass(false),
      mForceOpaque(false),
      mSceneMask(false),
      mUsageHints{0, 0, 0},
      mCBuffer(nullptr),
      mVolumetricBlendParams(~0UL),
      mExposedProps(~0UL),
      mSyncFlags(0) {
    _CreateCBuffer();
    _CacheGraphFlags();
}

// Reconstructed from eboot.elf at 0x4F7530.
void RndMaterialRuntimeData::_CacheGraphFlags() {
    if (mGraph == nullptr) {
        mRootFlags[0] = false;
        mRootFlags[1] = false;
        mRootFlags[2] = false;
        return;
    }
    mRootFlags[0] = mGraph->IsLit();
    mRootFlags[1] = mGraph->UsesSceneTex();
    mRootFlags[2] = mGraph->UsesSceneDepth();
}

// Reconstructed from eboot.elf at 0x4F7580. Deletes the constant buffer;
// the members then release their resource references.
RndMaterialRuntimeData::~RndMaterialRuntimeData() {
    RndShaderCBuffer::SafeDelete(mCBuffer);
}

// Reconstructed from eboot.elf at 0x4F7C00.
void RndMaterialRuntimeData::SyncCBufferStatic() {
    RndShaderCBuffer* buffer = mCBuffer;
    if (buffer != nullptr && buffer->mSyncPending) {
        buffer->_CreateImpl();
        buffer->mSyncPending = false;
    }
}

// Reconstructed from eboot.elf at 0x4F7C30.
void RndMaterialRuntimeData::SyncCBufferGlobals(RndContext& context) {
    RndShaderCBuffer* buffer = mCBuffer;
    if (buffer != nullptr && buffer->mSyncPending) {
        buffer->_SyncImpl(context, 0, buffer->mNumElements);
        buffer->mSyncPending = false;
    }
}

// Reconstructed from eboot.elf at 0x4F7880. gVolumetricBlendParams holds
// two per-blend-mode weights for modes 1-10, as (a, a, a, b); other modes
// write zeros. The weights' meaning is not recovered.
void RndMaterialRuntimeData::SyncCBufferMaterialParams() {
    // The tables at 0x128C9D0 and 0x128C9A0.
    static const float kWeightA[10] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 1};
    static const float kWeightB[10] = {1, 0, 1, 0, 0, 1, 1, 1, 1, 1};
    if (mGraph == nullptr || mCBuffer == nullptr) {
        return;
    }
    float a = 0.0F;
    float b = 0.0F;
    const auto index = static_cast<unsigned int>(mBlendMode - 1);
    if (index < 10) {
        a = kWeightA[index];
        b = kWeightB[index];
    }
    auto* params = static_cast<float*>(mCBuffer->mData) +
        mVolumetricBlendParams * 4;
    params[0] = a;
    params[1] = a;
    params[2] = a;
    params[3] = b;
    mCBuffer->mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x4F8D10. Without a graph, or when the
// graph has no program for the geometry type, the error shader draws.
void RndMaterialRuntimeData::SelectShader(
    RndContext& context,
    const RndSceneBatchContext& batch,
    RndShaderGeoType geoType) {
    if (mCBuffer != nullptr) {
        mCBuffer->_SelectImpl(context);
    }
    if (mGraph != nullptr && mGraph->Select(context, batch, *this, geoType)) {
        return;
    }
    gRndDevice->mShaderMgr.mErrorShader->Select(context, geoType);
}
