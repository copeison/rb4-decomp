#pragma once

#include <new>

#include "stategraph/nodes/StateNodeBlendTreeBase.h"
#include "utl/text/Symbol.h"

// The node that runs the ClipJukeboxCom on its object (class id
// "StateNodeClipJukebox"; hotkey "j"; after the map's build). Its registry
// and statics are in stategraph/StateNodeBlendTreeBase.o (_Init at
// 0x7186C0); every method is inline and emitted in stategraph/StateGraph.o.
// The vtable at 0x193A0B8 has 56 slots; the StateNodeTiming one at
// 0x193A288 holds the base's thunks. The object is 160 bytes.
class StateNodeClipJukebox : public StateNodeBlendTreeBase {
public:
    StateNodeClipJukebox() {}
    // Copies the node for an imprint. Inlined into _Imprint (0x6FFB60).
    StateNodeClipJukebox(const StateNodeClipJukebox& other) : StateNodeBlendTreeBase(other) {}
    // Slots 0-1 at 0x6FF9B0 and 0x6FFA40.
    ~StateNodeClipJukebox() override {}

    // Slot 4 at 0x6FFAE0.
    Symbol GetId() const override {
        return sId;
    }
    // Slot 5 at 0x6FFAF0.
    Symbol GetClassName() const override {
        return sClassName;
    }
    // Slot 7 at 0x6FFB00.
    int CurrentRev() const override {
        return const_cast<StateNodeClipJukebox*>(this)->_GetPropRegistry().mCurrentRev;
    }
    // Slot 8 at 0x6FFB20.
    bool IsA(Symbol type) const override {
        for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
            if (metadata->mId == type) {
                return true;
            }
        }
        return false;
    }
    // Slot 9 at 0x6FFB50.
    Component* AsComponent() override {
        return this;
    }
    // Slot 10 at 0x6FFB60. The copy goes at the buffer's next 8-byte
    // boundary, and its properties follow it. The map's _Imprint(char*,
    // Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override {
        ScopedImprint scope;
        auto* copy =
            reinterpret_cast<StateNodeClipJukebox*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
        Component* result = nullptr;
        if (imprint != nullptr) {
            new (copy) StateNodeClipJukebox(*this);
            copy->mImprinted = true;
            *imprint = copy;
            result = copy;
        }
        return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
    }
    // Slots 22-23 at 0x6FFD10 and 0x6FFD20.
    PropRegistry& _GetPropRegistry() override {
        return sPropRegistry;
    }
    ComMetaData& _GetMetaData() override {
        return sMetaData;
    }

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F8DC0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FF930.
    static Component* _Create();
    // The node requires the jukebox on its object.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7186C0

    static Symbol sId;  // 0x1AB36C8, "StateNodeClipJukebox"
    // Also "StateNodeClipJukebox". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB36D0
    static PropRegistry sPropRegistry;  // 0x1AB36E0
    static ComMetaData sMetaData;       // 0x1AB3780
};

static_assert(sizeof(StateNodeClipJukebox) == 160);
