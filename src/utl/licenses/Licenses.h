#pragma once

#include <cstddef>

// A third-party license notice, registered by a static instance in each
// library that needs one (utl/Licenses.o).
class Licenses {
public:
    // Whether the notice must be shown. Name not in the reference map,
    // which names the type.
    enum Requirement : unsigned char {
        kRequirementNone = 0,
    };

    // Links the notice into the list. The map has Licenses(char const*,
    // Licenses::Requirement); this build also takes the notice's text.
    Licenses(const char* name, const char* text, Requirement requirement);  // 0x247110
    // Walks the notices; the printing is compiled out in this build.
    static void PrintAll();  // 0x247160

    // Field names are not in the reference map.
    Requirement mRequirement;
    const char* mName;
    const char* mText;
    Licenses* mNext;
};

static_assert(offsetof(Licenses, mNext) == 0x18);
