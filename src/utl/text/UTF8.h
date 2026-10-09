#pragma once

#include "utl/containers/Vector.h"
#include "utl/text/Str.h"

// UTF-8 and 16-bit character conversions (utl/UTF8.o). Wide characters are
// 16 bits, as wchar_t is on this platform; decoding handles one- to
// three-byte sequences and turns any other lead byte into '*'.

// Line-breaking rules for CJK text, added after the map's build. Names not
// in the reference map.
// Whether the character is CJK: kana, CJK symbols and punctuation, CJK
// ideographs and extension A, and the U+F900-U+FAFF compatibility block.
bool IsCJKChar(unsigned short ch);  // 0x11848E0
// Whether a line may start with the character: it is not in the set at
// 0x1B5D2A8.
bool CanBeginLine(unsigned short ch);  // 0x1184950
// Whether a line may end with the character: it is not in the set at
// 0x1B5D2E0.
bool CanEndLine(unsigned short ch);  // 0x11849D0

// Decodes the character at `str` and returns its length in bytes.
int DecodeUTF8(unsigned short& ch, const char* str);  // 0x1184A50
// Encodes the character with a terminator and returns its length in bytes;
// characters past 0xFFFF become '*'.
int EncodeUTF8(char* out, unsigned int ch);  // 0x1184AC0
int EncodeUTF8(String& out, unsigned int ch);  // 0x1184B40
// Decodes `in`, writing characters past 0xFF as `replacement`.
void UTF8toASCIIs(char* out, unsigned long size, const char* in, char replacement);  // 0x1184C50
// Encodes each Latin-1 character of `in`, zero-filling `out`.
void ASCIItoUTF8(char* out, unsigned long size, const char* in);  // 0x1184D40
// Widens each character of `str` to 16 bits, terminator included, and
// returns `out`. The size is unused in this build. Null gives null.
unsigned short* CharToWideChar(const char* str, unsigned short* out, unsigned long size);  // 0x1184E00
// Narrows each character of `str` to 8 bits, writing '*' for characters past
// 0xFF, and returns `out`. Text that does not fit `size` ends in
// "... [TRUNCATED]". Null gives null.
char* WideCharToChar(const unsigned short* str, char* out, unsigned long size);  // 0x1184F00

// Copies at most size - 1 characters and returns `out`; null gives empty
// text. Narrowing writes '*' for characters past 0xFF; widening
// sign-extends.
char* ConvertStr(const unsigned short* in, char* out, unsigned long size);  // 0x1184F90
char* ConvertStr(const wchar_t* in, char* out, unsigned long size);  // 0x1185000
char* ConvertStr(const char* in, char* out, unsigned long size);  // 0x1185070
unsigned short* ConvertStr(const char* in, unsigned short* out, unsigned long size);  // 0x11850A0
wchar_t* ConvertStr(const char* in, wchar_t* out, unsigned long size);  // 0x1185100

// The length in characters.
int UTF8StrLen(const char* str);  // 0x1185160
int WideStrLen(const unsigned short* str);  // 0x11851C0
// The first occurrence of the character, or null.
const char* UTF8strchr(const char* str, unsigned short ch);  // 0x11851E0
// Copies `in` into `size` bytes at `out`, replacing every occurrence of
// `find` with `replacement`; characters are never split. Name not in the
// reference map.
void UTF8SearchReplace(
    char* out,
    unsigned long size,
    const char* in,
    const char* find,
    const char* replacement);  // 0x1185270

// Case conversions of Latin-1 and Latin Extended-A characters.
unsigned short WToLower(unsigned short ch);  // 0x11853C0
// Writes the encoded lowercase character without a terminator.
void UTF8ToLower(unsigned short ch, char* out);  // 0x1185460
unsigned short WToUpper(unsigned short ch);  // 0x1185640
void UTF8ToUpper(unsigned short ch, char* out);  // 0x11856E0
// Case-insensitive comparisons of at most `count` characters.
int UTF8StrniCmp(const char* a, const unsigned short* b, int count);  // 0x11858C0
int WStrnCmp(const unsigned short* a, const unsigned short* b, int count);  // 0x1185B20
int WStrniCmp(const unsigned short* a, const unsigned short* b, int count);  // 0x1185B50

// Copies `in`, replacing characters missing from `allowed`.
void UTF8FilterString(
    char* out,
    unsigned long size,
    const char* in,
    const char* allowed,
    char replacement);  // 0x1185D50
// Copies `in` without leading, trailing or repeated spaces.
void UTF8RemoveSpaces(char* out, unsigned long size, const char* in);  // 0x1185F00
// Copies at most `count` characters that fit `size` bytes, zero-filling
// `out`, and returns the bytes copied.
unsigned long UTF8Resize(char* out, unsigned long count, unsigned long size, const char* in);  // 0x11860C0
// Decodes at most size - 1 characters, stores the count in `length` when
// given, and returns `out`.
unsigned short* UTF8toWide(
    const char* in,
    unsigned short* out,
    unsigned long size,
    unsigned long* length);  // 0x11861C0
wchar_t* UTF8toWChar_t(wchar_t* out, const char* in);  // 0x11862C0
wchar_t* UTF8toWChar_t(wchar_t* out, unsigned long size, const char* in);  // 0x1186370
// Prints each encoded character of `in` to `out`.
void WChar_tToUTF8(String& out, const wchar_t* in);  // 0x1186450

void UTF8toWideVector(eastl::vector<unsigned short>& out, const char* in);  // 0x11865B0
void ASCIItoWideVector(eastl::vector<unsigned short>& out, const char* in);  // 0x1186730
// Narrows each character, writing '*' for characters past 0xFF.
String WideVectorToASCII(eastl::vector<unsigned short>& in);  // 0x1186960
// Replaces `out` with the encoded text and returns its length in bytes.
int WideVectorToUTF8(eastl::vector<unsigned short>& in, String& out);  // 0x11869D0
