#pragma once

#include <cstddef>

#include "render/core/transition_aliases.h"

// Common base of resources that shaders read: compute buffers and textures.
// Slots 0-9 are shared by every resource vtable.
class RndShaderResource {
public:
    RndShaderResource() : mFrameStamp(-1) {}
    virtual ~RndShaderResource() {}  // slots 0-1

    // Slot 2. Compute buffers return -1 (0x636DB0); textures return their
    // description's type (0x690500). Name not in the reference map.
    virtual int _GetTypeImpl() const = 0;

    // Slots 3-8. The compute stage takes an extra argument.
    virtual void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) = 0;
    virtual void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) = 0;
    virtual void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) = 0;
    virtual void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) = 0;
    virtual void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) = 0;
    virtual void _SelectForCSImpl(
        RndContext& context,
        unsigned long slot,
        unsigned int flags,
        unsigned long extra) = 0;

    // Slot 9.
    virtual void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) = 0;

    // Render-system frame epoch of the most recent bind. Name not in the
    // reference map.
    long mFrameStamp;
};

static_assert(offsetof(RndShaderResource, mFrameStamp) == 8);
static_assert(sizeof(RndShaderResource) == 16);
