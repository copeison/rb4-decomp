#pragma once

// Engine system startup, configuration and per-frame service (os/System.o)
// and its PS4 half (os/System_PS4.o).

#include "utl/text/Symbol.h"

class DataArray;
class FixedString;

// Loads the main configuration and installs the system config handlers.
void SystemInit(const char* config_path);  // 0x368000
// Stops the services SystemInit started. SystemInit registers it as an exit
// callback.
void SystemTerminate();  // 0x367ED0
// Makes SystemInit parse its argument as the configuration's text rather
// than read it from a file.
void ReadConfigAsDataArrayString();  // 0x367FF0
// The system configuration, or null before SystemInit.
DataArray* SystemConfig();  // 0x368AF0
// The array at the path in the system configuration.
DataArray* SystemConfig(Symbol key);                          // 0x368B00
DataArray* SystemConfig(Symbol key1, Symbol key2);            // 0x368CD0
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3);  // 0x369A90
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3, Symbol key4);  // 0x369AD0
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3, Symbol key4,
                        Symbol key5);  // 0x369B30
// Replaces the system configuration.
void OverrideSystemConfig(DataArray* config);  // 0x369A30
// Milliseconds since the system timer started, advancing the timer.
int SystemMs();  // 0x369860
// Updates the core runtime, input assignments, platform services, timers and
// file state once per frame.
void SystemPoll();  // 0x369940
// The first and second halves of SystemPoll. Names not in the reference map.
void SystemPollServices();  // 0x3699B0
void SystemPollTimers();    // 0x3699F0
// The language the system runs in.
Symbol SystemLanguage();  // 0x369BA0
// The system configuration's titles array.
DataArray* SystemTitles();  // 0x369BB0
// The system language block's default language, or "".
Symbol GetDefaultLanguage();  // 0x369BC0
// The sync_changelist value of the system configuration, or -1.
int SystemSyncedAutobuildChangelist();  // 0x369D70
// The supported languages, or with `cheat` the languages cheats may select.
DataArray* SupportedLanguages(bool cheat);  // 0x369E40
// "English" to "eng" and back; "" when unknown. The map spells the first
// LanguageToAbbrevation.
Symbol LanguageToAbbrevation(Symbol language);    // 0x36A070
Symbol AbbreviationToLanguage(Symbol language);   // 0x36A280
bool IsSupportedLanguage(Symbol language, bool cheat);  // 0x36A490
// Switches the system language, falling back to the configured default when
// it is not supported, and reloads the locale on a change.
void SetSystemLanguage(Symbol language, bool cheat);  // 0x36A500

// The host_config, host_logging, host_file and host_cached options, read by
// SystemInit. At 0x19FE628, 0x19FE629, 0x19FE630 and 0x19FE638.
extern bool gHostConfig;
extern bool gHostLogging;
extern const char* gHostFile;
extern bool gHostCached;

// The changelist the executable was built from, 1299228. At 0x1245E64. Name
// not in the reference map.
extern const int gAutobuildChangelist;

// Appends the symbolized call stack in `stack`, a null-terminated array of
// return addresses, to `out`. In this build it sits between the Debug.o
// functions.
void AppendStackTrace(FixedString& out, void* stack);  // 0x35C230

// os/System_PS4.o.
// Empty in this build.
void SystemPlatformTerminate();  // 0x3767C0
// The system software's language, or `def` when the game has no
// translation for it.
Symbol GetSystemLanguage(Symbol def);  // 0x3767D0
// Writes up to 50 return addresses of the caller's callers into `trace`,
// following the frame pointers. The map has CaptureStackTrace(int,
// StackData*, void*); this build ignores `context`.
void CaptureStackTrace(void** trace, void* context);  // 0x376A30
// Whether a debugger is attached; false in this build. Name not in the
// reference map.
bool PlatformDebuggerAttached();  // 0x376A70
// Empty in this build. The map has PlatformDebugBreak(bool).
void PlatformDebugBreak();  // 0x376A80
// Shows the message in a system dialog with an OK button and waits for it
// to close. Only the message is used. Name not in the reference map.
void PlatformModalDialog(Symbol title, const char* msg, const char* button, bool fail,
                         bool wait);  // 0x376A90
// The symbol map's path: the configured ps4_map_file, or the executable's
// path with a .map extension. The map has GetMapFileName(String&).
void GetMapFileName(FixedString& out);  // 0x376B80
