#pragma once

#include <new>

#include "stategraph/nodes/StateNodeBlendTreeBase.h"
#include "utl/text/Symbol.h"

// The node that runs the ClipBlendTreeCom on its object (class id
// "StateNodeClipBlendTree", alias "StateNodeBlendTree"; hotkey "b"; after
// the map's build). Its registry and statics are in
// stategraph/StateNodeBlendTreeBase.o (_Init at 0x7181C0); every method is
// inline and emitted in stategraph/StateGraph.o. The vtable at 0x1939EA8
// has 56 slots; the StateNodeTiming one at 0x193A078 holds the base's
// thunks. The object is 160 bytes.
class StateNodeClipBlendTree : public StateNodeBlendTreeBase {
public:
    StateNodeClipBlendTree() {}
    // Copies the node for an imprint. Inlined into _Imprint (0x6FF6C0).
    StateNodeClipBlendTree(const StateNodeClipBlendTree& other) : StateNodeBlendTreeBase(other) {}
    // Slots 0-1 at 0x6FF510 and 0x6FF5A0.
    ~StateNodeClipBlendTree() override {}

    // Slot 4 at 0x6FF640.
    Symbol GetId() const override {
        return sId;
    }
    // Slot 5 at 0x6FF650.
    Symbol GetClassName() const override {
        return sClassName;
    }
    // Slot 7 at 0x6FF660.
    int CurrentRev() const override {
        return const_cast<StateNodeClipBlendTree*>(this)->_GetPropRegistry().mCurrentRev;
    }
    // Slot 8 at 0x6FF680.
    bool IsA(Symbol type) const override {
        for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
            if (metadata->mId == type) {
                return true;
            }
        }
        return false;
    }
    // Slot 9 at 0x6FF6B0.
    Component* AsComponent() override {
        return this;
    }
    // Slot 10 at 0x6FF6C0. The copy goes at the buffer's next 8-byte
    // boundary, and its properties follow it. The map's _Imprint(char*,
    // Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override {
        ScopedImprint scope;
        auto* copy =
            reinterpret_cast<StateNodeClipBlendTree*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
        Component* result = nullptr;
        if (imprint != nullptr) {
            new (copy) StateNodeClipBlendTree(*this);
            copy->mImprinted = true;
            *imprint = copy;
            result = copy;
        }
        return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
    }
    // Slots 22-23 at 0x6FF870 and 0x6FF880.
    PropRegistry& _GetPropRegistry() override {
        return sPropRegistry;
    }
    ComMetaData& _GetMetaData() override {
        return sMetaData;
    }

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F8930.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FF490.
    static Component* _Create();
    // The node requires the tree on its object.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7181C0

    static Symbol sId;  // 0x1AB3468, "StateNodeClipBlendTree"
    // Also "StateNodeClipBlendTree". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB3470
    static PropRegistry sPropRegistry;  // 0x1AB3480
    static ComMetaData sMetaData;       // 0x1AB3520
};

static_assert(sizeof(StateNodeClipBlendTree) == 160);
