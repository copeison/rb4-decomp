#pragma once

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
