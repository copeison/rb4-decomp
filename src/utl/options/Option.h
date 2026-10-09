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

// Address not yet recovered. Name not in the reference map.
extern OptionArgs gOptionArgs;

// The map has OptionStr(char const*, char const*), reading a global list;
// this build takes the list explicitly.
const char* OptionStr(OptionArgs& args, const char* name, const char* def);

// The map has OptionCheck(); this build takes the list explicitly.
void OptionCheck(OptionArgs& args);
