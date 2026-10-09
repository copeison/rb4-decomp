#pragma once

#include <cstddef>

#include "math/geometry/Sphere.h"
#include "math/vector/Vector4.h"
#include "render/meshes/RndMesh.h"

class RndMaterialRuntimeData;
class RndShaderCBuffer;

// The draw buckets the scene drawer sorts its instances into: a draw
// instance's mStateFlags has one bit per bucket, and the drawer and the
// culler keep one PodVector per bucket. The scene levels (RndSceneLod) are
// declared with RndDrawInstanceCom. Bucket 2 is the only one the
// drawer also reserves in its third instance set. Name not in the reference
// map; the buckets' meanings are recovered with the passes that flush them.
static constexpr unsigned long kNumDrawBuckets = 22;

// One drawable the scene drawer may draw: what to draw, its render state,
// its bounds and its instance data (232 bytes). Draw instance components
// fill them (RndDrawInstanceCom::InitDrawInstances; 0x6C3500 sets the
// state flags), the culler copies the visible ones into the drawer's
// buckets, and the flush templates batch consecutive instances whose state
// matches. Field names are not in the reference map; they follow the
// readers in render/RndSceneDrawer.o and render/RndBasicCuller.o.
struct RndDrawInstance {
    // Inlined wherever instances are made (RndBasicCuller's registration
    // and free list, 0x45A490 and 0x45AA60, and eastl's DoInsertValues at
    // 0x45E4D0). It leaves mPad31, mPad74 and mInstanceData.mPackedState.
    RndDrawInstance()
        : mDrawable(nullptr),
          mInstanceCBuffer(nullptr),
          mMaterial(nullptr),
          mActive(true),
          mShowing(true),
          mCounterClockwise(true),
          mUsesSceneTex(false),
          mUsesSceneDepth(false),
          mReceiveAtmosphere(true),
          mReceiveDecals(1),
          mCullMode(0),
          mEnvironIndex(-2),
          mWorldShowHideFlags(0),
          mStateFlags(0),
          mDebugTag(-1),
          mBounds(Sphere::sZero),
          mSortBy(2),
          mSortingHint(0),
          mNoCull(false),
          mDistance(0.0F) {
        for (auto& row : mInstanceData.mXfm) {
            for (float& value : row) {
                value = 0.0F;
            }
        }
        for (auto& row : mInstanceData.mNormalXfm) {
            for (float& value : row) {
                value = 0.0F;
            }
        }
        for (auto& row : mInstanceData.mParams) {
            for (float& value : row) {
                value = 0.0F;
            }
        }
        for (Vector4& plane : mClipPlanes) {
            plane = Vector4{0.0F, 0.0F, 0.0F, 0.0F};
        }
    }

    // Batched draws call its _DrawBatchImpl (slot 2) with the instances'
    // RndInstanceData.
    RndDrawable* mDrawable;
    // An optional constant buffer selected before the batch (slot 4); a
    // batch with one draws its material with the buffer flag set.
    RndShaderCBuffer* mInstanceCBuffer;
    // The material component's runtime data (RndMeshCom's instance sync,
    // 0x5CA2D0, stores RndMaterialCom+272), selected through
    // RndMaterialRuntimeData::SelectShader (0x4F8D10) when it changes;
    // batching needs one.
    RndMaterialRuntimeData* mMaterial;
    // The culler skips instances without both flags.
    bool mActive;
    bool mShowing;
    // The draw node's winding (+179, det(world) >= 0), passed to
    // RndContext slot 12 as the front face.
    bool mCounterClockwise;
    // The material's graph flags (RndMaterialRuntimeData::mRootFlags[1]
    // and [2]); the depth pass reselects the material only when they are
    // set.
    bool mUsesSceneTex;
    bool mUsesSceneDepth;
    // The material's "receive_atmosphere" and "receive_decals" (+248,
    // +249). The deferred passes write them into the stencil with
    // mEnvironIndex (0x42C710: a clear mReceiveAtmosphere adds 8,
    // mReceiveDecals is shifted by 4); the forward passes switch the
    // atmosphere shading on mReceiveAtmosphere.
    bool mReceiveAtmosphere;
    unsigned char mReceiveDecals;
    unsigned char mPad31;
    // The cull mode passed to RndContext slot 13; -1 leaves it.
    int mCullMode;
    // The draw node's "environ_index" (+128): the lighting environment,
    // drawn into the stencil (-2 and -1 select references 1 and 0; others
    // add 2) or into the forward passes' draw state constants.
    int mEnvironIndex;
    // The draw node's "world_show_hide_flags" (+124), matched against
    // RndShowHideContext.
    unsigned int mWorldShowHideFlags;
    // The material's usage hints and the draw node's 0x40000; each of the
    // low kNumDrawBuckets bits puts the instance in a draw bucket (the
    // culler's bucket count, 0x45B940).
    unsigned int mStateFlags;
    // The draw instance component's tag (+80, RuntimeData::mActiveDebugTag):
    // the second sort key; batches need it equal.
    int mDebugTag;
    Sphere mBounds;
    // The component's "sort_by".
    int mSortBy;
    // The component's "sorting_hint" when the material's bucket is 5: the
    // first sort key, compared signed.
    signed char mSortingHint;
    // Drawn without the frustum and clip-plane tests.
    bool mNoCull;
    unsigned char mPad74[2];
    // Its mPackedState is the component's "billboarding" and its mParams are
    // "extra_data" and "extra_data_1".
    RndInstanceData mInstanceData;
    // Two user clip planes (a, b, c, d); a zero plane leaves the context's
    // plane disabled.
    Vector4 mClipPlanes[2];
    // Camera distance for the back-to-front sorts.
    float mDistance;
};

static_assert(offsetof(RndDrawInstance, mInstanceCBuffer) == 8);
static_assert(offsetof(RndDrawInstance, mMaterial) == 16);
static_assert(offsetof(RndDrawInstance, mActive) == 24);
static_assert(offsetof(RndDrawInstance, mCounterClockwise) == 26);
static_assert(offsetof(RndDrawInstance, mReceiveAtmosphere) == 29);
static_assert(offsetof(RndDrawInstance, mCullMode) == 32);
static_assert(offsetof(RndDrawInstance, mEnvironIndex) == 36);
static_assert(offsetof(RndDrawInstance, mWorldShowHideFlags) == 40);
static_assert(offsetof(RndDrawInstance, mStateFlags) == 44);
static_assert(offsetof(RndDrawInstance, mDebugTag) == 48);
static_assert(offsetof(RndDrawInstance, mBounds) == 52);
static_assert(offsetof(RndDrawInstance, mSortBy) == 68);
static_assert(offsetof(RndDrawInstance, mSortingHint) == 72);
static_assert(offsetof(RndDrawInstance, mNoCull) == 73);
static_assert(offsetof(RndDrawInstance, mInstanceData) == 76);
static_assert(offsetof(RndDrawInstance, mClipPlanes) == 196);
static_assert(offsetof(RndDrawInstance, mDistance) == 228);
static_assert(sizeof(RndDrawInstance) == 232);
