#pragma once

#include "utl/data/DataNode.h"

// A script function: takes the calling array and returns its result. Name
// not in the reference map.
using DataFunc = DataNode (*)(DataArray*);

// Makes a script function callable by name.
void DataRegisterFunc(Symbol name, DataFunc func);  // 0x2221F0
