#pragma once

#include <cstddef>

// One command-line entry. Name not in the reference map.
struct OptionArg {
    const char* mStr;
    bool mUsed;
};

static_assert(offsetof(OptionArg, mUsed) == 8);
static_assert(sizeof(OptionArg) == 16);

// Vector of parsed command-line entries. Name not in the reference map.
struct OptionArgs {
    OptionArg* mBegin;
    OptionArg* mEnd;
    OptionArg* mCapacity;
};

// At 0x19E7FD8. Name not in the reference map.
extern OptionArgs gOptionArgs;

// The map has OptionStr(char const*, char const*), reading a global list;
// this build takes the list explicitly.
const char* OptionStr(OptionArgs& args, const char* name, const char* def);

// Whether "-<name>" was given, toggling `def`; the entry is marked used.
// The map's OptionBool(char const*, bool); this build takes the list.
bool OptionBool(OptionArgs& args, const char* name, bool def);  // 0x2525A0

// The map has OptionCheck(); this build takes the list explicitly.
void OptionCheck(OptionArgs& args);
