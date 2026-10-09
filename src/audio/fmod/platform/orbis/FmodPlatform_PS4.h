#pragma once

class Transform;

// PS4 parts of the engine's FMOD platform object, the singleton at
// 0x19F29C0 whose vtable is at 0x18EFDF8. The class has not been named or
// reconstructed; these are its recovered members as free functions. Names
// not in the reference map.

// Loads the FMOD modules and pins FMOD's threads. Reconstructed from
// eboot.elf at 0x261F60.
void FmodLoadModules();

// Slot 2 of the platform object at 0x262300: moves Studio listener 0 when
// the platform's FMOD output is enabled.
void FmodSetListenerXfm(bool fmodEnabled, const Transform& xfm);
