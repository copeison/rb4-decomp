#pragma once

// The localized text tables (os/Locale.o). Only the members SystemInit and
// SetSystemLanguage call are declared; the class is not reconstructed.
class Locale {
public:
    // Loads the tables for the system language.
    void Init();  // 0x35D530
    void Terminate();  // 0x35E7A0
};

// At 0x19FDE08.
extern Locale TheLocale;
