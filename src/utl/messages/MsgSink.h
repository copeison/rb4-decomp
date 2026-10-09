#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "utl/containers/LinkedList.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class DataArray;

// A receiver of script messages (utl/MsgSink.o). Only the vtable layout the
// reconstructed sinks override is declared: the destructors, Handle, and a
// fourth slot whose default at 0x8E60 returns null.
class MsgSink {
public:
    virtual ~MsgSink() {}
    // Slot 2: handles the message, returning its result.
    virtual DataNode Handle(DataArray* msg, bool warn) = 0;
    // Slot 3 at 0x8E60: the object behind the sink. Name not in the
    // reference map, which names no MsgSink methods. Weak: every vtable
    // found, MsgSource-derived ones included, keeps the shared null-returning
    // body, and no caller was identified.
    virtual void* GetSinkObject() {
        return nullptr;
    }
};

static_assert(sizeof(MsgSink) == 8);

// A sender of script events to the sinks subscribed to them (utl/MsgSink.o,
// 0x248AB0-0x249610). The vtable at 0x18EF390 has seven slots; Entity is
// built on it. This build keeps a much smaller object than the map's: no
// property sinks, and the subscriptions are owned by the subscribers.
class MsgSource : public MsgSink {
public:
    // One sink's subscription to an event. The subscriber owns it; the
    // source links it into the event's list. The map names the type; the
    // field names are not in the reference map.
    struct EventSinkElem {
        // Reaches the link of a subscription in its event's list.
        class ListNode {
        public:
            static LinkedList::Node& ToNode(EventSinkElem& elem) {
                return elem.mLink;
            }
            static EventSinkElem* FromNode(LinkedList::Node* node) {
                return reinterpret_cast<EventSinkElem*>(
                    reinterpret_cast<char*>(node) - offsetof(EventSinkElem, mLink));
            }
        };

        MsgSink* mSink;
        // The source the subscription is linked in, or null.
        MsgSource* mSource;
        // The message type the sink receives the event as; empty to keep
        // the event's own type.
        Symbol mHandler;
        LinkedList::Node mLink;
    };

    // The subscriptions to one event, from the small-block pool. The map
    // names the type; the field names are not in the reference map.
    struct EventSink {
        POOL_OVERLOAD(EventSink)

        // Reaches the link of an event in the source's list.
        class ListNode {
        public:
            static LinkedList::Node& ToNode(EventSink& sink) {
                return sink.mLink;
            }
            static EventSink* FromNode(LinkedList::Node* node) {
                return reinterpret_cast<EventSink*>(
                    reinterpret_cast<char*>(node) - offsetof(EventSink, mLink));
            }
        };

        // The event's message type; empty for the sinks of every event.
        Symbol mEvent;
        LinkedList::List<EventSinkElem, EventSinkElem::ListNode> mSinks;
        LinkedList::Node mLink;
    };

    // Starts without subscriptions. Inlined into Entity's constructor
    // (0xEBB70).
    MsgSource();  // 0x248AB0
    // Slots 0-1: 0x248AD0, 0x248C40. Unlinks every subscription and frees
    // the events.
    ~MsgSource() override;
    // Slot 2: subscribes for "addsink_private" and exports every other
    // message to its sinks. Unhandled unless it subscribed.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x2494C0
    // Slot 4: links the subscription into the event's list, unlinking it
    // from its old source first. An empty handler keeps the event's type.
    // The sinks of every event (the empty event) are kept first. The map's
    // signature is AddSink(MsgSink*, Symbol, Symbol); this build adds the
    // subscription.
    virtual void AddSink(EventSinkElem* elem, MsgSink* sink, Symbol event, Symbol handler);  // 0x248D10
    // Slot 5: sends the message to the sinks of every event, then to those
    // of its type (node 1). True when a sink handled it.
    virtual bool Export(DataArray* msg);  // 0x248ED0
    // Slot 6: unlinks the subscription. The map's signature is
    // RemoveSink(MsgSink*, Symbol, Symbol); this build takes the
    // subscription.
    virtual void RemoveSink(EventSinkElem* elem);  // 0x248EA0

    // Sends the message to the event's sinks in turn, each under its
    // handler's type, and restores the type. True when a sink handled it.
    // The map's signature is ExportToSink(DataArray*, MsgSource::EventSink*).
    static bool ExportToSink(EventSink* sink, DataArray* msg);  // 0x248F70
    // The "addsink_private" handler: the message's nodes 2-5 are the
    // subscription, the sink, the event and the handler.
    DataNode OnAddSink(DataArray* msg);  // 0x249420
    // Unlinks every subscription and frees the events; Entity::_Destroy
    // calls it first. Name not in the reference map.
    void RemoveAllSinks();  // 0x248B90

    // Field name not in the reference map.
    LinkedList::List<EventSink, EventSink::ListNode> mEventSinks;
};

static_assert(sizeof(MsgSource::EventSinkElem) == 40);
static_assert(offsetof(MsgSource::EventSinkElem, mLink) == 24);
static_assert(sizeof(MsgSource::EventSink) == 40);
static_assert(offsetof(MsgSource::EventSink, mLink) == 24);
static_assert(offsetof(MsgSource, mEventSinks) == 8);
static_assert(sizeof(MsgSource) == 24);

class CritSec;

// A MsgSource whose subscriptions and exports take a lock when it has one.
// PlatformMgr is built on it. Its overrides sit in the second utl block
// (0x11818D0-0x11819C0). Name not in the reference map; not reconstructed.
class SyncMsgSource : public MsgSource {
public:
    // Inlined into its users, for example KeyboardInitCommon (0x3A18A0).
    explicit SyncMsgSource(CritSec* lock) : mLock(lock) {}

    // Slot 4.
    void AddSink(EventSinkElem* elem, MsgSink* sink, Symbol event, Symbol handler) override;  // 0x11818D0
    // Slot 5.
    bool Export(DataArray* msg) override;  // 0x11819C0
    // Slot 6.
    void RemoveSink(EventSinkElem* elem) override;  // 0x1181960

    CritSec* mLock;  // Name not in the reference map.
};

static_assert(sizeof(SyncMsgSource) == 32);
