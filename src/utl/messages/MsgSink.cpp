#include "utl/messages/MsgSink.h"

#include "utl/data/DataArray.h"

// Reconstructed from eboot.elf at 0x248AB0.
MsgSource::MsgSource() {}

// Reconstructed from eboot.elf at 0x248AD0. The binary inlines
// RemoveAllSinks.
MsgSource::~MsgSource() {
    RemoveAllSinks();
}

// Reconstructed from eboot.elf at 0x248B90. The subscriptions stay with
// their subscribers, unlinked and without a source.
void MsgSource::RemoveAllSinks() {
    while (!mEventSinks.empty()) {
        EventSink& sink = mEventSinks.front();
        while (!sink.mSinks.empty()) {
            EventSinkElem& elem = sink.mSinks.front();
            elem.mSource = nullptr;
            sink.mSinks.remove(elem);
        }
        delete &sink;
    }
}

// Reconstructed from eboot.elf at 0x248D10. The new subscription goes to
// the end of its event's list.
void MsgSource::AddSink(EventSinkElem* elem, MsgSink* sink, Symbol event, Symbol handler) {
    if (handler == Symbol()) {
        handler = event;
    }
    if (elem->mSource != nullptr) {
        elem->mSource->RemoveSink(elem);
    }
    elem->mSource = this;
    elem->mSink = sink;
    elem->mHandler = handler;
    EventSink* found = nullptr;
    for (EventSink& candidate : mEventSinks) {
        if (candidate.mEvent == event) {
            found = &candidate;
            break;
        }
    }
    if (found == nullptr) {
        found = new EventSink;
        found->mEvent = event;
        if (event == Symbol()) {
            mEventSinks.push_front(*found);
        } else {
            mEventSinks.push_back(*found);
        }
    }
    found->mSinks.push_back(*elem);
}

// Reconstructed from eboot.elf at 0x248EA0.
void MsgSource::RemoveSink(EventSinkElem* elem) {
    elem->mLink.Remove();
    elem->mSource = nullptr;
}

// Reconstructed from eboot.elf at 0x248ED0. The sinks of every event come
// first in the list; the search for the message's event starts after them.
bool MsgSource::Export(DataArray* msg) {
    const Symbol event = msg->Node(1).LiteralSym(msg);
    auto sink = mEventSinks.begin();
    if (sink == mEventSinks.end()) {
        return false;
    }
    bool handled = false;
    if (sink->mEvent == Symbol()) {
        handled = ExportToSink(&*sink, msg);
        ++sink;
    }
    for (; sink != mEventSinks.end(); ++sink) {
        if (sink->mEvent == event) {
            return ExportToSink(&*sink, msg) | handled;
        }
    }
    return handled;
}

// Reconstructed from eboot.elf at 0x248F70. The subscriptions move to a
// local list and back one at a time, so a sink may unsubscribe itself, or
// subscribe others, while it handles the message.
bool MsgSource::ExportToSink(EventSink* sink, DataArray* msg) {
    const DataNode type = msg->Node(1);
    LinkedList::List<EventSinkElem, EventSinkElem::ListNode> pending;
    pending.splice(sink->mSinks);
    bool handled = false;
    while (!pending.empty()) {
        EventSinkElem& elem = pending.front();
        pending.remove(elem);
        sink->mSinks.push_back(elem);
        if (elem.mHandler == Symbol()) {
            msg->Node(1) = type;
        } else {
            msg->Node(1) = DataNode(elem.mHandler);
        }
        if (elem.mSink->Handle(msg, false) != DataNode()) {
            handled = true;
        }
    }
    msg->Node(1) = type;
    return handled;
}

// Reconstructed from eboot.elf at 0x249420.
DataNode MsgSource::OnAddSink(DataArray* msg) {
    AddSink(
        static_cast<EventSinkElem*>(msg->Evaluate(2).mValue.object),
        static_cast<MsgSink*>(msg->Evaluate(3).LiteralSink(msg)),
        msg->Sym(4),
        msg->Sym(5));
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x2494C0. The binary inlines OnAddSink.
DataNode MsgSource::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol addSink;
    if (addSink == Symbol()) {
        addSink = Symbol("addsink_private");
    }
    if (type == addSink) {
        return OnAddSink(msg);
    }
    Export(msg);
    return DataNode();
}
