#pragma once

struct EventParameterInfo;
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
// Names not in the reference map.
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
    virtual void _Unknown6();   // slot 6 at 0x262B80
    virtual void _Unknown7();   // slot 7 at 0x262B70
    virtual void _Unknown8();   // slot 8 at 0x2632F0
    virtual void _Unknown9();   // slot 9 at 0x263300
    virtual void _Unknown10();  // slot 10 at 0x262950
    virtual void _Unknown11();  // slot 11 at 0x262B00
    // Slots 12 to 15 look up a Studio event by path and report its
    // parameters; each fails when the event or parameter is unknown.
    // Slot 12 at 0x263030: the parameter's default value.
    virtual bool GetEventParameterDefault(const char* event, const char* parameter, float* value);
    // Slot 13 at 0x2630B0: the number of parameters, or zero.
    virtual int GetEventParameterCount(const char* event);
    // Slot 14 at 0x263120.
    virtual bool GetEventParameterByIndex(const char* event, int index, EventParameterInfo* info);
    // Slot 15 at 0x2631C0.
    virtual bool GetEventParameter(const char* event, const char* parameter, EventParameterInfo* info);
};

// Name not in the reference map.
extern FmodPlatformInterface* gFmodPlatformInterface;  // 0x19C90A0
