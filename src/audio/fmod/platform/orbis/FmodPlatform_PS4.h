#pragma once

struct EventParameterInfo;
class FmodAudioStreamResource;
class ResourcePath;
template <class T>
class ResourcePtr;
class TextStream;
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
    // Slot 3 at 0x262270: once, releases the engine's FMOD systems
    // (0x19F29D0 through 0x19F29E0) and marks the platform terminated.
    virtual void Terminate();
    // Slot 4 at 0x2632E0: whether Terminate has run.
    virtual bool IsTerminated();
    // Slot 5 at 0x262390: FMOD's current and highest memory use in bytes,
    // from FMOD_Memory_GetStats.
    virtual void GetMemoryStats(unsigned long* current, unsigned long* highest);
    // Slot 6 at 0x262B80: samples each system's FMOD CPU use and collects
    // its timing reports. The evidence for the name is weak.
    virtual void UpdateCpuStats();
    // Slot 7 at 0x262B70: tail-calls 0x274E80, which unloads and reloads
    // every localized FModBankResource, as after a language change.
    virtual void ReloadLocalizedBanks();
    // Slots 8-9 at 0x2632F0 and 0x263300: the Studio event or bus paths of
    // every loaded bank, gathered by 0x2625C0 in mode 0 or 1. The output
    // vector arrives in the third argument register.
    virtual void GetAllEventPaths(void* unused, void* paths);
    virtual void GetAllBusPaths(void* unused, void* paths);
    // Slot 10 at 0x262950: prints "FMOD Memory Usage" from slot 5, then the
    // loaded banks' paths.
    virtual void PrintStats(TextStream& stream);
    // Slot 11 at 0x262B00: FmodAudioStreamResource::GetOrLoad on the path.
    virtual ResourcePtr<FmodAudioStreamResource> GetOrLoadStream(const ResourcePath& path);
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
