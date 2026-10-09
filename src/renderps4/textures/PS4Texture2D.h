#pragma once

#include <cstddef>

#include "render/textures/RndTexture2D.h"

namespace rb4 {
struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;
}  // namespace rb4

// The vtable is at 0x195EC70.
class PS4Texture2D : public RndTexture2D {
public:
    explicit PS4Texture2D(const Description& desc);  // 0x8D62C0
    ~PS4Texture2D() override;                         // 0x8D6310, 0x8D6440

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6E40
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6F10
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6F80
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6FF0
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D7060
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8D70D0
    // Not yet reconstructed.
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8D7140
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8D6460
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8D6D10
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8D6E20
    void _Slot20Impl() override;                                  // 0x8D6E30

    // Reconstructed from eboot.elf at 0x8D71E0: the active frame's target,
    // falling back to the first. Name from the map's GetRenderTarget.
    const rb4::OrbisGpuRenderTarget* GetRenderTarget() const;
    // Reconstructed from eboot.elf at 0x8D7240.
    const rb4::OrbisGpuDepthRenderTarget* GetDepthStencilTarget() const {
        return mDepthTarget;
    }
    // Presentation counts per back buffer. Names not in the reference map.
    void AddPendingPresentation(unsigned long buffer) {
        ++mPendingPresentations[buffer];
    }
    void CompletePendingPresentation(unsigned long buffer) {
        --mPendingPresentations[buffer];
    }

    // GPU memory size and alignment. Name not in the reference map.
    struct SurfaceSize {
        unsigned int mSize;
        unsigned int mAlign;
    };

    // Field names are not in the reference map.
    void* mGpuTextures[2];
    void* mPlaneTexture;
    SurfaceSize mDepthSize;
    SurfaceSize mStencilSize;
    SurfaceSize mHtileSize;
    unsigned long mActiveStorage;
    void* mStorageRegions;
    void* mStorageControl;
    unsigned char mUnknown480[8];
    rb4::OrbisGpuRenderTarget* mRenderTargets[2];
    rb4::OrbisGpuDepthRenderTarget* mDepthTarget;
    int* mPendingPresentations;

private:
    // Map names; this build's signatures differ. Not yet reconstructed.
    void _SyncDepthStencil();                             // 0x8D6480
    void _SyncRegular(const RndTextureBase* reuse);       // 0x8D6850
    // Stand-ins for code inlined into the destructor, the dynamic sync, and
    // the stage selects; not yet reconstructed. Names not in the reference
    // map.
    static void ReleaseStorage(void* control);
    void FlipStorage();
    void UploadMips();
    const void* SelectView(unsigned int flags) const;
};

static_assert(offsetof(PS4Texture2D, mGpuTextures) == 408);
static_assert(sizeof(PS4Texture2D) == 520);
