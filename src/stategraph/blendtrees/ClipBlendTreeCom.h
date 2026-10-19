#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRef.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/blendtrees/BlendTreeBaseCom.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class AnimDriverCom;
class CharClipResource;
class Entity;
struct TagArgs;

// The blend tree that mixes clips and subtrees by a parameter
// (stategraph/ClipBlendTreeCom.o, 0x7FC520-0x80368F, after the map's build;
// class id "ClipBlendTree", aliases "CharBlendTree" and "BlendTree"). Each
// child takes full effect at its "value" of the parameter, a float property
// the tree reads through a PropRef; between two values the neighbours
// crossfade. The children's clips play through the context entity's
// AnimDriverCom, all at one phase: the tree loops over "period" or over
// the clips' own durations. A StateNodeClipBlendTree runs it. The vtable at
// 0x194F638 has 51 slots. The object is 304 bytes. Name not in the
// reference map; it follows the class id and the engine's "Com" suffix (the
// pool label "ClipBlendTree::Instance" suggests the original name
// ClipBlendTree).
class ClipBlendTreeCom : public BlendTreeBaseCom {
public:
    // One child ("nodes"; the struct type "ClipBlendTree::Node"). The
    // field names are not in the reference map; they follow the
    // properties.
    struct Node {
        Node();  // 0x7FC5D0

        // "type": a clip or a subtree (ChildType).
        int mType;
        // "object": the clip's object in the context entity or the
        // subtree's in the graph.
        GameObjectId mObject;
        // "value": the parameter value at which the child takes full
        // effect.
        float mValue;
        // "speed": the playback speed at full effect.
        float mSpeed;
    };

    // The tree running for an entity: weighs the hierarchy by the
    // parameter each frame and plays every clip with a weight, all at the
    // tree's phase. The vtable at 0x194F7E0 has nine slots. The object is
    // 160 bytes, from the small-block pool. The map's name is not known;
    // the pool label is "ClipBlendTree::Instance".
    class Instance : public BlendTreeBaseCom::Instance {
    public:
        static void* operator new(unsigned long size) {
            return PoolAlloc(size, "ClipBlendTree::Instance");
        }
        static void operator delete(void* allocation) {
            PoolFree(sizeof(Instance), allocation);
        }

        // The weight of a tree in the blend and of its children. Name not
        // in the reference map.
        struct TreeState {
            float mWeight;
            eastl::vector<float> mChildWeights;
        };

        // A clip in the blend: its playback, and the weight and speed the
        // trees give it this frame. Name not in the reference map.
        struct ClipState {
            ClipState();  // 0x7FF6A0

            AnimInstID mAnim;
            float mWeight;
            float mSpeed;
        };

        Instance(ClipBlendTreeCom* tree, Entity* entity, bool editMode);  // 0x7FF530
        // Slots 0-1: 0x8004F0, 0x800530.
        ~Instance() override {}

        void Poll(float weight) override;  // slot 2: 0x7FF6C0
        void Stop() override;              // slot 3: 0x7FFEA0
        // Slot 4 at 0x800580.
        BlendTreeBaseCom* GetBlendTree() override {
            return mTree;
        }
        float GetWeight(const BlendTreeBaseCom* tree) override;  // slot 5: 0x7FFEF0
        const eastl::vector<float>& GetChildWeights(const BlendTreeBaseCom* tree) override;  // slot 6: 0x7FFF60
        // Slots 7-8 at 0x800060 and 0x800070.
        float GetTimingPosition() override {
            return static_cast<float>(mCycles) + mPhase;
        }
        void SetTimingPosition(float position) override {
            mPhase = position;
        }

        // Records the tree's weight and its children's, and adds the
        // weight and speed of each clip under it to the clips in the
        // blend. Names not in the reference map.
        void _Evaluate(ClipBlendTreeCom* tree, float weight, float speed);  // 0x7FFAC0
        // The children's weights at the tree's parameter value: the two
        // whose values enclose it crossfade linearly; outside the values
        // the first or the last takes it all.
        void _ComputeChildWeights(
            const ClipBlendTreeCom* tree,
            bool editMode,
            eastl::vector<float>& weights);  // 0x800080

        // Field names are not in the reference map.
        ClipBlendTreeCom* mTree;
        Entity* mEntity;
        // The root object's driver, which plays the clips.
        AnimDriverCom* mDriver;
        // The parameter values are the editor's (gEditParameterValues).
        bool mEditMode;
        float mWeight;
        // The fraction of the current pass, and the passes completed.
        float mPhase;
        unsigned int mCycles;
        eastl::map<const BlendTreeBaseCom*, TreeState> mTreeStates;
        eastl::map<CharClipResource*, ClipState> mClips;
    };

    ClipBlendTreeCom();  // 0x7FC530
    // Copies the properties for an imprint; the edit value starts at 0.
    // Inlined into _Imprint (0x8002E0).
    ClipBlendTreeCom(const ClipBlendTreeCom& other)
        : BlendTreeBaseCom(other),
          mParameter(other.mParameter),
          mTimeUnits(other.mTimeUnits),
          mNormalizePeriod(other.mNormalizePeriod),
          mAlignPeriod(other.mAlignPeriod),
          mPeriod(other.mPeriod),
          mNodes(other.mNodes),
          mEditParameterValue(0.0f) {}
    ~ClipBlendTreeCom() override;  // slots 0-1: 0x7FE810, 0x8001D0

    Symbol GetId() const override;          // slot 4: 0x800260
    Symbol GetClassName() const override;   // slot 5: 0x800270
    int CurrentRev() const override;        // slot 7: 0x800280
    bool IsA(Symbol type) const override;   // slot 8: 0x8002A0
    Component* AsComponent() override;      // slot 9: 0x8002D0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8002E0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8004D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8004E0
    unsigned int _GetNumChildren() const override;                    // slot 41: 0x7FEE50
    int _GetChildType(unsigned long index) const override;            // slot 42: 0x7FEE60
    GameObjectId _GetChildObject(unsigned long index) const override;  // slot 43: 0x7FEE70
    // Slot 44: "time_units", or the context entity's driver's default
    // timeline for kNumTimeUnits.
    int _GetTimeUnits() const override;  // 0x7FEC50
    void _Refresh() override;            // slot 45: 0x7FECD0
    // Slot 46: a new child after the last, its value continuing the
    // spacing of the last two; a subtree gets a new object with a
    // ClipBlendTreeCom.
    void _AddChild(int type) override;  // 0x7FEE80
    // Slot 47: a subtree's object is destroyed with it.
    void _RemoveChild(unsigned long index) override;  // 0x7FEFA0
    // Slot 48: ("nodes" 0 "object").
    const PropPath& _GetChildObjectPath() const override;  // 0x7FF060
    Instance* _CreateInstance(Entity* entity, bool editMode) override;  // slot 49: 0x7FF180
    // Slot 50: the average length of the clips under the tree, each
    // divided by its speed.
    float _GetScale(int units, float rate) const override;  // 0x7FF1D0

    // The parameter's value: in the editor (`editMode`), the edit value
    // ("parameter_value"); otherwise the property the parameter names in
    // the entity, or 0. Name not in the reference map.
    float GetParameterValue(Entity* entity, bool editMode) const;  // 0x7FE8C0
    // Adds the lengths of the clips under the tree at the speed, in the
    // units, and counts them. Name not in the reference map.
    void _AccumulateClipLengths(float& total, unsigned long& count, int units, float speed) const;  // 0x7FF2D0
    // Whether the child the tag's path names ("nodes" i ...) is a clip.
    // No caller in this build; the tag functions inline it. Name not in
    // the reference map.
    bool IsClipNode(const TagArgs& args) const;  // 0x7FE8A0

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F7C20.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FEFF0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7FC5F0

    static Symbol sId;  // 0x1ABE6B0, "ClipBlendTree"
    // Also "ClipBlendTree". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABE6B8
    static PropRegistry sPropRegistry;  // 0x1ABE6C0
    static ComMetaData sMetaData;       // 0x1ABE760

    // Field names are not in the reference map; they follow the
    // properties.
    // "parameter": the float property that selects the blend.
    PropRef mParameter;
    // "time_units": the TimeUnits the clips play on; kNumTimeUnits takes
    // the driver's default timeline.
    int mTimeUnits;
    // "normalize_period" and "align_period": loop over mPeriod, and keep
    // the loop in phase with the clock.
    bool mNormalizePeriod;
    bool mAlignPeriod;
    // "period", in mTimeUnits.
    float mPeriod;
    // "nodes".
    PropArray<Node> mNodes;
    // "parameter_value" while the parameter is not set; once it is, the
    // edit values live in gEditParameterValues by parameter.
    float mEditParameterValue;
};

static_assert(sizeof(ClipBlendTreeCom::Node) == 16);
static_assert(offsetof(ClipBlendTreeCom::Instance, mTree) == 8);
static_assert(offsetof(ClipBlendTreeCom::Instance, mDriver) == 24);
static_assert(offsetof(ClipBlendTreeCom::Instance, mEditMode) == 32);
static_assert(offsetof(ClipBlendTreeCom::Instance, mWeight) == 36);
static_assert(offsetof(ClipBlendTreeCom::Instance, mPhase) == 40);
static_assert(offsetof(ClipBlendTreeCom::Instance, mCycles) == 44);
static_assert(offsetof(ClipBlendTreeCom::Instance, mTreeStates) == 48);
static_assert(offsetof(ClipBlendTreeCom::Instance, mClips) == 104);
static_assert(sizeof(ClipBlendTreeCom::Instance) == 160);
static_assert(offsetof(ClipBlendTreeCom, mParameter) == 24);
static_assert(offsetof(ClipBlendTreeCom, mTimeUnits) == 240);
static_assert(offsetof(ClipBlendTreeCom, mNormalizePeriod) == 244);
static_assert(offsetof(ClipBlendTreeCom, mAlignPeriod) == 245);
static_assert(offsetof(ClipBlendTreeCom, mPeriod) == 248);
static_assert(offsetof(ClipBlendTreeCom, mNodes) == 256);
static_assert(offsetof(ClipBlendTreeCom, mEditParameterValue) == 296);
static_assert(sizeof(ClipBlendTreeCom) == 304);
