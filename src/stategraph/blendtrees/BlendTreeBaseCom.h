#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropPath.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class Entity;

// The base of the blend tree components, which mix a set of animations or
// subtrees by some criteria (stategraph/BlendTreeBaseCom.o,
// 0x7FB540-0x7FC51F, after the map's build; class id "BlendTreeBase"). A
// blend tree sits on its own object of a state graph's entity; a
// StateNodeBlendTreeBase on the same object runs it, and its children are
// clips (CharClipCom) of the context entity or subtrees on other objects of
// the graph. The class is never created itself. The vtable at 0x194F490 has
// 51 slots. The object is 24 bytes. Name not in the reference map; it
// follows the class id and the engine's "Com" suffix. The metadata names
// the class "BlendTree" (InitSuperclass), which may be the original name.
class BlendTreeBaseCom : public Component {
public:
    // The kinds of children. The values are the binary's; the names are
    // not in the reference map and follow the editor labels ("Clip",
    // "Subtree").
    enum ChildType : int {
        kChildClip = 0,
        kChildBlendTree = 1,
    };

    // A running blend tree: plays the tree's clips, weighted by the tree,
    // through the context entity's AnimDriverCom. A StateNodeBlendTreeBase
    // instance owns one while its node runs. Each subclass derives its own
    // (ClipBlendTreeCom::Instance, ClipJukeboxCom::Instance); this class
    // only declares the nine slots they share, and no vtable of its own is
    // emitted. Name not in the reference map; the pool labels of the
    // subclasses are "ClipBlendTree::Instance" and "ClipJukebox::Instance".
    class Instance {
    public:
        // Slots 0-1.
        virtual ~Instance() {}
        // Slot 2: plays the clips at the weight for the frame.
        virtual void Poll(float weight) = 0;
        // Slot 3: stops every clip the instance plays.
        virtual void Stop() = 0;
        // Slot 4: the tree the instance runs.
        virtual BlendTreeBaseCom* GetBlendTree() = 0;
        // Slot 5: the weight the tree (the instance's or one of its
        // subtrees) has in the blend.
        virtual float GetWeight(const BlendTreeBaseCom* tree) = 0;
        // Slot 6: the weights of the tree's children in the blend.
        virtual const eastl::vector<float>& GetChildWeights(const BlendTreeBaseCom* tree) = 0;
        // Slots 7-8: the position on the tree's timing: completed passes
        // plus the fraction of the current one.
        virtual float GetTimingPosition() = 0;
        virtual void SetTimingPosition(float position) = 0;
    };

    // Slots 0-1: 0x7FC150 (a jump to Component's destructor) and 0x7FC160.
    ~BlendTreeBaseCom() override {}

    Symbol GetId() const override;          // slot 4: 0x7FC180
    Symbol GetClassName() const override;   // slot 5: 0x7FC190
    int CurrentRev() const override;        // slot 7: 0x7FC1A0
    bool IsA(Symbol type) const override;   // slot 8: 0x7FC1C0
    Component* AsComponent() override;      // slot 9: 0x7FC1F0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7FC200
    // Slot 20: destroying the tree's object (not the whole entity)
    // destroys its subtrees' objects too.
    void _PreDestroy(DestroyType type) override;  // 0x7FB6A0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x7FC320
    ComMetaData& _GetMetaData() override;       // slot 23: 0x7FC330

    // The number of children (slot 41) and the child at the index: its
    // ChildType (slot 42, -1 here) and its object (slot 43). Names not in
    // the reference map.
    virtual unsigned int _GetNumChildren() const;                    // 0x7FC340
    virtual int _GetChildType(unsigned long index) const;            // 0x7FC350
    virtual GameObjectId _GetChildObject(unsigned long index) const;  // 0x7FC360
    // Slot 44: the TimeUnits the tree plays its clips on. Name not in the
    // reference map.
    virtual int _GetTimeUnits() const;  // 0x7FC370
    // Slot 45 at 0x7FC380: rebuilds the tree's clips (CharClipCom::Refresh);
    // nothing here. Name not in the reference map; the evidence is weak.
    virtual void _Refresh() {}
    // Slots 46-47 at 0x7FC390 and 0x7FC3A0: adds a child of the ChildType
    // and removes the child at the index; nothing here. Names not in the
    // reference map; they follow the "add_clip" and "-" editor actions.
    virtual void _AddChild(int type) {
        static_cast<void>(type);
    }
    virtual void _RemoveChild(unsigned long index) {
        static_cast<void>(index);
    }
    // Slot 48: the path of the first child's object property, such as
    // ("nodes" 0 "object"); empty here. Name not in the reference map; no
    // caller was found.
    virtual const PropPath& _GetChildObjectPath() const;  // 0x7FC3B0
    // Slot 49: a new instance of the tree for the entity, in the editor's
    // preview when `editMode` is set; none here. Name not in the reference
    // map.
    virtual Instance* _CreateInstance(Entity* entity, bool editMode);  // 0x7FC440
    // Slot 50: the length of the tree's clips in the units, with beats
    // converted at the tempo `rate` (beats per minute); zero here. Name not
    // in the reference map.
    virtual float _GetScale(int units, float rate) const;  // 0x7FC450

    // The public faces of the slots. The ones that take an index first
    // call _GetNumChildren, whose result a compiled-out assertion used.
    // Names not in the reference map.
    unsigned int GetNumChildren() const {
        return _GetNumChildren();
    }
    int GetChildType(unsigned long index) const;      // 0x7FBAD0
    bool IsChildBlendTree(unsigned long index) const;  // 0x7FBB00
    bool IsChildClip(unsigned long index) const;       // 0x7FBB30
    GameObjectId GetChildObject(unsigned long index) const;  // 0x7FBB60
    int GetTimeUnits() const;                          // 0x7FBB70
    void Refresh();                                    // 0x7FBB80
    // The subtree component on the child's object, or null.
    BlendTreeBaseCom* GetChildBlendTree(unsigned long index) const;  // 0x7FBB90
    // Adds a child and tells the editor, which selects it.
    void AddChild(int type);  // 0x7FBC70
    // Removes the child and tells the editor.
    void RemoveChild(unsigned long index);  // 0x7FBF10
    const PropPath& GetChildObjectPath() const;  // 0x7FBF40
    // Whether the tree is on the object or holds a subtree that is.
    bool ContainsObject(const GameObject* object) const;  // 0x7FBF50
    Instance* CreateInstance(Entity* entity, bool editMode);  // 0x7FC0C0
    float GetScale(int units, float rate) const;  // 0x7FC0D0
    // Appends the objects of the subtrees, and with `recursive` theirs too.
    // The node's StateNode::GetChildNodes. Name not in the reference map.
    void GetChildObjects(eastl::vector<GameObjectId>& children, bool recursive) const;  // 0x7FB780
    // Whether the tree is the top of its hierarchy: its object holds a
    // StateNode (the StateNodeBlendTreeBase that runs it). The editor shows
    // the timing properties only there. Name not in the reference map.
    bool IsRootBlendTree() const;  // 0x7FC0E0

    // The entity the graph is inlined into, whose clips the tree plays, or
    // null, as StateBase::_GetContextEntityPrototype. Name not in the
    // reference map.
    Entity* _GetContextEntityPrototype() const;  // 0x7FB9E0
    // Sends ("blend_tree_modified" <object id> <added>) to the entity's
    // sinks so the editor redraws the tree; `added` is set when a child was
    // added. Name not in the reference map.
    void _SendModified(bool added) const;  // 0x7FBCA0

    // Registers the class (InitComponentClass): it may not be created and
    // ends the superclass walk. Emitted in stategraph/StateGraph.o at
    // 0x6F7860.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FEF80.
    static Component* _Create();
    // The description and category; the class has no properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7FB540
    // Makes BlendTreeBaseCom the superclass of a subclass's metadata, as
    // StateNode::_InitAsSuperclass does. Emitted in
    // stategraph/ClipBlendTreeCom.o. Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x7FE760

    static Symbol sId;  // 0x1ABE350, "BlendTreeBase"
    // Also "BlendTreeBase". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABE358
    static PropRegistry sPropRegistry;  // 0x1ABE360
    static ComMetaData sMetaData;       // 0x1ABE400
};

static_assert(sizeof(BlendTreeBaseCom) == 24);
static_assert(sizeof(BlendTreeBaseCom::Instance) == 8);
