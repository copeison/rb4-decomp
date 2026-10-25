// rb_meta/RBUIListProviderBaseCom.o (0xCB5B10 to 0xCB64BF). The class is
// abstract and has no statics, so the object has no static initializer.
#include "rb_meta/ui/RBUIListProviderBaseCom.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/InstanceCom.h"
#include "ui/input/UIMsg.h"
#include "ui/lists/UIListCom.h"
#include "utl/data/DataArray.h"

namespace {

// The list on the root of the entity the instance component holds, or null.
// Inlined into _Enter, _InstallSelf and _PostPoll. Name not in the
// reference map.
UIListCom* FindInstancedList(const InstanceCom* instance) {
    if (instance == nullptr || instance->mRuntime.mEntity == nullptr) {
        return nullptr;
    }
    return instance->mRuntime.mEntity->GetRoot()->GetCom<UIListCom>();
}

}  // namespace

// Reconstructed from eboot.elf at 0xCB5B10.
RBUIListProviderBaseCom::RBUIListProviderBaseCom()
    : mData(),
      mListOnObject(false),
      mInstance(nullptr),
      mList(nullptr),
      mNeedsUpdate(false),
      mFocusIndex(static_cast<unsigned long>(-1)),
      mTrackFocus(true) {}

// Reconstructed from eboot.elf at 0xCB5B70. The deleting destructor is at
// 0xCB5C20.
RBUIListProviderBaseCom::~RBUIListProviderBaseCom() {}

// Reconstructed from eboot.elf at 0xCB5C40.
bool RBUIListProviderBaseCom::_OnResourcesLoaded() {
    mList = mObject->GetCom<UIListCom>();
    if (mList != nullptr) {
        mInstance = nullptr;
        mListOnObject = true;
        return true;
    }
    mInstance = mObject->GetBaseCom<InstanceCom>();
    if (mInstance == nullptr) {
        static_cast<void>(MakeErrorName());
    }
    return true;
}

// Reconstructed from eboot.elf at 0xCB5D20.
void RBUIListProviderBaseCom::_Enter() {
    if (mListOnObject) {
        _UpdateList();
        return;
    }
    _InstallSelf();
}

// Reconstructed from eboot.elf at 0xCB5E00.
void RBUIListProviderBaseCom::_UpdateList() {
    mTrackFocus = false;
    if (!_BuildListData(mData)) {
        mNeedsUpdate = true;
        return;
    }
    mNeedsUpdate = false;
    mList->SetData(&mData);
    unsigned long index = _GetListFocusIndex(*mList, mData);
    if (index < mData.GetNumRows()) {
        mList->SetFocusByDataIndex(index);
        // The binary calls UIListCom::GetDataFocusIndex(bool) const
        // (0x8B7E40), which the model keeps inline.
        index = static_cast<unsigned long>(static_cast<long>(mList->GetDataFocusIndex(true)));
    } else {
        index = 0;
        mList->SetFocusByDataIndex(0);
    }
    mFocusIndex = index;
    if (mData.GetNumRows() != 0) {
        mTrackFocus = true;
    }
}

// Reconstructed from eboot.elf at 0xCB5ED0.
void RBUIListProviderBaseCom::_InstallSelf() {
    if (mList != nullptr) {
        return;
    }
    mList = FindInstancedList(mInstance);
    if (mList == nullptr) {
        mList = _GetListCom();
    }
    if (mList != nullptr) {
        _UpdateList();
    }
}

// Reconstructed from eboot.elf at 0xCB5F90.
void RBUIListProviderBaseCom::_EditPoll() {
    _PollImpl(true);
}

// Reconstructed from eboot.elf at 0xCB6020.
void RBUIListProviderBaseCom::_PollImpl(bool immediate) {
    if (mList == nullptr || mNeedsUpdate) {
        if (immediate) {
            _PostPoll();
        } else {
            mObject->mEntity->QueueForPostPoll(this);
        }
        return;
    }
    if (!mTrackFocus) {
        return;
    }
    const unsigned long index = static_cast<unsigned long>(static_cast<long>(mList->GetDataFocusIndex(true)));
    if (index != static_cast<unsigned long>(-1) && index != mFocusIndex) {
        mFocusIndex = index;
        _OnListFocusChanged(*mList, mData, mFocusIndex);
    }
}

// Reconstructed from eboot.elf at 0xCB60D0.
void RBUIListProviderBaseCom::_Poll() {
    _PollImpl(false);
}

// Reconstructed from eboot.elf at 0xCB6160.
void RBUIListProviderBaseCom::_PostPoll() {
    _InstallSelf();
    if (mNeedsUpdate && mList != nullptr) {
        _UpdateList();
    }
}

// Reconstructed from eboot.elf at 0xCB6240.
void RBUIListProviderBaseCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
}

// Reconstructed from eboot.elf at 0xCB6250.
void RBUIListProviderBaseCom::_UninstallSelf() {
    if (mList != nullptr) {
        mList->SetData(nullptr);
    }
}

// Reconstructed from eboot.elf at 0xCB6270.
DataNode RBUIListProviderBaseCom::OnMsg(const UISelectMsg& msg) {
    if (mList == nullptr) {
        return DataNode();
    }
    const unsigned long index = static_cast<unsigned long>(static_cast<long>(mList->GetDataFocusIndex(true)));
    return _OnListSelection(*mList, mData, index, msg);
}

// Reconstructed from eboot.elf at 0xCB6310.
DataNode RBUIListProviderBaseCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    if (msg->Sym(1) == UISelectMsg::Event()) {
        DataNode result = OnMsg(UISelectMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xCB6490.
UIListCom* RBUIListProviderBaseCom::_GetListCom() {
    return nullptr;
}

// Reconstructed from eboot.elf at 0xCB64A0.
unsigned long RBUIListProviderBaseCom::_GetListFocusIndex(const UIListCom& list, const UIListData& data) {
    static_cast<void>(list);
    static_cast<void>(data);
    return 0;
}

// Reconstructed from eboot.elf at 0xCB64B0.
void RBUIListProviderBaseCom::_OnListFocusChanged(
    const UIListCom& list,
    const UIListData& data,
    const unsigned long& index) {
    static_cast<void>(list);
    static_cast<void>(data);
    static_cast<void>(index);
}

// Reconstructed from eboot.elf at 0xCAF960. The binary folds identical
// bodies of other classes into this one.
bool RBUIListProviderBaseCom::_HasPostPoll() const {
    return true;
}

// Reconstructed from eboot.elf at 0xCAF970. Folded like _HasPostPoll.
DataNode RBUIListProviderBaseCom::_OnListSelection(
    const UIListCom& list,
    const UIListData& data,
    const unsigned long& index,
    const UISelectMsg& msg) {
    static_cast<void>(list);
    static_cast<void>(data);
    static_cast<void>(index);
    static_cast<void>(msg);
    return DataNode();
}
