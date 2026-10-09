#include "renderps4/buffers/PS4ComputeBuffer.h"

#include <cstring>

#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"

namespace {

// RndComputeBuffer::Description::mFlags bits. Names not in the reference map.
constexpr unsigned int kFlagWritable = 0x1;
constexpr unsigned int kFlagIndirectArgs = 0x8;
constexpr unsigned int kFlagDoubleBuffered = 0x10;
// Bound with flags & kSelectReadWrite as a read-write buffer.
constexpr unsigned int kSelectReadWrite = 0x1;

// Initial contents of an indirect-argument buffer without initial data.
// Name not in the reference map.
const unsigned int kDefaultIndirectArgs[4] = {0, 0, 0, 1};  // 0x12D1730

}  // namespace

// Reconstructed from eboot.elf at 0x8E3250.
PS4ComputeBuffer::PS4ComputeBuffer(const Description& desc)
    : RndComputeBuffer(desc),
      mBuffers{},
      mStorage{nullptr, nullptr},
      mActiveBank(0) {}

// Reconstructed from eboot.elf at 0x8E3290. The deleting destructor is at
// 0x8E32F0.
PS4ComputeBuffer::~PS4ComputeBuffer() {
    _FreeImpl();
}

// Reconstructed from eboot.elf at 0x8E3350. Buffers that bring their own GPU
// memory are described in place; the others get one allocation per bank from
// the "gpu" heap.
bool PS4ComputeBuffer::_SyncStaticImpl() {
    gPS4Device->DeferredDelete(mStorage[0]);
    mStorage[0] = nullptr;
    gPS4Device->DeferredDelete(mStorage[1]);
    mStorage[1] = nullptr;

    const unsigned long banks = (mDesc.mFlags & kFlagDoubleBuffered) != 0 ? 2 : 1;
    for (unsigned long bank = 0; bank < banks; ++bank) {
        void* data = mDesc.mGpuData;
        if (data == nullptr) {
            const int alignment = (mDesc.mFlags & kFlagIndirectArgs) != 0 ? 8 : 4;
            static long sGpuHeap = MemFindHeap("gpu");
            MemPushHeap(sGpuHeap);
            data = MemAlloc(
                mDesc.mNumElements * mDesc.mElementSize, "ComputeBuffer", alignment);
            mStorage[bank] = data;
            MemPopHeap();
            if (mDesc.mInitialData != nullptr) {
                std::memcpy(
                    mStorage[bank],
                    mDesc.mInitialData,
                    mDesc.mNumElements * mDesc.mElementSize);
            } else if ((mDesc.mFlags & kFlagIndirectArgs) != 0) {
                std::memcpy(mStorage[bank], kDefaultIndirectArgs, sizeof(kDefaultIndirectArgs));
            }
        }
        mBuffers[bank].initAsRegularBuffer(
            data,
            static_cast<unsigned int>(mDesc.mElementSize),
            static_cast<unsigned int>(mDesc.mNumElements));
        mBuffers[bank].setResourceMemoryType(
            (mDesc.mFlags & (kFlagWritable | kFlagIndirectArgs)) != 0
                ? sce::Gnm::kResourceMemoryTypeGC
                : sce::Gnm::kResourceMemoryTypeRO);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x8E34D0.
void PS4ComputeBuffer::_FreeImpl() {
    for (auto& storage : mStorage) {
        gPS4Device->DeferredDelete(storage);
        storage = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x8E3510.
void PS4ComputeBuffer::_SyncDynamicImpl(RndContext&) {
    mActiveBank = (mActiveBank & 1U) == 0 ? 1 : 0;
    std::memcpy(mStorage[mActiveBank], mStagingData, mStagingSize);
}

// Reconstructed from eboot.elf at 0x8E3540.
void PS4ComputeBuffer::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E37D0.
void PS4ComputeBuffer::_GpuCopyFromImpl(RndContext&, RndShaderResource&) {}

// Reconstructed from eboot.elf at 0x8E3580. A vertex-stage buffer is bound
// to both the ES and VS stages.
void PS4ComputeBuffer::_SelectForVSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    auto& gfx = static_cast<PS4Context&>(context)._ActiveGfxContext();
    const auto index = static_cast<unsigned int>(slot);
    gfx.setBuffers(sce::Gnm::kShaderStageEs, index, 1, &ActiveBuffer());
    gfx.setBuffers(sce::Gnm::kShaderStageVs, index, 1, &ActiveBuffer());
}

// Reconstructed from eboot.elf at 0x8E3600.
void PS4ComputeBuffer::_SelectForHSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    static_cast<PS4Context&>(context)._ActiveGfxContext().setBuffers(
        sce::Gnm::kShaderStageHs, static_cast<unsigned int>(slot), 1, &ActiveBuffer());
}

// Reconstructed from eboot.elf at 0x8E3630. The domain shader runs on the LS
// stage.
void PS4ComputeBuffer::_SelectForDSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    static_cast<PS4Context&>(context)._ActiveGfxContext().setBuffers(
        sce::Gnm::kShaderStageLs, static_cast<unsigned int>(slot), 1, &ActiveBuffer());
}

// Reconstructed from eboot.elf at 0x8E3660.
void PS4ComputeBuffer::_SelectForGSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    static_cast<PS4Context&>(context)._ActiveGfxContext().setBuffers(
        sce::Gnm::kShaderStageGs, static_cast<unsigned int>(slot), 1, &ActiveBuffer());
}

// Reconstructed from eboot.elf at 0x8E3690.
void PS4ComputeBuffer::_SelectForPSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int flags) {
    auto& gfx = static_cast<PS4Context&>(context)._ActiveGfxContext();
    const auto index = static_cast<unsigned int>(slot);
    if ((flags & kSelectReadWrite) != 0) {
        gfx.setRwBuffers(sce::Gnm::kShaderStagePs, index, 1, &ActiveBuffer());
    } else {
        gfx.setBuffers(sce::Gnm::kShaderStagePs, index, 1, &ActiveBuffer());
    }
}

// Reconstructed from eboot.elf at 0x8E36D0.
void PS4ComputeBuffer::_SelectForCSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int flags,
    unsigned long) {
    auto& ps4 = static_cast<PS4Context&>(context);
    const auto index = static_cast<unsigned int>(slot);
    if ((flags & kSelectReadWrite) != 0) {
        if (context.mActivePipe == 1) {
            ps4._ActiveComputeContext().setRwBuffers(
                static_cast<int>(index), 1, &ActiveBuffer());
        } else if (context.mActivePipe == 0) {
            ps4._ActiveGfxContext().setRwBuffers(
                sce::Gnm::kShaderStageCs, index, 1, &ActiveBuffer());
        }
    } else if (context.mActivePipe == 1) {
        ps4._ActiveComputeContext().setBuffers(static_cast<int>(index), 1, &ActiveBuffer());
    } else if (context.mActivePipe == 0) {
        ps4._ActiveGfxContext().setBuffers(
            sce::Gnm::kShaderStageCs, index, 1, &ActiveBuffer());
    }
}
