#include <cstring>
#include <message_dialog.h>
#include <system_service.h>

#include "os/files/File.h"
#include "os/system/System.h"
#include "utl/data/DataArray.h"
#include "utl/files/FileUtl.h"
#include "utl/options/Option.h"
#include "utl/text/MakeString.h"

namespace {

// The system language, read once from the system software; "   " when the
// game has no translation for it. At 0x19FEBF8.
Symbol sLang;

}  // namespace

// Reconstructed from eboot.elf at 0x3767C0, the empty body every empty
// function of this build is folded into.
void SystemPlatformTerminate() {}

// Reconstructed from eboot.elf at 0x3767D0.
Symbol GetSystemLanguage(Symbol def) {
    if (sLang == Symbol()) {
        static_cast<void>(scePthreadSelf());
        int language;
        sceSystemServiceParamGetInt(SCE_SYSTEM_SERVICE_PARAM_ID_LANG, &language);
        // Indexed by SCE_SYSTEM_PARAM_LANG_*; Portuguese reads as Brazilian
        // and Canadian French as French.
        Symbol languages[] = {
            Symbol("jpn"), Symbol("eng"), Symbol("fre"), Symbol("esl"), Symbol("deu"),
            Symbol("ita"), Symbol("   "), Symbol("ptb"), Symbol("rus"), Symbol("kor"),
            Symbol("cht"), Symbol("chs"), Symbol("fin"), Symbol("swe"), Symbol("   "),
            Symbol("nor"), Symbol("   "), Symbol("ptb"), Symbol("eng"), Symbol("   "),
            Symbol("mex"), Symbol("   "), Symbol("fre"),
        };
        if (static_cast<unsigned int>(language) < sizeof(languages) / sizeof(languages[0])) {
            sLang = languages[language];
        } else {
            sLang = Symbol("   ");
        }
    }
    return strcmp(sLang.Str(), "   ") != 0 ? sLang : def;
}

// Reconstructed from eboot.elf at 0x376A30.
void CaptureStackTrace(void** trace, void*) {
    auto** frame = static_cast<void**>(__builtin_frame_address(1));
    for (long i = 0; frame != nullptr && i < 50 && frame[0] != nullptr; ++i) {
        trace[i] = frame[1];
        frame = static_cast<void**>(frame[0]);
    }
}

// Reconstructed from eboot.elf at 0x376A70.
bool PlatformDebuggerAttached() {
    return false;
}

// Reconstructed from eboot.elf at 0x376A80.
void PlatformDebugBreak() {}

// Reconstructed from eboot.elf at 0x376A90.
void PlatformModalDialog(Symbol, const char* msg, const char*, bool, bool) {
    sceCommonDialogInitialize();
    sceMsgDialogInitialize();
    SceMsgDialogParam param;
    sceMsgDialogParamInitialize(&param);
    SceMsgDialogUserMessageParam userMessage;
    memset(&userMessage, 0, sizeof(userMessage));
    userMessage.msg = msg;
    userMessage.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
    param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
    param.userMsgParam = &userMessage;
    sceMsgDialogOpen(&param);
    sceSystemServiceHideSplashScreen();
    while (sceMsgDialogUpdateStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
    }
    sceMsgDialogTerminate();
}

// Reconstructed from eboot.elf at 0x376B80.
void GetMapFileName(FixedString& out) {
    if (SystemConfig() != nullptr) {
        DataArray* mapFile = SystemConfig(Symbol("system"), Symbol("ps4_map_file"));
        FormatString format(mapFile->Node(1).Str(mapFile));
        format << "s";
        FileQualifiedFilename(out, format.Str(), true);
    } else {
        char path[512];
        strcpy(path, FileGetName(OptionExecutable(gOptionArgs)));
        char* extension = strrchr(path, '.');
        if (extension != nullptr) {
            strcpy(extension, ".map");
        }
        FileQualifiedFilename(out, path, true);
    }
}
