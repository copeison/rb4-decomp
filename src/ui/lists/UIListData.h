#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

// The rows of named columns a UIListCom shows (ui/UIListData.o, from
// 0x8C3650). It exports messages such as "ui_list_data_rows_cleared_msg"
// when it changes. The vtable at 0x195DFD0 has MsgSource's seven slots with
// the destructors overridden. Only the layout and the constructor and
// destructor are reconstructed. The object is 128 bytes. Field names are not
// in the reference map.
class UIListData : public MsgSource {
public:
    // One row: an entry per column and two flags. The map names the type
    // (UIListData::DataRow); the field names are not in the reference map,
    // and which flag is which is weak (the map has SetRowSelectable and
    // SetRowFocusable).
    struct DataRow {
        eastl::vector<DataNode> mEntries;
        bool mSelectable;
        bool mFocusable;
    };

    UIListData();  // 0x8C3650
    // Slots 0-1: 0x8C4A20, 0x8C4AB0.
    ~UIListData() override;

    // The number of rows. Not reconstructed.
    unsigned long GetNumRows() const;  // 0x8C3BE0

    eastl::vector<DataRow> mRows;
    // The column names, in column order.
    eastl::vector<Symbol> mColumns;
    // The row new rows copy: the columns' default entries and the flags
    // (0x8C3A70).
    DataRow mDefaultRow;
};

static_assert(sizeof(UIListData::DataRow) == 40);
static_assert(offsetof(UIListData, mRows) == 24);
static_assert(offsetof(UIListData, mColumns) == 56);
static_assert(offsetof(UIListData, mDefaultRow) == 88);
static_assert(sizeof(UIListData) == 128);
