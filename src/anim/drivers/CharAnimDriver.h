#pragma once

#include <cstddef>

#include "anim/char/CharAnimBuffer.h"
#include "anim/drivers/AnimDriver.h"
#include "utl/containers/Vector.h"

class CharAnimResource;
class CharIkCom;
class GameObject;
class PropRef;
class RndSkeletonPoseCom;

// The driver of character animations and clips (anim/CharAnimDriver.o,
// 0x2DD2F0-0x2DF92F): each poll it blends the playbacks of CharAnimResources
// and CharClipResources, slot by slot, into its CharAnimBuffer and writes
// the result into the RndSkeletonPoseCom on the entity's root, then lets the
// root's IK components adjust the pose. Within a blend-tree slot a layered
// playback hides the ones before it by its weight; a slot's weight scales
// its playbacks. The vtable at 0x18F51A0 has five slots. The object is 400
// bytes. The map's methods take the object (ObjPtr const&); this build's
// take the root GameObject. Field names are not in the reference map.
class CharAnimDriver : public AnimDriver {
public:
    CharAnimDriver();  // 0x2DD2F0
    // Slots 0-1: 0x2DD380 and 0x2DD450.
    ~CharAnimDriver() override;

    // Slot 2: starts a CharAnimResource or a CharClipResource; the null id
    // for anything else, a failed resource or a driver already playing 101
    // animations.
    AnimInstID StartAnim(Resource* resource, const AnimOptions& options) override;  // 0x2DE2E0
    // Slot 4: blends the playbacks into the pose of the root object.
    void _Poll(GameObject* root) override;  // 0x2DDA60

    // Sets the weight of a blend-tree slot; the slots added before it get
    // weight 1.
    void SetSubTreeWeight(unsigned long subTree, float weight);  // 0x2DD470
    // Stops the playbacks of a blend-tree slot.
    void StopSubTree(unsigned long subTree);  // 0x2DD560
    // Stops every playback.
    void StopAll();  // 0x2DD600
    // Collects the root's IK components (those whose class is or derives
    // from CharIk), sorted by priority.
    void _CacheIkComs(GameObject* root);  // 0x2DD6A0
    // Finds the root's RndSkeletonPoseCom and lays the buffer and the pose
    // out for every channel of every bone of its skeleton. Nothing happens
    // without a skeleton.
    void _Init(GameObject* root);  // 0x2DE000
    // Stops every playback and makes the next poll lay the pose out again.
    void Reset(GameObject* root);  // 0x2DE4A0
    // The weighted value of a float property the playing animations' objects
    // hold, in `value`, blended as the poses are; returns the total weight.
    // Only the playbacks of a blend-tree slot count unless `subTree` is -1.
    float EvaluateFloatProperty(const PropRef& ref, float& value, unsigned long subTree);  // 0x2DE600
    // The CharAnimResource whose weight dominates the blend, in a slot or in
    // all of them (-1), or null.
    CharAnimResource* FindMajorityAnim(unsigned long subTree) const;  // 0x2DE990
    // As EvaluateFloatProperty for a bool property, counted as 0 or 1, over
    // every playback.
    float EvaluateBoolProperty(const PropRef& ref, float& value);  // 0x2DED30

    // Set once _Init found a skeleton.
    bool mInitialized;
    // Stops the polls. No writer was found.
    bool mDisabled;
    CharAnimBuffer mBuffer;
    // The weight of each blend-tree slot (1 by default).
    eastl::vector<float> mSubTreeWeights;
    eastl::vector<CharIkCom*> mIkComs;
    // The root's pose component, set by _Init.
    RndSkeletonPoseCom* mSkeletonCom;
};

static_assert(offsetof(CharAnimDriver, mInitialized) == 64);
static_assert(offsetof(CharAnimDriver, mDisabled) == 65);
static_assert(offsetof(CharAnimDriver, mBuffer) == 72);
static_assert(offsetof(CharAnimDriver, mSubTreeWeights) == 328);
static_assert(offsetof(CharAnimDriver, mIkComs) == 360);
static_assert(offsetof(CharAnimDriver, mSkeletonCom) == 392);
static_assert(sizeof(CharAnimDriver) == 400);
