#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/blendtrees/BlendTreeBaseCom.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class AnimDriverCom;
class Entity;

// The blend tree that plays its clips one after another, crossfading
// between them (stategraph/ClipJukeboxCom.o, 0x803690-0x807E7F, after the
// map's build; class id "ClipJukebox", aliases "CharBlendTree" and
// "BlendTreeCom"). The clips play in their order or, with "randomize", in
// a random order by their weights, never the same clip twice in a row;
// each crossfades into the next over "crossfade_frames" before its end. A
// StateNodeClipJukebox runs it. The vtable at 0x19500A8 has 51 slots. The
// object is 304 bytes. Name not in the reference map; it follows the class
// id and the engine's "Com" suffix (the pool label
// "ClipJukebox::Instance" suggests the original name ClipJukebox).
class ClipJukeboxCom : public BlendTreeBaseCom {
public:
    // One clip ("nodes"; the struct type "ClipJukebox::Node"). The field
    // names are not in the reference map; they follow the properties.
    struct Node {
        Node();  // 0x803720

        // "clip": the clip's object in the context entity.
        GameObjectId mClip;
        // "weight": the clip's chance of being picked next; the weights add
        // up to 1 (SetNodeWeight).
        float mWeight;
        // "speed": the playback speed.
        float mSpeed;
    };

    // The jukebox running for an entity: plays the current clip and, while
    // it ends, crossfades into the next. The vtable at 0x1950250 has nine
    // slots. The object is 128 bytes, from the small-block pool. The map's
    // name is not known; the pool label is "ClipJukebox::Instance".
    class Instance : public BlendTreeBaseCom::Instance {
    public:
        static void* operator new(unsigned long size) {
            return PoolAlloc(size, "ClipJukebox::Instance");
        }
        static void operator delete(void* allocation) {
            PoolFree(sizeof(Instance), allocation);
        }

        // A clip the instance plays: its index in the jukebox, -1 for none,
        // and its playback. Name and field names not in the reference map.
        struct Clip {
            Clip() : mIndex(-1UL), mAnim() {}

            unsigned long mIndex;
            AnimInstID mAnim;
        };

        // Inlined into _CreateInstance (0x8053C0).
        Instance(ClipJukeboxCom* tree, Entity* entity);  // 0x805870
        // Slots 0-1: 0x806560, 0x806590.
        ~Instance() override {}

        void Poll(float weight) override;  // slot 2: 0x805950
        void Stop() override;              // slot 3: 0x806150
        // Slot 4 at 0x8065E0.
        BlendTreeBaseCom* GetBlendTree() override {
            return mTree;
        }
        // Slots 5-6 at 0x806180 and 0x806190: the jukebox has no subtrees.
        float GetWeight(const BlendTreeBaseCom* tree) override {
            static_cast<void>(tree);
            return mWeight;
        }
        const eastl::vector<float>& GetChildWeights(const BlendTreeBaseCom* tree) override {
            static_cast<void>(tree);
            return mClipWeights;
        }
        // Slots 7-8: the clips played before the current one, and its
        // position.
        float GetTimingPosition() override;  // 0x8061A0
        void SetTimingPosition(float position) override;  // 0x8061F0

        // Starts the clip that follows `previous` (-1 for the first) in
        // `clip`: the next in order, or a random one by the weights other
        // than `previous`. Name not in the reference map.
        void _StartClip(Clip& clip, unsigned long previous);  // 0x805EE0

        // Field names are not in the reference map.
        ClipJukeboxCom* mTree;
        Entity* mEntity;
        // The root object's driver, which plays the clips.
        AnimDriverCom* mDriver;
        float mWeight;
        // The weight of each clip in the blend.
        eastl::vector<float> mClipWeights;
        Clip mCurrent;
        // The clip being crossfaded into.
        Clip mNext;
        // The passes of the clips that finished.
        unsigned int mCycles;
    };

    ClipJukeboxCom();  // 0x803690
    // Copies the properties for an imprint. Inlined into _Imprint
    // (0x806350).
    ClipJukeboxCom(const ClipJukeboxCom& other)
        : BlendTreeBaseCom(other),
          mCrossfadeFrames(other.mCrossfadeFrames),
          mRandomize(other.mRandomize),
          mNodes(other.mNodes) {}
    ~ClipJukeboxCom() override;  // slots 0-1: 0x804BB0, 0x806240

    Symbol GetId() const override;          // slot 4: 0x8062D0
    Symbol GetClassName() const override;   // slot 5: 0x8062E0
    int CurrentRev() const override;        // slot 7: 0x8062F0
    bool IsA(Symbol type) const override;   // slot 8: 0x806310
    Component* AsComponent() override;      // slot 9: 0x806340
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x806350
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x806540
    ComMetaData& _GetMetaData() override;       // slot 23: 0x806550
    unsigned int _GetNumChildren() const override;                    // slot 41: 0x804C40
    // Slot 42: every child is a clip.
    int _GetChildType(unsigned long index) const override;            // 0x804C50
    GameObjectId _GetChildObject(unsigned long index) const override;  // slot 43: 0x804C60
    // Slot 44: the timeline most of the weight is on.
    int _GetTimeUnits() const override;  // 0x804C70
    void _Refresh() override;            // slot 45: 0x804E30
    // Slot 46: a new clip after the last, taking an equal share of the
    // weight.
    void _AddChild(int type) override;  // 0x804F10
    // Slot 47: the others take the clip's weight.
    void _RemoveChild(unsigned long index) override;  // 0x805130
    // Slot 48: ("nodes" 0 "clip").
    const PropPath& _GetChildObjectPath() const override;  // 0x8052A0
    Instance* _CreateInstance(Entity* entity, bool editMode) override;  // slot 49: 0x8053C0
    // Slot 50: the clips' lengths, each divided by its speed, averaged by
    // their weights.
    float _GetScale(int units, float rate) const override;  // 0x8054B0

    // Sets the clip's weight and scales the others' so the weights still
    // add up to 1; a single clip always has 1. Inlined into its users. Name
    // not in the reference map.
    void SetNodeWeight(unsigned long index, float weight);  // 0x805060

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F8060.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FF200.
    static Component* _Create();
    // The registry's revision is 1: a jukebox saved before had a plain
    // float "crossfade_frames".
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x803740

    static Symbol sId;  // 0x1ABEA70, "ClipJukebox"
    // Also "ClipJukebox". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABEA78
    static PropRegistry sPropRegistry;  // 0x1ABEA80
    static ComMetaData sMetaData;       // 0x1ABEB20

    // Field names are not in the reference map; they follow the
    // properties.
    // "crossfade_frames": 10 by default.
    DrivenProp<float> mCrossfadeFrames;
    // "randomize": true by default.
    bool mRandomize;
    // "nodes".
    PropArray<Node> mNodes;
};

static_assert(sizeof(ClipJukeboxCom::Node) == 12);
static_assert(offsetof(ClipJukeboxCom::Instance, mTree) == 8);
static_assert(offsetof(ClipJukeboxCom::Instance, mDriver) == 24);
static_assert(offsetof(ClipJukeboxCom::Instance, mWeight) == 32);
static_assert(offsetof(ClipJukeboxCom::Instance, mClipWeights) == 40);
static_assert(offsetof(ClipJukeboxCom::Instance, mCurrent) == 72);
static_assert(offsetof(ClipJukeboxCom::Instance, mNext) == 96);
static_assert(offsetof(ClipJukeboxCom::Instance, mCycles) == 120);
static_assert(sizeof(ClipJukeboxCom::Instance) == 128);
static_assert(offsetof(ClipJukeboxCom, mCrossfadeFrames) == 24);
static_assert(offsetof(ClipJukeboxCom, mRandomize) == 256);
static_assert(offsetof(ClipJukeboxCom, mNodes) == 264);
static_assert(sizeof(ClipJukeboxCom) == 304);
