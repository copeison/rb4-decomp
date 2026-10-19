#pragma once

#include "utl/text/Symbol.h"

// The localized text tables (os/Locale.o). Only the members SystemInit and
// SetSystemLanguage call are declared; the class is not reconstructed.
class Locale {
public:
    // Loads the tables for the system language.
    void Init();  // 0x35D530
    void Terminate();  // 0x35E7A0

    // Reports each localized-string notification; UIMgr reads the "ui"
    // config's verbose_locale_notifies into it.
    static bool sVerboseNotify;  // 0x19FDDFC
};

// At 0x19FDE08.
extern Locale TheLocale;

// The localized text of the token, or the token's own text when the table
// has no entry; `found`, when given, receives whether it had one. The map
// has Localize(Symbol, bool*, Locale&); this build passes a third argument
// that the body does not read, which its callers set to zero.
const char* Localize(Symbol token, bool* found, int unused, Locale& locale);  // 0x35E9B0
