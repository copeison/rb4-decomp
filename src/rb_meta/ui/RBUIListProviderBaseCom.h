#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "ui/lists/UIListData.h"
#include "utl/data/DataNode.h"

class DataArray;
class InstanceCom;
class UIListCom;
class UISelectMsg;

// The base of the game's list providers (rb_meta/RBUIListProviderBaseCom.o,
// 0xCB5B10-0xCB64BF). A subclass builds the rows (_BuildListData) and the
// base installs them on a UIListCom: the object's own, the one on the root
// of an instanced entity, or the one the subclass finds (_GetListCom). It
// keeps the list's focused row and tells the subclass when it changes and
// when a row is selected. The class is abstract: the identity slots are
// pure, and it has no statics or registration of its own; the map's
// _InitBase<T> template registers the subclasses' shared properties. The
// vtable at 0x19784F0 has 46 slots: Component's 41 and five of the class's
// own. The object is 200 bytes. Field names are not in the reference map.
class RBUIListProviderBaseCom : public Component {
public:
    RBUIListProviderBaseCom();  // 0xCB5B10
    // Slots 0-1: 0xCB5B70, 0xCB5C20.
    ~RBUIListProviderBaseCom() override;

    // Slot 2: "ui_select" goes to OnMsg; other messages are unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xCB6310
    // Slot 20: empty in this build. The map's _PreDestroy has no entry for
    // the class; _UninstallSelf is uncalled.
    void _PreDestroy(DestroyType type) override;  // 0xCB6240
    // Slot 29: finds the object's UIListCom, or else the instance component
    // whose entity's root holds it. The map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0xCB5C40
    // Slot 31: fills the list, finding it first when it is not on the
    // object. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xCB5D20
    // Slot 33: _PollImpl that queues the post-poll. The map's
    // _Poll(ObjPtr const&).
    void _Poll() override;  // 0xCB60D0
    // Slot 38: _PollImpl that post-polls at once. The map's
    // _EditPoll(ObjPtr const&).
    void _EditPoll() override;  // 0xCB5F90
    // Slot 39: true. The binary shares the body with other classes.
    bool _HasPostPoll() const override;  // 0xCAF960
    // Slot 40: finds the list if needed and refills it when the last build
    // failed. The map's _PostPoll(ObjPtr const&).
    void _PostPoll() override;  // 0xCB6160

    // Slot 41: the list to fill when neither the object nor an instance
    // holds one; none by default. The map's _GetListCom(ObjPtr const&).
    virtual UIListCom* _GetListCom();  // 0xCB6490
    // Slot 42: fills the rows; false leaves the list as it is and tries
    // again on the next post-poll. Named after the subclasses' overrides
    // (RBCharacterPieceListCom::_BuildListData).
    virtual bool _BuildListData(UIListData& data) = 0;
    // Slot 43: the row to focus after a fill; the first by default.
    virtual unsigned long _GetListFocusIndex(const UIListCom& list, const UIListData& data);  // 0xCB64A0
    // Slot 44: the focused row changed; nothing by default.
    virtual void _OnListFocusChanged(
        const UIListCom& list,
        const UIListData& data,
        const unsigned long& index);  // 0xCB64B0
    // Slot 45: a row was selected; unhandled by default. The binary shares
    // the body with other classes.
    virtual DataNode _OnListSelection(
        const UIListCom& list,
        const UIListData& data,
        const unsigned long& index,
        const UISelectMsg& msg);  // 0xCAF970

    // Passes a selection on to _OnListSelection with the focused row.
    // Inlined into Handle; the binary keeps an uncalled copy.
    DataNode OnMsg(const UISelectMsg& msg);  // 0xCB6270
    // Builds the rows and installs them, focusing the subclass's row (or
    // the first when it is out of range). The map's _UpdateList(ObjPtr
    // const&).
    void _UpdateList();  // 0xCB5E00
    // Finds the list on the instanced entity's root or through _GetListCom
    // and fills it. The map's _InstallSelf(ObjPtr const&).
    void _InstallSelf();  // 0xCB5ED0
    // Takes the rows off the list. Uncalled in this build.
    void _UninstallSelf();  // 0xCB6250
    // Without a list, or after a failed build, post-polls (at once when
    // `immediate` is set, else through the entity's queue); otherwise
    // reports a change of the focused row. The map's _PollImpl(ObjPtr
    // const&, bool).
    void _PollImpl(bool immediate);  // 0xCB6020

    // The rows the subclass builds.
    UIListData mData;
    // Set when the list is a component of the object itself.
    bool mListOnObject;
    // The object's instance component, when the list is on its entity.
    InstanceCom* mInstance;
    // The list, once found.
    UIListCom* mList;
    // Set when the last build failed.
    bool mNeedsUpdate;
    // The focused row, -1 at first.
    unsigned long mFocusIndex;
    // Set after a fill with rows: the poll reports focus changes.
    bool mTrackFocus;
};

static_assert(offsetof(RBUIListProviderBaseCom, mData) == 24);
static_assert(offsetof(RBUIListProviderBaseCom, mListOnObject) == 152);
static_assert(offsetof(RBUIListProviderBaseCom, mInstance) == 160);
static_assert(offsetof(RBUIListProviderBaseCom, mList) == 168);
static_assert(offsetof(RBUIListProviderBaseCom, mNeedsUpdate) == 176);
static_assert(offsetof(RBUIListProviderBaseCom, mFocusIndex) == 184);
static_assert(offsetof(RBUIListProviderBaseCom, mTrackFocus) == 192);
static_assert(sizeof(RBUIListProviderBaseCom) == 200);
