#pragma once

#include "utl/data/DataArray.h"

class TextStream;

// The data file reader and writer (utl/DataFile.o), which is not
// reconstructed. Only the functions callers outside it use are declared.

// Parses the text into a new array.
DataArrayPtr DataReadString(const char* str);  // 0x2203A0
// Writes the array's nodes from `start` on, one per line.
void DataWriteStream(TextStream* stream, const DataArray* data, int start);  // 0x220810
