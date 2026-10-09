#include "os/system/System.h"

#include <cstring>

#include "entity/resources/Resource.h"
#include "math/easing/Easing.h"
#include "net/NetInit.h"
#include "os/content/ContentMgr.h"
#include "os/debug/Cheats.h"
#include "os/debug/Debug.h"
#include "os/debug/GlitchBreaker.h"
#include "os/files/Archive.h"
#include "os/files/File.h"
#include "os/joypads/Joypad.h"
#include "os/joypads/Keyboard.h"
#include "os/locale/Locale.h"
#include "os/memory/MemMgr.h"
#include "os/movie/Movie.h"
#include "os/platform/PlatformMgr.h"
#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"
#include "os/stagekit/StageKit.h"
#include "os/system/AppChild.h"
#include "os/system/Core.h"
#include "os/system/MapFile.h"
#include "os/threading/CallOnMainThread.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFile.h"
#include "utl/data/DataFunc.h"
#include "utl/data/DataUtl.h"
#include "utl/licenses/Licenses.h"
#include "utl/options/Option.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/threading/PollMgr.h"
#include "utl/threading/Thread.h"
#include "utl/time/Timer.h"

extern const int gAutobuildChangelist = 1299228;

bool gHostConfig;
bool gHostLogging;
const char* gHostFile;
bool gHostCached;

namespace {

// The system configuration, at 0x19FE548. Name not in the reference map.
DataArrayPtr gSystemConfig;
// The language the system runs in, at 0x19FE540. Name not in the reference
// map.
Symbol gSystemLanguage;
// The timer SystemMs reads, at 0x19FE550. Name not in the reference map.
PerfTimer gSystemTimer(Symbol("system"), nullptr);
// The milliseconds SystemMs has counted and the fraction it carries, at
// 0x19FE664 and 0x19FE660. Names not in the reference map.
int gSystemMs;
float gSystemMsFraction;
// Set by ReadConfigAsDataArrayString, at 0x19FE650. Name not in the
// reference map.
bool gReadConfigAsString;
// The configuration's titles array, at 0x19FE658. Name not in the
// reference map.
DataArray* gSystemTitles;
// The glitch_break_ms watchdog and its thread, at 0x19FE648 and 0x19FE640.
// Names not in the reference map.
GlitchBreaker* gGlitchBreaker;
NamedThread* gGlitchThread;

// The configuration array under the key. Inlined into its users.
DataArray* FindConfig(DataArray* config, Symbol key) {
    return config->FindArray(key, false);
}

// The script functions SystemInit registers. Names not in the reference
// map.
DataNode OnSystemLanguage(DataArray*) {  // 0x36A830
    return DataNode(gSystemLanguage);
}

// Runs nothing in this build; the command is read and -1 returned.
DataNode OnSystemExec(DataArray* msg) {  // 0x36A850
    String output;
    if (msg->Size() >= 3) {
        static_cast<void>(msg->Int(2));
    }
    static_cast<void>(msg->Str(1));
    return DataNode(-1);
}

DataNode OnUsingArks(DataArray*) {  // 0x36A900
    return DataNode(1);
}

DataNode OnSupportedLanguages(DataArray*) {  // 0x36A920
    return DataNode(SupportedLanguages(false), kDataArray);
}

DataNode OnSystemMs(DataArray*) {  // 0x36A950
    return DataNode(SystemMs());
}

// The language block's array under the key. Inlined into the language
// functions.
DataArray* LanguageConfig(Symbol key) {
    static Symbol sSystem;
    if (sSystem == Symbol()) {
        sSystem = Symbol("system");
    }
    static Symbol sLanguage;
    if (sLanguage == Symbol()) {
        sLanguage = Symbol("language");
    }
    return FindConfig(FindConfig(FindConfig(gSystemConfig, sSystem), sLanguage), key);
}

}  // namespace

// Reconstructed from eboot.elf at 0x35C230. The binary places it among the
// Debug.o functions.
void AppendStackTrace(FixedString& out, void* stack) {
    StackString<512> mapFile;
    GetMapFileName(mapFile);
    auto** trace = static_cast<void**>(stack);
    int count = 0;
    while (count <= 49 && trace[count] != nullptr) {
        ++count;
    }
    FormatString format("   Stack Trace: (%d)");
    format << count;
    out += format.Str();
    if (!MapFile::Parse(mapFile.c_str(), stack, MapFile::GetReferenceAddress(), out, kPlatformPS4)) {
        MapFile::Parse(mapFile.c_str(), stack, MapFile::GetReferenceAddress(), out, kPlatformNone);
    }
    out += "\r\n";
}

// Reconstructed from eboot.elf at 0x367ED0.
void SystemTerminate() {
    if (gGlitchThread != nullptr) {
        gGlitchBreaker->Terminate();
        gGlitchThread->mThread._Join();
        delete gGlitchBreaker;
        gGlitchBreaker = nullptr;
        if (gGlitchThread != nullptr) {
            gGlitchThread->mThread._ForceKillThread();
            delete gGlitchThread;
        }
        gGlitchThread = nullptr;
    }
    TheDebug.RemoveExitCallback(SystemTerminate);
    MovieMgr::Terminate();
    TheLocale.Terminate();
    CheatsTerminate();
    KeyboardTerminate();
    JoypadTerminate();
    TheContentMgr.Terminate();
    FileTerminate();
    File::Destroy();
    gSystemConfig = nullptr;
    MemTerminate();
    AppChild::Terminate();
    core_terminate();
}

// Reconstructed from eboot.elf at 0x367FF0.
void ReadConfigAsDataArrayString() {
    gReadConfigAsString = true;
}

// Reconstructed from eboot.elf at 0x368000.
void SystemInit(const char* config_path) {
    SceMapFile::Init();
    PlatformMgrPreInit();
    FilePlatformPreInit();

    // A shipped header marks the archive build.
    FormatString header("main_%s.hdr");
    header << PlatformSymbol(kPlatformPS4);
    FileInfo info = {};
    if (FileGetStat(header.Str(), &info) >= 0) {
        gFileArchiveMode = 1;
    }
    gHostConfig = OptionBool(gOptionArgs, "host_config", false);
    gHostLogging = OptionBool(gOptionArgs, "host_logging", false);
    gHostFile = OptionStr(gOptionArgs, "host_file", nullptr);
    if (gHostFile != nullptr) {
        gHostConfig = true;
    }
    gHostCached = OptionBool(gOptionArgs, "host_cached", false);
    FileInit();
    AppChild::Init();
    ArchiveInit();
    TheDebug.Init();

    // A host configuration is read from the disk, not the archive.
    const unsigned char archiveMode = gFileArchiveMode;
    Archive* const archive = TheArchive;
    if (gHostConfig) {
        gFileArchiveMode = 0;
        TheArchive = nullptr;
    }

    DataArrayPtr defines(MakeDataArray(DataNode(1)));
    DataSetMacro(Symbol("HX_PS4"), defines);
    DataSetMacro(Symbol("_SHIP"), defines);
    {
        eastl::vector<const char*> names = OptionStrings(gOptionArgs, "define");
        for (const char* name : names) {
            DataSetMacro(Symbol(name), defines);
        }
    }

    const char* option = OptionStr(gOptionArgs, "config", nullptr);
    if (option != nullptr) {
        config_path = option;
        gReadConfigAsString = false;
    }
    if (option == nullptr && gReadConfigAsString) {
        DataArrayPtr config(DataReadString(config_path));
        gSystemConfig = config.mData;
    } else {
        DataArrayPtr config(DataReadFile(config_path, true));
        gSystemConfig = config.mData;
    }
    DataSetGlobal(Symbol("syscfg"), DataNode(gSystemConfig));
    gFileArchiveMode = archiveMode;
    TheArchive = archive;

    DataRegisterFunc(Symbol("system_language"), OnSystemLanguage);
    DataRegisterFunc(Symbol("system_exec"), OnSystemExec);
    DataRegisterFunc(Symbol("using_arks"), OnUsingArks);
    DataRegisterFunc(Symbol("supported_languages"), OnSupportedLanguages);
    DataRegisterFunc(Symbol("system_ms"), OnSystemMs);
    defines = nullptr;

    Resource::InitNonCachedFolders(gSystemConfig);

    // The system software's language, remapped and forced by the
    // configuration and overridden by the lang option.
    DataArray* languages = FindConfig(FindConfig(gSystemConfig, Symbol("system")), Symbol("language"));
    const Symbol systemLanguage = GetSystemLanguage(Symbol("eng"));
    Symbol language = systemLanguage;
    if (DataArray* remap = FindConfig(languages, Symbol("remap"))) {
        remap->FindData(systemLanguage, language, false);
    }
    Symbol forced;
    if (languages->FindData(Symbol("force"), forced, false) && forced.Str()[0] != '\0') {
        language = forced;
    }
    if (const char* lang = OptionStr(gOptionArgs, "lang", nullptr)) {
        language = Symbol(lang);
    }
    SetSystemLanguage(language, false);

    MemInit(FindConfig(gSystemConfig, Symbol("mem")));
    JoypadInit();
    KeyboardInit();
    PerfInit(FindConfig(gSystemConfig, Symbol("timer")));
    PollMgr::Init();
    DataInit();
    gSystemTimer.Start();
    gSystemTitles = FindConfig(FindConfig(gSystemConfig, Symbol("system")), Symbol("titles"));
    DataArray* maxFiles =
        FindConfig(FindConfig(gSystemConfig, Symbol("system")), Symbol("max_file_instances"));
    FileSetMaxFileInstances(maxFiles->Int(1));
    TheLocale.Terminate();
    TheLocale.Init();
    CheatsInit();
    NetInit();
    PlatformMgr::InitJoypadExtraLags();
    ThePlatformMgr->Init();
    TheContentMgr.Init();
    MovieMgr::Init();
    StageKitInit();
    EditorJoypadDataCom::Init();
    KeyboardTransControllerCom::Init();
    PlatformMgrCom::Init();
    InitEasingParameters();
    TheDebug.AddExitCallback(SystemTerminate);

    if (OptionBool(gOptionArgs, "licenses", false)) {
        Licenses::PrintAll();
        TheDebug.Exit(0, true);
    }
    DataExecuteBlock(FindConfig(gSystemConfig, Symbol("init")), 1);
    if (DataArray* version = FindConfig(gSystemConfig, Symbol("version"))) {
        static_cast<void>(version->Str(1));
    }

    const int glitchMs = OptionInt(gOptionArgs, "glitch_break_ms", 0);
    if (glitchMs > 0) {
        gGlitchThread = new NamedThread;
        gGlitchThread->Init("Unknown Thread!");
        gGlitchBreaker = new GlitchBreaker(glitchMs, true);
        NamedThread* const thread = gGlitchThread;
        FormatString name("GlitchBreaker (%d ms)");
        name << glitchMs;
        thread->Create(GlitchBreaker::ThreadMain, gGlitchBreaker, name.Str(), 0,
                       Thread::kPriorityDefault, 0, 0);
        gGlitchThread->mThread.Start();
    }
}

// Reconstructed from eboot.elf at 0x368AF0.
DataArray* SystemConfig() {
    return gSystemConfig;
}

// Reconstructed from eboot.elf at 0x368B00.
DataArray* SystemConfig(Symbol key) {
    return FindConfig(gSystemConfig, key);
}

// Reconstructed from eboot.elf at 0x368CD0.
DataArray* SystemConfig(Symbol key1, Symbol key2) {
    return FindConfig(FindConfig(gSystemConfig, key1), key2);
}

// Reconstructed from eboot.elf at 0x369860.
int SystemMs() {
    // Restart the timer's frame, recording the time since the last call.
    PerfTimer::Frame& frame = gSystemTimer.mFrames[PerfTimer::gCurrentFrameIndex];
    const unsigned long now = __builtin_ia32_rdtsc();
    const unsigned long cycles = frame.mDepth != 0 ? now - frame.mStartCycles : frame.mCycles;
    frame.mStartCycles = now;
    frame.mCycles = 0;
    frame.mDepth = 1;
    gSystemTimer.UpdateMs(static_cast<float>(Hmx::Timer::CyclesToMs(cycles)));
    const int frameNumber = frame.mFrameNumber++;
    if (frameNumber >= frame.mWorstFrame + PerfTimer::kWorstResetFrames) {
        frame.mWorstMs = 0.0F;
        frame.mWorstFrame = 0;
    }
    // The binary reads the first frame's time whatever the current frame.
    const float total = gSystemMsFraction + gSystemTimer.mFrames[0].mMs;
    const int whole = static_cast<int>(total);
    gSystemMs += whole;
    gSystemMsFraction = total - static_cast<float>(whole);
    return gSystemMs;
}

// Reconstructed from eboot.elf at 0x369940.
void SystemPoll() {
    SystemMs();
    TheDebug.Poll();
    core_poll();
    ThePlatformMgr->Poll();
    input_refresh_player_assignments();
    JoypadClientPoll();
    Hmx::CallOnMainThreadPoll();
    core_update_time();
    NetPoll();
    TheContentMgr.Poll();
    TheMovieMgr->Poll();
}

// Reconstructed from eboot.elf at 0x3699B0.
void SystemPollServices() {
    SystemMs();
    TheDebug.Poll();
    core_poll();
    ThePlatformMgr->Poll();
    input_refresh_player_assignments();
    JoypadClientPoll();
    Hmx::CallOnMainThreadPoll();
}

// Reconstructed from eboot.elf at 0x3699F0.
void SystemPollTimers() {
    core_update_time();
    NetPoll();
    TheContentMgr.Poll();
    TheMovieMgr->Poll();
}

// Reconstructed from eboot.elf at 0x369A30.
void OverrideSystemConfig(DataArray* config) {
    gSystemConfig = config;
}

// Reconstructed from eboot.elf at 0x369A90.
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3) {
    return FindConfig(FindConfig(FindConfig(gSystemConfig, key1), key2), key3);
}

// Reconstructed from eboot.elf at 0x369AD0.
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3, Symbol key4) {
    return FindConfig(FindConfig(FindConfig(FindConfig(gSystemConfig, key1), key2), key3), key4);
}

// Reconstructed from eboot.elf at 0x369B30.
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3, Symbol key4, Symbol key5) {
    return FindConfig(
        FindConfig(FindConfig(FindConfig(FindConfig(gSystemConfig, key1), key2), key3), key4), key5);
}

// Reconstructed from eboot.elf at 0x369BA0.
Symbol SystemLanguage() {
    return gSystemLanguage;
}

// Reconstructed from eboot.elf at 0x369BB0.
DataArray* SystemTitles() {
    return gSystemTitles;
}

// Reconstructed from eboot.elf at 0x369BC0.
Symbol GetDefaultLanguage() {
    static Symbol sDefault;
    if (sDefault == Symbol()) {
        sDefault = Symbol("default");
    }
    DataArray* language = LanguageConfig(sDefault);
    if (language == nullptr) {
        return Symbol("");
    }
    return language->Sym(1);
}

// Reconstructed from eboot.elf at 0x369D70.
int SystemSyncedAutobuildChangelist() {
    int changelist = -1;
    if (gSystemConfig != nullptr) {
        static Symbol sSyncChangelist;
        if (sSyncChangelist == Symbol()) {
            sSyncChangelist = Symbol("sync_changelist");
        }
        gSystemConfig->FindData(sSyncChangelist, changelist, false);
    }
    return changelist;
}

// Reconstructed from eboot.elf at 0x369E40.
DataArray* SupportedLanguages(bool cheat) {
    static Symbol sSupported;
    if (sSupported == Symbol()) {
        sSupported = Symbol("supported");
    }
    static Symbol sCheatSupported;
    if (sCheatSupported == Symbol()) {
        sCheatSupported = Symbol("cheat_supported");
    }
    DataArray* languages = LanguageConfig(cheat ? sCheatSupported : sSupported);
    DataArrayPtr array(languages->Node(1).Array(languages));
    return array;
}

// Reconstructed from eboot.elf at 0x36A070.
Symbol LanguageToAbbrevation(Symbol language) {
    static const char* const kNames[][2] = {
        {"English", "eng"},
        {"Spanish", "esl"},
        {"French", "fre"},
        {"German", "deu"},
        {"Italian", "ita"},
        {"Japanese", "jpn"},
        {"Spanish(LatinAm)", "mex"},
        {"Portuguese(Brazilian)", "ptb"},
        {"Chinese(Traditional)", "cht"},
        {"Chinese(Simplified)", "chs"},
        {"Russian", "rus"},
        {"Finnish", "fin"},
        {"Swedish", "swe"},
        {"Norwegian", "nor"},
    };
    for (const auto& name : kNames) {
        if (strcmp(language.Str(), name[0]) == 0) {
            return Symbol(name[1]);
        }
    }
    return Symbol("");
}

// Reconstructed from eboot.elf at 0x36A280.
Symbol AbbreviationToLanguage(Symbol language) {
    static const char* const kNames[][2] = {
        {"eng", "English"},
        {"esl", "Spanish"},
        {"fre", "French"},
        {"deu", "German"},
        {"ita", "Italian"},
        {"jpn", "Japanese"},
        {"mex", "Spanish(LatinAm)"},
        {"ptb", "Portuguese(Brazilian)"},
        {"cht", "Chinese(Traditional)"},
        {"chs", "Chinese(Simplified)"},
        {"rus", "Russian"},
        {"fin", "Finnish"},
        {"swe", "Swedish"},
        {"nor", "Norwegian"},
    };
    for (const auto& name : kNames) {
        if (strcmp(language.Str(), name[0]) == 0) {
            return Symbol(name[1]);
        }
    }
    return Symbol("");
}

// Reconstructed from eboot.elf at 0x36A490.
bool IsSupportedLanguage(Symbol language, bool cheat) {
    DataArray* languages = SupportedLanguages(cheat);
    for (int i = 0; i < languages->Size(); ++i) {
        if (languages->Node(i).Sym(languages) == language) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x36A500.
void SetSystemLanguage(Symbol language, bool cheat) {
    if (!IsSupportedLanguage(language, cheat)) {
        static Symbol sDefault;
        if (sDefault == Symbol()) {
            sDefault = Symbol("default");
        }
        DataArray* fallback = LanguageConfig(sDefault);
        if (fallback == nullptr) {
            return;
        }
        language = fallback->Sym(1);
        if (!IsSupportedLanguage(language, cheat)) {
            return;
        }
    }
    if (language == gSystemLanguage) {
        gSystemLanguage = language;
    } else {
        TheLocale.Terminate();
        gSystemLanguage = language;
        TheLocale.Init();
    }
}
