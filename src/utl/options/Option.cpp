#include "utl/options/Option.h"

#include <cstdlib>
#include <cstring>

// Reconstructed from eboot.elf at 0x252500.
const char* OptionExecutable(OptionArgs& args) {
    if (args.mBegin == args.mEnd) {
        return "";
    }
    return args.mBegin->mStr;
}

// Reconstructed from eboot.elf at 0x2525A0. A given switch inverts the
// default.
bool OptionBool(OptionArgs& args, const char* name, bool def) {
    for (auto* arg = args.mBegin; arg != args.mEnd; ++arg) {
        if (arg->mStr[0] == '-' && std::strcmp(arg->mStr + 1, name) == 0) {
            arg->mUsed = true;
            return !def;
        }
    }
    return def;
}

// Reconstructed from eboot.elf at 0x252660. The value is the entry after the
// switch; a switch in the last position reads past the end, as in the binary.
const char* OptionStr(OptionArgs& args, const char* name, const char* def) {
    for (auto* arg = args.mBegin; arg != args.mEnd; ++arg) {
        if (arg->mStr[0] == '-' && std::strcmp(arg->mStr + 1, name) == 0) {
            arg->mUsed = true;
            return (arg + 1)->mStr;
        }
    }
    return def;
}

// Reconstructed from eboot.elf at 0x2526C0.
int OptionInt(OptionArgs& args, const char* name, int def) {
    for (auto* arg = args.mBegin; arg != args.mEnd; ++arg) {
        if (arg->mStr[0] == '-' && std::strcmp(arg->mStr + 1, name) == 0) {
            arg->mUsed = true;
            return static_cast<int>(std::strtol((arg + 1)->mStr, nullptr, 0));
        }
    }
    return def;
}

// Reconstructed from eboot.elf at 0x2529E0. Each value is the entry after
// its switch; the search resumes after the value.
eastl::vector<const char*> OptionStrings(OptionArgs& args, const char* name) {
    eastl::vector<const char*> values;
    for (auto* arg = args.mBegin; arg != args.mEnd;) {
        if (arg->mStr[0] == '-' && std::strcmp(arg->mStr + 1, name) == 0) {
            arg->mUsed = true;
            values.push_back((arg + 1)->mStr);
            arg += 2;
        } else {
            ++arg;
        }
    }
    return values;
}

// Reconstructed from eboot.elf at 0x252BC0.
void OptionCheck(OptionArgs& args) {
    for (auto* arg = args.mBegin; arg != args.mEnd; ++arg) {
        if (arg->mStr[0] == '-' && !arg->mUsed) {
            arg->mUsed = true;
        }
    }
}
