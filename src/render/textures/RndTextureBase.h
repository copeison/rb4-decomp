#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/shaders/RndShaderResource.h"
#include "render/textures/RndPixelFormat.h"

class BinStream;

// Common texture base. Each texture keeps a copy of its description's base
// fields here; the per-class description follows in the subclass. The base
// vtable is at 0x1935678.
class RndTextureBase : public RndShaderResource {
public:
    // Texture kinds stored in Description::mType. Names not in the
    // reference map.
    enum Type : int {
        kTexture1D = 0,
        kTexture2D = 1,
        kTexture3D = 2,
        kTextureCube = 3,
        kTextureArray1D = 4,
        kTextureArray2D = 5,
        kTextureArrayCube = 7,
    };

    // 144-byte description shared by every texture kind. Field names are not
    // in the reference map.
    class Description {
    public:
        Description();  // 0x69B930

        // Reconstructed from eboot.elf at 0x6A4770. Fills unset fields of
        // the resolved format from the requested format and the usage
        // defaults. Name not in the reference map.
        void ResolveFormat(int type, long fallback);
        // Reconstructed from eboot.elf at 0x69B990. The map has
        // RndTextureBase::_ShouldKeepPixelData() const.
        bool ShouldKeepPixelData() const;

        int mType;
        RndPixelFormat mRequestedFormat;
        RndPixelFormat mFormat;
        int mDataFormat;
        unsigned int mWidth;
        unsigned int mHeight;
        unsigned int mDepth;
        unsigned int mUnknown108;
        // Array textures store their layer count here.
        union {
            void* mSourceData;
            unsigned long mArraySize;
        };
        union {
            unsigned int mSourceSize;
            unsigned int mTargetFlags;
        };
        bool mKeepPixelData;
        union {
            int mBindlessIndex;
            int mAttachmentIndex;
        };
        union {
            unsigned int mResourceFlags;
            int mAttachmentCount;
        };
        const char* mName;
    };

    RndTextureBase();  // 0x69B6E0
    ~RndTextureBase() override {}  // 0x69B770, 0x69B780

    int _GetTypeImpl() const override;  // 0x690500

    // Slots 10-11. Names not in the reference map.
    virtual unsigned long _GetNumMipsImpl() const = 0;
    virtual unsigned long _GetTotalBytesImpl() const = 0;
    // Slot 12.
    virtual void _SetRequestedFormatImpl(const RndPixelFormat& format) = 0;
    // Slot 13.
    virtual void _FreePixelDataImpl() = 0;
    // Slot 14 at 0x697870. 2D textures can stand in for a linked texture
    // and slice. Name not in the reference map.
    virtual RndTextureBase* _GetLinkedTextureImpl(long& index);
    // Slot 15. The map has no parameter; this build forwards the texture
    // whose storage may be reused.
    virtual void _SyncStaticImpl(const RndTextureBase* reuse) = 0;
    // Slot 16 at 0x690510.
    virtual bool _SyncStaticFromStreamImpl(BinStream& stream);
    // Slot 17 at 0x697880.
    virtual void _LoadBuffersImpl(BinStream& stream);
    // Slots 18-20, overridden by every platform texture.
    virtual void _SyncDynamicImpl(RndContext& context) = 0;
    virtual void _SyncFromGpuImpl(RndContext& context) = 0;
    // Name not in the reference map.
    virtual void _Slot20Impl() = 0;

    // Reconstructed from eboot.elf at 0x69B7A0. Skipped while precaching.
    void SyncStatic(const RndTextureBase* reuse);

    // Name not in the reference map.
    unsigned int WrapMode() const {
        return mBaseDesc.mFormat.mWrapMode;
    }
    // Name not in the reference map.
    unsigned int FilterMode() const {
        return mBaseDesc.mFormat.mFilterMode;
    }

    DELETE_OVERLOAD

    Description mBaseDesc;  // Name not in the reference map.
    int mResourceIndex;     // Name not in the reference map.

protected:
    // Copies a subclass description's base fields. Name not in the
    // reference map.
    void SetBaseDesc(const Description& desc) {
        mBaseDesc = desc;
    }
};

static_assert(sizeof(RndPixelFormat) == 44);
static_assert(offsetof(RndTextureBase::Description, mRequestedFormat) == 4);
static_assert(offsetof(RndTextureBase::Description, mFormat) == 48);
static_assert(offsetof(RndTextureBase::Description, mDataFormat) == 92);
static_assert(offsetof(RndTextureBase::Description, mSourceData) == 112);
static_assert(offsetof(RndTextureBase::Description, mKeepPixelData) == 124);
static_assert(offsetof(RndTextureBase::Description, mAttachmentIndex) == 128);
static_assert(offsetof(RndTextureBase::Description, mName) == 136);
static_assert(sizeof(RndTextureBase::Description) == 144);
static_assert(offsetof(RndTextureBase, mBaseDesc) == 16);
static_assert(offsetof(RndTextureBase, mResourceIndex) == 160);
static_assert(sizeof(RndTextureBase) == 168);

// Default wrap and filter modes for a resource kind, at 0x50CE00 and
// 0x50CE30; -1 when the kind has no default. Names not in the reference map.
int TextureDefaultWrapMode(unsigned int kind);
int TextureDefaultFilterMode(unsigned int kind);
