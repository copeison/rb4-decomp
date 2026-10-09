#pragma once

#include "utl/containers/Map.h"
#include "utl/data/DataNode.h"

// A script function: takes the calling array and returns its result. Name
// not in the reference map.
using DataFunc = DataNode (*)(DataArray*);

// The registered script functions by name, at 0x19E77A0. DataExecute
// (0x21EAB0) looks commands up here.
extern eastl::map<Symbol, DataFunc> gDataFuncs;

// Makes a script function callable by name.
void DataRegisterFunc(Symbol name, DataFunc func);  // 0x2221F0
