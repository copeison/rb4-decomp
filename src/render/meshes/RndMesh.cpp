#include "render/meshes/RndMesh.h"

#include <cstring>

#include "render/core/system/render_epoch.h"
#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x5C26D0.
RndMesh* RndMesh::New(RndVertexType type, const char* name) {
    return TheRndFactory()->CreateMesh(type, name);
}

// Reconstructed from eboot.elf at 0x5C2700.
RndMesh::RndMesh(const char* name)
    : mNumVerts(0),
      mNumFaces(0),
      mGeometryFrame(-1),
      mKeepMeshData(false),
      mKeepFaces(false),
      mVertexUsageFlags(0),
      mFaceUsageFlags(0),
      mUnknown92{~0U, ~0U, ~0U, ~0U},
      mPendingSync(0),
      mLastUseFrame(~0UL),
      mName(name) {}

// Reconstructed from eboot.elf at 0x5C2930.
void RndMesh::SetKeepMeshData(bool keep) {
    mKeepMeshData = keep;
}

// Reconstructed from eboot.elf at 0x5C2940.
void RndMesh::SetVertexUsageFlags(unsigned int flags) {
    mVertexUsageFlags = flags;
}

// Reconstructed from eboot.elf at 0x5C2950.
void RndMesh::SetFaceUsageFlags(unsigned int flags) {
    mFaceUsageFlags = flags;
}

bool RndMesh::KeepVertices() const {
    return mKeepMeshData || mKeepFaces || (mVertexUsageFlags & 5U) != 0;
}

bool RndMesh::KeepFaces() const {
    return mKeepMeshData || mKeepFaces || (mFaceUsageFlags & 5U) != 0;
}

void RndMesh::SyncStatic() {
    mNumFaces = mFaces.size();
    _SyncStaticImpl();

    if (mKeepMeshData || mKeepFaces) {
        return;
    }
    if ((mVertexUsageFlags & 5U) == 0) {
        _FreeVerticesImpl();
        if (mKeepMeshData) {
            return;
        }
    }
    if (!mKeepFaces && (mFaceUsageFlags & 5U) == 0) {
        mFaces.set_capacity(0);
    }
}

// Records the frame of the update before forwarding it.
void RndMesh::SyncDynamic(RndContext& context, unsigned int flags) {
    mLastUseFrame = rb4::current_render_epoch();
    if ((flags & 2U) != 0) {
        mNumFaces = mFaces.size();
    }
    _SyncDynamicImpl(context, flags);
}

// Applies the pending update flags, then clears them.
void RndMesh::_SyncDynamicGpuDataImpl(RndContext& context) {
    const auto flags = mPendingSync.load(std::memory_order_relaxed);
    if (flags == 0) {
        return;
    }

    SyncDynamic(context, flags);
    mPendingSync.exchange(0);
}
