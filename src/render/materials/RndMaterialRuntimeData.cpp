// render/RndMaterialRuntimeData.o (0x4F71E0 to 0x4F998F). LoadExposedTextures
// (0x4F7C60), its per-texture loader (0x4F82D0) and the texture-resource
// loads it instantiates (0x4F8FD0-0x4F97EE) are not reconstructed. The
// static initializer (0x4F9970) sets three ints of a shared header
// (0x1A8C088), which are not modelled.
#include "render/materials/RndMaterialRuntimeData.h"

#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/shaders/RndShaderCBufferConfig.h"
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

// Reconstructed from eboot.elf at 0x4F7470. The layout is built in
// temporary memory.
void RndMaterialRuntimeData::_CreateCBuffer() {
    if (mGraph == nullptr) {
        return;
    }
    RndShaderCBufferConfig config;
    unsigned int temp;
    MemPushTemp(temp, true, true);
    mGraph->FillCBufferConfig(config, &mVolumetricBlendParams, &mExposedProps);
    MemPopTemp(temp);
    if (config.mSize != 0) {
        mCBuffer = RndShaderCBuffer::New(
            config, RndShaderCBuffer::kDeferInitialSync);
    }
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

// Reconstructed from eboot.elf at 0x4F7720. The graph's layout is measured
// without recording its constants.
bool RndMaterialRuntimeData::IsValid(
    const RndShaderGraphResource* graph) const {
    if (mShaderGraph.Get() != graph) {
        return false;
    }
    if (mGraph == nullptr) {
        return true;
    }
    RndShaderCBufferConfig config;
    config.MarkForMaterialValidation();
    mGraph->FillCBufferConfig(config, nullptr, nullptr);
    const unsigned long size =
        mCBuffer != nullptr ? mCBuffer->mConfigNumElements : 0;
    if (config.mSize != size) {
        return false;
    }
    return mTextures.size() == mGraph->mExposedTextures.size();
}

// Reconstructed from eboot.elf at 0x4F7800.
int RndMaterialRuntimeData::GetSceneTexUsage() const {
    return mGraph != nullptr ? mGraph->mSceneTexUsage : -1;
}

// Reconstructed from eboot.elf at 0x4F7820.
int RndMaterialRuntimeData::GetSceneTexCaptureUsage() const {
    return mGraph != nullptr ? mGraph->mSceneTexCaptureUsage : -1;
}

// Reconstructed from eboot.elf at 0x4F7840.
int RndMaterialRuntimeData::GetSceneDepthUsage() const {
    return mGraph != nullptr ? mGraph->mSceneDepthUsage : -1;
}

// Reconstructed from eboot.elf at 0x4F7860.
bool RndMaterialRuntimeData::UsesLinearDepth() const {
    return mGraph != nullptr && mGraph->mUsesLinearDepth;
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

// Reconstructed from eboot.elf at 0x4F78F0. Each exposed property takes
// the registers of its type, starting at mExposedProps; a bool is widened
// to a word, matrices are transposed into rows, and the integer and 4x3
// types are skipped.
void RndMaterialRuntimeData::SyncCBufferExposedProps(
    const PropArray<unsigned char>& props) {
    // The registers each RndShaderNumericType takes (0x128C910).
    static const unsigned long kRegisterCounts[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 4, 4};
    if (mGraph == nullptr || mCBuffer == nullptr) {
        return;
    }
    unsigned long reg = mExposedProps;
    for (const RndShaderGraph::ExposedPropLayout& layout :
         mGraph->mExposedPropLayout) {
        const unsigned char* source = props.data() + layout.mOffset;
        auto* dest = static_cast<unsigned char*>(mCBuffer->mData) + reg * 16;
        const auto* floats = reinterpret_cast<const float*>(source);
        auto* rows = reinterpret_cast<float*>(dest);
        bool written = true;
        switch (layout.mType) {
        case kShaderNumericBool:
            *reinterpret_cast<unsigned short*>(dest) = *source;
            dest[2] = 0;
            dest[3] = 0;
            break;
        case kShaderNumericFloat:
            std::memcpy(dest, source, 4);
            break;
        case kShaderNumericFloat2:
            std::memcpy(dest, source, 8);
            break;
        case kShaderNumericFloat3:
            std::memcpy(dest + 8, source + 8, 4);
            std::memcpy(dest, source, 8);
            break;
        case kShaderNumericFloat4:
            std::memcpy(dest, source, 16);
            break;
        case kShaderNumericFloat3x3:
            for (int row = 0; row < 3; ++row) {
                for (int column = 0; column < 3; ++column) {
                    rows[row * 4 + column] = floats[column * 3 + row];
                }
            }
            break;
        case kShaderNumericFloat3x4:
            for (int row = 0; row < 3; ++row) {
                for (int column = 0; column < 4; ++column) {
                    rows[row * 4 + column] = floats[column * 3 + row];
                }
            }
            break;
        case kShaderNumericFloat4x4:
            for (int row = 0; row < 4; ++row) {
                for (int column = 0; column < 4; ++column) {
                    rows[row * 4 + column] = floats[column * 4 + row];
                }
            }
            break;
        default:
            written = false;
            break;
        }
        if (written) {
            mCBuffer->mSyncPending = true;
        }
        reg += layout.mType <= kShaderNumericFloat4x4
            ? kRegisterCounts[layout.mType]
            : 0;
    }
}

// Reconstructed from eboot.elf at 0x4F8770.
void RndMaterialRuntimeData::_ClearExposedTextures() {
    mExposedTextures1D.clear();
    mExposedTextures2D.clear();
    mExposedTexturesCube.clear();
    mExposedTexturesArray2D.clear();
    mTextures.clear();
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

// Reconstructed from eboot.elf at 0x4F8D90. Each quality level gets one
// usage group from the graph and the render state, with 8 added for a
// graph whose root asks for it (GetUsageHints bit 0) and for scene-mask
// materials. A material without a graph uses 0x40 alone. The meanings of
// the bits are not recovered: the emissive groups are 0x400 (lit, opaque),
// 0x800 (lit) and 0x1000; the buckets 1 to 3 take 0x20000, 0x80000 and
// 0x100000; blended materials take 0x8000, 0x10000 (forced opaque) or
// 0x20000 (scene texture level 2); opaque ones add 4 to a forward (0x2002,
// 0x2000, 0x4000) or deferred group (0x10, 0x20, 0x40, or with a depth
// prepass 0x81, 0x101, 0x201).
void RndMaterialRuntimeData::InitUsageHints() {
    // RndBlendMode::kSource, the opaque mode.
    constexpr int kOpaqueBlendMode = 5;
    for (unsigned int quality = 0; quality < 3; ++quality) {
        unsigned int& hints = mUsageHints[quality];
        hints = 0;
        if (mGraph == nullptr) {
            hints = 0x40;
            continue;
        }
        if (mGraph->IsEmissive()) {
            if (mGraph->IsLit()) {
                hints |= mBlendMode == kOpaqueBlendMode ? 0x400 : 0x800;
            } else {
                hints |= 0x1000;
            }
        } else {
            if ((mGraph->GetUsageHints() & 1) != 0) {
                hints |= 8;
            }
            if (mBucket == 3) {
                hints |= 0x100000;
            } else if (mBucket == 2) {
                hints |= 0x80000;
            } else if (mBucket == 1) {
                hints |= 0x20000;
            } else if (mBlendMode != kOpaqueBlendMode) {
                if (mGraph->mSceneTexUsage == 2) {
                    hints |= 0x20000;
                } else {
                    hints |= mForceOpaque ? 0x10000 : 0x8000;
                }
            } else {
                hints |= 4;
                const int sceneTex = mGraph->mSceneTexUsage;
                if (sceneTex == 2) {
                    hints |= 0x20004;
                } else if (sceneTex == 1 || mGraph->mUsesLinearDepth) {
                    hints |= mGraph->IsLit() ? 0x2000 : 0x4000;
                } else if (mGraph->IsFwdLit(static_cast<int>(quality))) {
                    hints |= 0x2002;
                } else if (mGraph->IsLit()) {
                    if (mGraph->UsesAlphaCut()) {
                        hints |= mDepthPrepass ? 0x101 : 0x20;
                    } else {
                        hints |= mDepthPrepass ? 0x81 : 0x10;
                    }
                } else {
                    hints |= mDepthPrepass ? 0x201 : 0x40;
                }
            }
        }
        if (mSceneMask) {
            hints |= 8;
        }
    }
}
