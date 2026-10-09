#pragma once

// Character conversions (utl/UTF8.o). Only the conversions the
// reconstructed code uses are declared.

// Widens each character of `str` to 16 bits, terminator included, and
// returns `out`. The size is unused in this build. Null gives null.
unsigned short* CharToWideChar(const char* str, unsigned short* out, unsigned long size);  // 0x1184E00
// Narrows each character of `str` to 8 bits, writing '*' for characters past
// 0xFF, and returns `out`. Text that does not fit `size` ends in
// "... [TRUNCATED]". Null gives null.
char* WideCharToChar(const unsigned short* str, char* out, unsigned long size);  // 0x1184F00
// Case conversions of Latin-1 and Latin Extended-A characters.
unsigned short WToLower(unsigned short ch);  // 0x11853C0
unsigned short WToUpper(unsigned short ch);  // 0x1185640

// Line-breaking rules for CJK text, added after the map's build. Names not
// in the reference map.
// Whether the character is CJK: kana, CJK symbols and punctuation, CJK
// ideographs and extension A, and the U+F900-U+FAFF compatibility block.
bool IsCJKChar(unsigned short ch);  // 0x11848E0
// Whether a line may start with the character: it is not in the set of
// characters at 0x1B5D2B0.
bool CanBeginLine(unsigned short ch);  // 0x1184950
// Whether a line may end with the character: it is not in the set of
// characters at 0x1B5D2E8.
bool CanEndLine(unsigned short ch);  // 0x11849D0
