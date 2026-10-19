// stategraph/ClipJukeboxCom.o (0x803690 to 0x807E7F). The class's
// registration and factory are emitted in stategraph/StateGraph.o, and so
// are the copies of PropArray<ClipJukeboxCom::Node>'s element functions
// (0x6FF250-0x6FF400).
#include "stategraph/blendtrees/ClipJukeboxCom.h"

#include <math.h>
#include <new>

#include "anim/char/CharClipCom.h"
#include "anim/char/CharClipResource.h"
#include "anim/core/AnimOptions.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "math/random/Rand.h"
#include "utl/containers/Sort.h"
#include "utl/time/TimeMgr.h"

namespace {

// The id of no object, at 0x1ABEA68, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The jukebox whose property a tag function or an accessor describes. Name
// not in the reference map.
ClipJukeboxCom& Jukebox(const TagArgs& args) {
    return const_cast<ClipJukeboxCom&>(static_cast<const ClipJukeboxCom&>(args.mComponent));
}
ClipJukeboxCom& Jukebox(const PropAccessorArgs& args) {
    return *static_cast<ClipJukeboxCom*>(args.mComponent);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x807DC0).
Symbol ClipJukeboxCom::sId("ClipJukebox");
Symbol ClipJukeboxCom::sClassName("ClipJukebox");
PropRegistry ClipJukeboxCom::sPropRegistry;
ComMetaData ClipJukeboxCom::sMetaData;

// Reconstructed from eboot.elf at 0x803690.
ClipJukeboxCom::ClipJukeboxCom() : mCrossfadeFrames(10.0f), mRandomize(true), mNodes() {}

// Reconstructed from eboot.elf at 0x803720. A new clip's weight comes from
// SetNodeWeight.
ClipJukeboxCom::Node::Node() : mClip{0xFFFFFFFFU}, mWeight(0.0f), mSpeed(1.0f) {}

// Reconstructed from eboot.elf at 0x803740. "weight" shows only while the
// jukebox randomizes; its default is an equal share.
void ClipJukeboxCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<Node>(Symbol("ClipJukebox::Node"));
    ClipJukeboxCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    // The driving property is in the entity the graph is inlined into; at
    // 0x8067F0.
    GroupMetadata& crossfade = prototype.mCrossfadeFrames.Register(
        registry,
        Symbol("crossfade_frames"),
        offsetof(ClipJukeboxCom, mCrossfadeFrames),
        [](const TagArgs& args) { return args.mComponent.mObject->mEntity->mResource->mInlineOwner->mEntity; });
    static_cast<void>(Symbol("driven_prop"));
    crossfade.mHelp = String(
        "The number of frames over which to crossfade from one clip to the next. Crossfading begins this many "
        "frames from the end of the current animation, and continues this many frames into the next animation.");
    FloatMetadata& crossfadeValue = TypeSpecificMetadata(
        const_cast<PropInfo&>(*crossfade.mRegistry.FindProp(PropPath(Symbol("value")), nullptr)),
        prototype.mCrossfadeFrames.mValue);
    static_cast<void>(Symbol("driven_prop_value"));
    crossfadeValue.mMin = 0.0f;

    BoolMetadata& randomize = TypeSpecificMetadata(
        registry.RegisterProp("randomize", offsetof(ClipJukeboxCom, mRandomize), kPropertyBool, 1),
        prototype.mRandomize);
    randomize.mDefault = prototype.mRandomize;
    static_cast<void>(Symbol("prop"));
    randomize.mHelp = String(
        "If true, the clips in this jukebox will be played back in a randomized order in accordance with their "
        "relative weights. If false, they will be played back in the order they are listed.");

    PropInfo& nodes =
        registry.RegisterPropArray("nodes", offsetof(ClipJukeboxCom, mNodes), sizeof(PropArray<Node>));
    ArrayMetadata& nodesMetadata = static_cast<ArrayMetadata&>(*nodes.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    nodesMetadata.mHelp = String(
        "The set of animations and subtrees controlled by this blend tree. Click the minus (-) button on an "
        "element to remove it.");
    nodesMetadata.mFixedSize = true;
    nodesMetadata.mExtraFlag1 = false;
    nodesMetadata.mExtraFlag2 = false;
    // The array's own insertion and removal do nothing: the actions add
    // clips and the "-" button removes them. At 0x806650 and 0x806660.
    nodesMetadata.mInsert = [](const PropAccessorArgs& args) { static_cast<void>(args); };
    void (*const removeHook)(const PropAccessorArgs&) = [](const PropAccessorArgs& args) {
        static_cast<void>(args);
    };
    nodesMetadata.mHook2 = reinterpret_cast<void*>(removeHook);
    PropRegistry::RegisterPropArrayItem(nodes, kPropertyStruct, sizeof(Node));
    GroupMetadata& item = static_cast<GroupMetadata&>(*nodesMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    Node nodePrototype;
    item.mHelp = String(
        "An animation or subtree controlled by this blend tree. Click the minus (-) button to remove this "
        "element.");
    item.mExtraFlag1 = false;
    item.mExtraFlag2 = false;
    item.mInlineAction = new PropMetadata::InlineAction();
    item.mInlineAction->mLabel = "-";
    item.mInlineAction->mEnabled = true;
    // At 0x806900.
    item.mInlineAction->mAction = [](const TagArgs& args) {
        Jukebox(args).RemoveChild(args.mPath.mNodes[1].mValue);
    };
    item.mKind = 1;
    PropRegistry& members = item.mRegistry;

    GameObjectIdMetadata& clip = TypeSpecificMetadata(
        members.RegisterProp("clip", offsetof(Node, mClip), kPropertyGameObjectId, 4), nodePrototype.mClip);
    clip.mDefault = nodePrototype.mClip;
    static_cast<void>(Symbol("prop"));
    clip.mHelp = String("A clip to be played back in jukebox style.");
    clip.mAllowsThis = false;
    clip.mRequiredComs = {CharClipCom::sId};
    // At 0x806A00.
    clip.mEntity._Set([](const TagArgs& args) { return Jukebox(args)._GetContextEntityPrototype(); });
    // At 0x806670.
    clip.mOnChanged = [](const PropAccessorArgs& args) { Jukebox(args)._SendModified(false); };

    FloatMetadata& weight = TypeSpecificMetadata(
        members.RegisterProp("weight", offsetof(Node, mWeight), kPropertyFloat, 4), nodePrototype.mWeight);
    weight.mDefault = nodePrototype.mWeight;
    static_cast<void>(Symbol("prop"));
    weight.mHelp = String(
        "The weight of this item; i.e. its likelihood of being chosen next. Note that the same item will never "
        "play twice in a row!");
    weight.mMin = 0.0f;
    weight.mMax = 1.0f;
    weight.mInstanceFile = true;
    // At 0x806680.
    weight.mSetter = [](const PropAccessorArgs& args) {
        const PropPath& path = *static_cast<const PropPath*>(args.mPath);
        Jukebox(args).SetNodeWeight(path.mNodes[1].mValue, *static_cast<const float*>(args.mValue));
    };
    // A single clip has all the weight; at 0x806B00.
    weight.mReadOnly._Set([](const TagArgs& args) { return Jukebox(args).mNodes.size() == 1; });
    // At 0x806C00.
    weight.mDefault._Set(
        [](const TagArgs& args) { return 1.0f / static_cast<float>(Jukebox(args).mNodes.size()); });
    // At 0x806D10.
    weight.mIsHidden._Set([](const TagArgs& args) { return !Jukebox(args).mRandomize; });

    FloatMetadata& speed = TypeSpecificMetadata(
        members.RegisterProp("speed", offsetof(Node, mSpeed), kPropertyFloat, 4), nodePrototype.mSpeed);
    speed.mDefault = nodePrototype.mSpeed;
    static_cast<void>(Symbol("prop"));
    speed.mHelp = String("The speed at which this item should play back.");

    GroupMetadata& actions =
        static_cast<GroupMetadata&>(*registry.RegisterProp("actions", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    actions.mHelp = String("Various actions that can be performed on the blend tree structure.");
    actions.mKind = 1;

    ActionMetadata& addClip =
        static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("add_clip").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    addClip.mHelp = String("Add a clip to the blend tree.");
    addClip.mDisplayName = String("+Clip");
    // At 0x806E10.
    addClip.mAction = [](const TagArgs& args) { Jukebox(args).AddChild(kChildClip); };

    ActionMetadata& sort = static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("sort").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    sort.mHelp = String("Sort the elements of the subtree by their weights from highest to lowest.");
    sort.mDisplayName = String("Sort");
    // At 0x806F10, with eastl::sort's copies at 0x807160.
    sort.mAction = [](const TagArgs& args) {
        ClipJukeboxCom& jukebox = Jukebox(args);
        eastl::sort(jukebox.mNodes.begin(), jukebox.mNodes.end(), [](const Node& a, const Node& b) {
            return a.mWeight > b.mWeight;
        });
        jukebox._SendModified(false);
    };
    // Unavailable once the weights descend; at 0x807620.
    sort.mReadOnly._Set([](const TagArgs& args) {
        const ClipJukeboxCom& jukebox = Jukebox(args);
        const unsigned long count = jukebox.mNodes.size();
        if (count < 2) {
            return true;
        }
        for (unsigned long index = 1; index < count; ++index) {
            if (jukebox.mNodes[index - 1].mWeight < jukebox.mNodes[index].mWeight) {
                return false;
            }
        }
        return true;
    });

    ActionMetadata& equalize =
        static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("equalize").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    equalize.mHelp = String("Equalize the weights of all of the clips.");
    equalize.mDisplayName = String("Equalize");
    // At 0x807760.
    equalize.mAction = [](const TagArgs& args) {
        ClipJukeboxCom& jukebox = Jukebox(args);
        const unsigned long count = jukebox.mNodes.size();
        if (count != 0) {
            const float share = 1.0f / static_cast<float>(count);
            for (Node& node : jukebox.mNodes) {
                node.mWeight = share;
            }
        }
        jukebox._SendModified(false);
    };
    // Unavailable once the weights are equal; at 0x8078B0.
    equalize.mReadOnly._Set([](const TagArgs& args) {
        const ClipJukeboxCom& jukebox = Jukebox(args);
        const unsigned long count = jukebox.mNodes.size();
        if (count < 2) {
            return true;
        }
        for (unsigned long index = 1; index < count; ++index) {
            if (fabsf(jukebox.mNodes[index - 1].mWeight - jukebox.mNodes[index].mWeight) > 0.0001f) {
                return false;
            }
        }
        return true;
    });

    // Before revision 1, "crossfade_frames" was a plain float; it becomes
    // the driven value's own. At 0x807A10.
    registry.SetCurrentRev(1);
    registry.mRevisionFunc = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev > 0) {
            return;
        }
        const float frames = GetPropValue<float>(*args.mSavedProps, PropPath(Symbol("crossfade_frames")));
        static_cast<ClipJukeboxCom*>(args.mComponent)->mCrossfadeFrames = DrivenProp<float>(frames);
    };

    BlendTreeBaseCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "Manages blending between a set of animations or subtrees of animations based on some specified "
        "criteria.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mAliases.push_back(Symbol("CharBlendTree"));
    metadata.mAliases.push_back(Symbol("BlendTreeCom"));
}

// Reconstructed from eboot.elf at 0x804BB0. The deleting destructor is at
// 0x806240.
ClipJukeboxCom::~ClipJukeboxCom() {}

// Reconstructed from eboot.elf at 0x804C40.
unsigned int ClipJukeboxCom::_GetNumChildren() const {
    return mNodes.size();
}

// Reconstructed from eboot.elf at 0x804C50.
int ClipJukeboxCom::_GetChildType(unsigned long index) const {
    static_cast<void>(index);
    return kChildClip;
}

// Reconstructed from eboot.elf at 0x804C60.
GameObjectId ClipJukeboxCom::_GetChildObject(unsigned long index) const {
    return mNodes[index].mClip;
}

// Reconstructed from eboot.elf at 0x804C70. Each clip votes for its
// resource's timeline with its weight; the earlier timeline wins a tie.
int ClipJukeboxCom::_GetTimeUnits() const {
    float votes[kNumTimeUnits] = {};
    Entity* const entity = _GetContextEntityPrototype();
    for (const Node& node : mNodes) {
        if (node.mClip.mId == gNullObjectId.mId) {
            continue;
        }
        const GameObject* const object = entity->GetObject(node.mClip);
        if (object == nullptr) {
            continue;
        }
        const CharClipCom* const clip = object->GetCom<CharClipCom>();
        if (clip == nullptr) {
            continue;
        }
        const CharClipResource* const resource = clip->mResource.Get();
        if (resource == nullptr || resource->Fail()) {
            continue;
        }
        votes[resource->GetUnits()] += node.mWeight;
    }
    int units = kTimeUnitsSeconds;
    float most = votes[kTimeUnitsSeconds] > 0.0f ? votes[kTimeUnitsSeconds] : 0.0f;
    for (int index = kTimeUnitsBeats; index < kNumTimeUnits; ++index) {
        if (votes[index] > most) {
            units = index;
        }
        most = votes[index] > most ? votes[index] : most;
    }
    return units;
}

// Reconstructed from eboot.elf at 0x804E30.
void ClipJukeboxCom::_Refresh() {
    Entity* const entity = _GetContextEntityPrototype();
    for (const Node& node : mNodes) {
        if (node.mClip.mId == gNullObjectId.mId) {
            continue;
        }
        const GameObject* const object = entity->GetObject(node.mClip);
        if (object == nullptr) {
            continue;
        }
        CharClipCom* const clip = object->GetCom<CharClipCom>();
        if (clip != nullptr) {
            clip->Refresh();
        }
    }
}

// Reconstructed from eboot.elf at 0x804F10.
void ClipJukeboxCom::_AddChild(int type) {
    static_cast<void>(type);
    Node node;
    mNodes._Insert(mNodes.size(), &node);
    const unsigned int count = mNodes.size();
    SetNodeWeight(count - 1, 1.0f / static_cast<float>(count));
}

// Reconstructed from eboot.elf at 0x805060.
void ClipJukeboxCom::SetNodeWeight(unsigned long index, float weight) {
    Node* const nodes = mNodes.data();
    const unsigned int count = mNodes.size();
    const float previous = nodes[index].mWeight;
    if (count == 1) {
        weight = 1.0f;
    }
    if (weight == previous) {
        return;
    }
    nodes[index].mWeight = weight;
    const float rest = 1.0f - weight;
    if (previous == 1.0f) {
        const float share = rest / static_cast<float>(count - 1UL);
        for (unsigned long other = 0; other < count; ++other) {
            if (other != index) {
                nodes[other].mWeight = share;
            }
        }
        return;
    }
    const float scale = rest / (1.0f - previous);
    for (unsigned long other = 0; other < count; ++other) {
        if (other != index) {
            nodes[other].mWeight *= scale;
        }
    }
}

// Reconstructed from eboot.elf at 0x805130.
void ClipJukeboxCom::_RemoveChild(unsigned long index) {
    SetNodeWeight(index, 0.0f);
    mNodes.Destruct(1, mNodes.ElementAt(index));
    if (static_cast<unsigned int>(index) < mNodes.mSize - 1) {
        mNodes.Move(mNodes.mSize - 1 - index, mNodes.ElementAt(index), mNodes.ElementAt(index + 1));
    }
    --mNodes.mSize;
}

// Reconstructed from eboot.elf at 0x8052A0. The path is a guarded local
// static (0x1ABECC8, guard 0x1ABED90).
const PropPath& ClipJukeboxCom::_GetChildObjectPath() const {
    static const PropPath path{
        PropPath::Node{} = Symbol("nodes"), PropPath::Node{} = 0UL, PropPath::Node{} = Symbol("clip")};
    return path;
}

// Reconstructed from eboot.elf at 0x8053C0. The edit mode makes no
// difference to a jukebox.
ClipJukeboxCom::Instance* ClipJukeboxCom::_CreateInstance(Entity* entity, bool editMode) {
    static_cast<void>(editMode);
    return new Instance(this, entity);
}

// Reconstructed from eboot.elf at 0x8054B0. A clip on the other side of
// seconds and beats converts at the tempo (beats per minute).
float ClipJukeboxCom::_GetScale(int units, float rate) const {
    if (units == kNumTimeUnits) {
        units = _GetTimeUnits();
    }
    Entity* const entity = _GetContextEntityPrototype();
    if (mNodes.size() == 0) {
        return 0.0f;
    }
    const float beatsPerSecond = rate * (1.0f / 60.0f);
    float totalWeight = 0.0f;
    float total = 0.0f;
    for (const Node& node : mNodes) {
        if (node.mClip.mId == gNullObjectId.mId) {
            continue;
        }
        const GameObject* const object = entity->GetObject(node.mClip);
        if (object == nullptr) {
            continue;
        }
        const CharClipCom* const clip = object->GetCom<CharClipCom>();
        if (clip == nullptr) {
            continue;
        }
        const CharClipResource* const resource = clip->mResource.Get();
        if (resource == nullptr || resource->Fail()) {
            continue;
        }
        const float end = resource->GetTimeFromFrame(resource->GetEndFrame(), kNumTimeUnits);
        const float start = resource->GetTimeFromFrame(resource->GetStartFrame(), kNumTimeUnits);
        float length = end - start;
        const TimeUnits clipUnits = resource->GetUnits();
        if (units != kNumTimeUnits && (units == kTimeUnitsBeats) != (clipUnits == kTimeUnitsBeats)) {
            length = clipUnits == kTimeUnitsBeats ? length / beatsPerSecond : length * beatsPerSecond;
        }
        totalWeight += node.mWeight;
        total += length / node.mSpeed * node.mWeight;
    }
    if (totalWeight == 0.0f) {
        return 0.0f;
    }
    return total / totalWeight;
}

// Reconstructed from eboot.elf at 0x805870. The driver is the one on the
// entity's root object.
ClipJukeboxCom::Instance::Instance(ClipJukeboxCom* tree, Entity* entity)
    : mTree(tree),
      mEntity(entity),
      mDriver(entity->GetRoot()->GetCom<AnimDriverCom>()),
      mWeight(0.0f),
      mClipWeights(),
      mCurrent(),
      mNext(),
      mCycles(0) {}

// Reconstructed from eboot.elf at 0x805950. Within the crossfade frames of
// the current clip's end the next clip starts and takes over linearly; once
// it has all the weight it becomes the current clip. A clip removed from the
// jukebox stops both.
void ClipJukeboxCom::Instance::Poll(float weight) {
    const unsigned long count = mTree->mNodes.size();
    if ((mCurrent.mIndex >= count && mCurrent.mAnim.mDriver != nullptr) ||
        (mNext.mIndex >= count && mNext.mAnim.mDriver != nullptr)) {
        mDriver->StopAnim(mCurrent.mAnim);
        mDriver->StopAnim(mNext.mAnim);
    }
    mWeight = weight;
    mClipWeights.clear();
    mClipWeights.resize(count);
    if (mCurrent.mAnim.mDriver == nullptr) {
        _StartClip(mCurrent, -1UL);
    }
    PlaybackInfo* const current = mDriver->GetPlaybackInfo(mCurrent.mAnim);
    if (current == nullptr) {
        return;
    }
    const float end = current->GetEndFrame();
    const float frame = current->GetCurrentFrame();
    const float crossfade = mTree->mCrossfadeFrames.GetValue(mEntity);
    float blend = 1.0f;
    if (crossfade != 0.0f) {
        const float low = crossfade < 0.0f ? crossfade : 0.0f;
        const float high = crossfade > 0.0f ? crossfade : 0.0f;
        const float remaining = end - frame;
        const float left = high < remaining ? high : (remaining > low ? remaining : low);
        blend = (crossfade - left) / crossfade;
    }
    if (blend > 0.0f && mNext.mAnim.mDriver == nullptr) {
        _StartClip(mNext, mCurrent.mIndex);
    }
    PlaybackInfo* const next = mDriver->GetPlaybackInfo(mNext.mAnim);
    if (next == nullptr) {
        if (mCurrent.mIndex < count) {
            mClipWeights[mCurrent.mIndex] = 1.0f;
        }
        current->mWeight = weight;
        current->mLooping = true;
        return;
    }
    current->mLooping = false;
    const Node* const nodes = mTree->mNodes.data();
    const float currentSpeed = nodes[mCurrent.mIndex].mSpeed;
    const float speed = (nodes[mNext.mIndex].mSpeed - currentSpeed) * blend + currentSpeed;
    if (mNext.mIndex < count) {
        mClipWeights[mNext.mIndex] = blend;
    }
    next->mWeight = blend * weight;
    next->mScale = speed;
    if (blend == 1.0f) {
        mCycles += current->mLoopCount + 1;
        mDriver->StopAnim(mCurrent.mAnim);
        mCurrent = mNext;
        mNext = Clip();
        return;
    }
    const float rest = 1.0f - blend;
    if (mCurrent.mIndex < count) {
        mClipWeights[mCurrent.mIndex] = rest;
    }
    current->mWeight = rest * weight;
    current->mScale = speed;
}

// Reconstructed from eboot.elf at 0x805EE0. A random pick leaves out the
// previous clip and rescales the others' weights; nothing follows a clip
// with all the weight.
void ClipJukeboxCom::Instance::_StartClip(Clip& clip, unsigned long previous) {
    clip.mIndex = -1UL;
    const ClipJukeboxCom* const tree = mTree;
    const unsigned long count = tree->mNodes.size();
    if (count == 0) {
        return;
    }
    unsigned long index;
    if (tree->mRandomize) {
        float previousWeight = 0.0f;
        if (previous != -1UL) {
            previousWeight = tree->mNodes[previous].mWeight;
            if (previousWeight == 1.0f) {
                return;
            }
        }
        const float scale = 1.0f / (1.0f - previousWeight);
        float pick = gRand.Float();
        index = -1UL;
        for (unsigned long candidate = 0; candidate < count; ++candidate) {
            if (candidate == previous) {
                continue;
            }
            const float share = scale * tree->mNodes[candidate].mWeight;
            if (pick < share) {
                index = candidate;
                break;
            }
            pick -= share;
        }
        if (index == -1UL) {
            return;
        }
    } else {
        index = previous == -1UL ? 0 : (previous + 1) % count;
    }
    const Node& node = tree->mNodes[index];
    if (node.mClip.mId == gNullObjectId.mId) {
        return;
    }
    const GameObject* const object = mEntity->GetObject(node.mClip);
    if (object == nullptr) {
        return;
    }
    const CharClipCom* const clipCom = object->GetCom<CharClipCom>();
    if (clipCom == nullptr) {
        return;
    }
    const CharClipResource* const resource = clipCom->mResource.Get();
    if (resource == nullptr || resource->Fail()) {
        return;
    }
    const ResourcePath path = resource->mPath;
    const AnimOptions options;
    clip.mAnim = mDriver->StartAnim(path, options);
    clip.mIndex = index;
    PlaybackInfo* const info = mDriver->GetPlaybackInfo(clip.mAnim);
    if (info != nullptr) {
        info->mScale = node.mSpeed;
    }
}

// Reconstructed from eboot.elf at 0x806150.
void ClipJukeboxCom::Instance::Stop() {
    mDriver->StopAnim(mCurrent.mAnim);
    mDriver->StopAnim(mNext.mAnim);
}

// Reconstructed from eboot.elf at 0x8061A0.
float ClipJukeboxCom::Instance::GetTimingPosition() {
    const PlaybackInfo* const info = mDriver->GetPlaybackInfo(mCurrent.mAnim);
    if (info == nullptr) {
        return 0.0f;
    }
    return static_cast<float>(mCycles) + info->GetTimingPosition();
}

// Reconstructed from eboot.elf at 0x8061F0.
void ClipJukeboxCom::Instance::SetTimingPosition(float position) {
    PlaybackInfo* const info = mDriver->GetPlaybackInfo(mCurrent.mAnim);
    if (info != nullptr) {
        info->mCurrentTime = (info->mEndTime - info->mBeginTime) * position + info->mBeginTime;
    }
}

// Reconstructed from eboot.elf at 0x8062D0.
Symbol ClipJukeboxCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8062E0.
Symbol ClipJukeboxCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8062F0.
int ClipJukeboxCom::CurrentRev() const {
    return const_cast<ClipJukeboxCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x806310.
bool ClipJukeboxCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x806340.
Component* ClipJukeboxCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x806350. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* ClipJukeboxCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<ClipJukeboxCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) ClipJukeboxCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x806540.
PropRegistry& ClipJukeboxCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x806550.
ComMetaData& ClipJukeboxCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x6FF200. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* ClipJukeboxCom::_Create() {
    return new ClipJukeboxCom();
}

// Reconstructed from eboot.elf at 0x6F8060. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void ClipJukeboxCom::Init() {
    InitComponentClass<ClipJukeboxCom>();
}
