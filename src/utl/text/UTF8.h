#pragma once

// Character conversions (utl/UTF8.o). Only the conversions the
// reconstructed code uses are declared.

// Widens each character of `str` to 16 bits, terminator included, and
// returns `out`. The size is unused in this build. Null gives null.
unsigned short* CharToWideChar(const char* str, unsigned short* out, unsigned long size);  // 0x1184E00
