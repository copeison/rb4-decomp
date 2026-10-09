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

// The platform object's interface, reached through the pointer at
// 0x19C90A0 that the sound manager's initialization sets to the object.
// Slots 6 to 9 are not declared. Names not in the reference map.
class FmodPlatformInterface {
public:
    virtual ~FmodPlatformInterface();  // slots 0-1: 0x2625A0, 0x2625B0
    // Slot 2 at 0x262300; FmodSetListenerXfm above.
    virtual void SetListenerXfm(bool fmodEnabled, const Transform& xfm);
    virtual void _Unknown3();  // slot 3 at 0x262270
    virtual void _Unknown4();  // slot 4 at 0x2632E0
    // Slot 5 at 0x262390: FMOD's current and highest memory use in bytes,
    // from FMOD_Memory_GetStats.
    virtual void GetMemoryStats(unsigned long* current, unsigned long* highest);
};

// Name not in the reference map.
extern FmodPlatformInterface* gFmodPlatformInterface;  // 0x19C90A0
