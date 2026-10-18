// ui/UIListData.o (from 0x8C3650). Only the constructor and the destructor
// are reconstructed.
#include "ui/lists/UIListData.h"

// Reconstructed from eboot.elf at 0x8C3650. The default row is not
// selectable and is focusable.
UIListData::UIListData() : mRows(), mColumns(), mDefaultRow() {
    mDefaultRow.mSelectable = false;
    mDefaultRow.mFocusable = true;
}

// Reconstructed from eboot.elf at 0x8C4A20. The deleting destructor is at
// 0x8C4AB0.
UIListData::~UIListData() {}
