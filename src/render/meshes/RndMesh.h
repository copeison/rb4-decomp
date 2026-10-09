#pragma once

#include <atomic>
#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/meshes/RndDynamicGpuData.h"
#include "render/meshes/RndVertex.h"
#include "utl/containers/Vector.h"
#include "utl/containers/VectorAdapter.h"

class RndContext;

// Per-instance transforms streamed with a draw: a 3x4 world transform, the
// 3x3 normal transform, packed state and two parameter vectors. Field names
// are not in the reference map.
struct RndInstanceData {
    float mXfm[3][4];
    float mNormalXfm[3][3];
    unsigned int mPackedState;
    float mParams[2][4];
};

static_assert(offsetof(RndInstanceData, mNormalXfm) == 48);
static_assert(offsetof(RndInstanceData, mPackedState) == 84);
static_assert(offsetof(RndInstanceData, mParams) == 88);
static_assert(sizeof(RndInstanceData) == 120);

// Anything that can be drawn in instanced batches.
class RndDrawable {
public:
    // Triangle subrange for a draw; a count of ~0 draws every face. Name not
    // in the reference map.
    struct DrawRange {
        static constexpr unsigned long kAllFaces = ~0UL;
        unsigned long mFirstFace;
        unsigned long mNumFaces;
    };

    virtual ~RndDrawable() {}
    // Slot 2. The map has _DrawBatchImpl(RndContext&,
    // VectorAdapter<RndInstanceData> const&); this build adds the range.
    virtual void _DrawBatchImpl(
        RndContext& context,
        const VectorAdapter<RndInstanceData>& instances,
        const DrawRange& range) = 0;
};

// Indexed triangle mesh. The vertex storage is added by RndMeshTyped; the
// GPU buffers by the platform subclass. RndDynamicGpuData is the secondary
// base at +8.
class RndMesh : public RndDrawable, public RndDynamicGpuData {
public:
    struct Face {
        unsigned int mIdx[3];
    };

    // Reconstructed from eboot.elf at 0x5C26D0.
    static RndMesh* New(RndVertexType type, const char* name);

    explicit RndMesh(const char* name);  // 0x5C2700
    ~RndMesh() override {}               // 0x5C27B0, 0x5C2870

    virtual RndVertexType _GetVertexTypeImpl() const = 0;             // slot 3
    virtual unsigned long _GetNumVerticesImpl() const = 0;            // slot 4
    virtual void _SetNumVerticesImpl(unsigned long count) = 0;        // slot 5
    // Slot 6. Releases the vertex storage. Name not in the reference map.
    virtual void _FreeVerticesImpl() = 0;
    virtual void* _GetVertexVoidImpl(unsigned long index) = 0;        // slot 7
    virtual void* _MakeVerticesCopyImpl() const = 0;                  // slot 8
    virtual void _SyncStaticImpl() = 0;                               // slot 9
    virtual void _SyncDynamicImpl(RndContext& context, unsigned int flags) = 0;  // slot 10
    // Slot 11 at 0x5C2E40, with the secondary thunk at 0x5C2EA0.
    void _SyncDynamicGpuDataImpl(RndContext& context) override;

    void SetKeepMeshData(bool keep);           // 0x5C2930
    void SetVertexUsageFlags(unsigned int flags);  // 0x5C2940
    void SetFaceUsageFlags(unsigned int flags);    // 0x5C2950
    // Reconstructed from eboot.elf at 0x5C2960 and 0x5C2980. Names not in
    // the reference map.
    bool KeepVertices() const;
    bool KeepFaces() const;

    // Reconstructed from eboot.elf at 0x5C29A0. Uploads the mesh, then frees
    // the CPU copies the usage flags do not need. The map has
    // SyncStatic(unsigned int).
    void SyncStatic();
    // Reconstructed from eboot.elf at 0x5C2DF0.
    void SyncDynamic(RndContext& context, unsigned int flags);

    // Field names are not in the reference map.
    eastl::vector<Face> mFaces;
    unsigned long mNumVerts;
    unsigned long mNumFaces;
    long mGeometryFrame;
    bool mKeepMeshData;
    bool mKeepFaces;
    unsigned int mVertexUsageFlags;
    unsigned int mFaceUsageFlags;
    unsigned int mUnknown92[4];
    std::atomic<unsigned int> mPendingSync;
    unsigned long mLastUseFrame;
    const char* mName;
};

static_assert(sizeof(RndMesh::Face) == 12);
static_assert(offsetof(RndMesh, mMgr) == 16);
static_assert(offsetof(RndMesh, mFaces) == 24);
static_assert(offsetof(RndMesh, mNumFaces) == 64);
static_assert(offsetof(RndMesh, mKeepMeshData) == 80);
static_assert(offsetof(RndMesh, mPendingSync) == 108);
static_assert(offsetof(RndMesh, mLastUseFrame) == 112);
static_assert(sizeof(RndMesh) == 128);
