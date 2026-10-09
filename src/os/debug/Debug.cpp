#include "os/debug/Debug.h"

#include <cstdlib>

#include "os/debug/Cheats.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/system/AppChild.h"
#include "os/system/System.h"
#include "os/threading/TLSValue.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"
#include "utl/options/Option.h"
#include "utl/text/MakeString.h"
#include "utl/text/StringTable.h"
#include "utl/threading/ThreadCall.h"

namespace {

// A message raised off the main thread and shown by the next Poll. Name and
// field names not in the reference map.
struct PendingModal {
    PendingModal() : mStackTrace(nullptr) {}

    StackString<1024> mText;
    // Zeroed by the constructor and not otherwise used in this build; the
    // name follows Debug::ModalMessage. Weak.
    void* mStackTrace;
    // Cleared by Poll once the notify is shown; the name follows Notify's
    // show flag. Weak.
    bool* mShow;
};

static_assert(sizeof(PendingModal) == 0x428);

// Guards the pending messages and the reflected streams, at 0x19FBDA8.
// Name not in the reference map.
CritSec gDebugCritSec;
// The failure Poll shows, at 0x19FBDB8. Name not in the reference map.
PendingModal gPendingFail;
// The report shown with it, at 0x19FC1E0. Name not in the reference map.
CrucibleReport gCrucibleReport;
// The notify Poll shows, at 0x19FD718. Name not in the reference map.
PendingModal gPendingNotify;

}  // namespace

Debug TheDebug;

namespace {

// The calling thread's try depth (HmxThrowOnFailObj, SetTry), at 0x19FDD40.
TLSValue<int> tThreadTryCount;
// The messages NotifyUniqueStr has shown, at 0x19FDD90, and their lock at
// 0x19FDDC8. Names not in the reference map.
StringTable gUniqueNotifies("Unique Notifies", 2000, 1);
CritSec gUniqueNotifiesCritSec;

// The dialog titles by ModalType. Name not in the reference map; the
// binary builds the array in each user.
struct ModalTitles {
    ModalTitles() : mTitles{Symbol(""), Symbol("WARN"), Symbol("NOTIFY"), Symbol("FAIL")} {}

    Symbol mTitles[4];
};

}  // namespace

// Reconstructed from eboot.elf at 0x35BA60.
Debug::Debug()
    : mDisabled(false),
      mExiting(false),
      mNoTry(true),
      mLog(nullptr),
      mModalQuestionCallback(nullptr),
      mModalCallback(nullptr),
      mStackTrace(),
      mNotifiesEnabled(true) {}

// Reconstructed from eboot.elf at 0x35BB60.
Debug::~Debug() {}

// Reconstructed from eboot.elf at 0x35BC90.
void Debug::SetModalQuestionCallback(ModalQuestionCallbackFunc* callback) {
    mModalQuestionCallback = callback;
}

// Reconstructed from eboot.elf at 0x35BCA0.
void Debug::SetModalCallback(ModalCallbackFunc* callback) {
    mModalCallback = callback;
}

// Reconstructed from eboot.elf at 0x35BCB0.
void Debug::Init() {
    mNoTry = OptionBool(gOptionArgs, "no_try", false);
    StartLog(OptionStr(gOptionArgs, "log", nullptr), false);
}

// Reconstructed from eboot.elf at 0x3767C0, the empty body every empty
// function of this build is folded into.
void Debug::StartLog(const char*, bool) {}

// Reconstructed from eboot.elf at 0x35BD00.
void Debug::Poll() {
    if (gPendingFail.mText.c_str()[0] != '\0') {
        ScopedCritSec lock(gDebugCritSec);
        ModalMessage msg = {kModalFail, &gPendingFail.mText, nullptr};
        Modal(msg, gCrucibleReport, false);
    }
    if (gPendingNotify.mText.c_str()[0] != '\0') {
        ScopedCritSec lock(gDebugCritSec);
        StackString<1024> text(gPendingNotify.mText.c_str());
        StackString<1024> response;
        gPendingNotify.mText = nullptr;
        gPendingNotify.mShow = nullptr;
        if (!gHmxNoModal) {
            ModalTitles titles;
            PlatformModalDialog(titles.mTitles[kModalNotify], text.c_str(), "Close", false, true);
        }
    }
}

// Reconstructed from eboot.elf at 0x35BF30.
void Debug::Modal(ModalMessage& msg, CrucibleReport& report, bool quiet) {
    const bool holmesClosed = msg.mText->startswith("holmes closed");
    if (mModalCallback != nullptr) {
        mModalCallback(msg.mType, msg.mText->c_str(), report.mCallstack, report.mAttachment);
    }
    if (!holmesClosed) {
        CrucibleErrorReport(report, msg.mType, msg.mText->c_str());
    }
    if (TheAppChild != nullptr) {
        TheAppChild->Sync(2);
    }
    if (!quiet && !gHmxNoModal && msg.mType != kModalWarn) {
        ModalTitles titles;
        PlatformModalDialog(titles.mTitles[msg.mType], msg.mText->c_str(), "Close",
                            msg.mType == kModalFail, true);
    }
    Exit(1, true);
}

// Reconstructed from eboot.elf at 0x35C110.
void Debug::SetTry(bool enable) {
    if (!mNoTry) {
        *tThreadTryCount += enable ? 1 : -1;
    }
}

// Reconstructed from eboot.elf at 0x35C1A0.
bool Debug::IsInTry() const {
    return *tThreadTryCount > 0;
}

// Reconstructed from eboot.elf at 0x35C3B0.
void Debug::Notify(const char*, bool* show) {
    if (show != nullptr && *show && !mNotifiesEnabled) {
        *show = false;
    }
}

// Reconstructed from eboot.elf at 0x35C3B0, which Notify shares.
void Debug::Warn(const char*, bool* show) {
    if (show != nullptr && *show && !mNotifiesEnabled) {
        *show = false;
    }
}

// Reconstructed from eboot.elf at 0x35C3D0.
void Debug::NotifyUniqueStr(const char* msg, bool* show) {
    gUniqueNotifiesCritSec.Enter();
    if (gUniqueNotifies.Contains(msg)) {
        gUniqueNotifiesCritSec.Exit();
        return;
    }
    static long sDebugHeap = MemFindHeap("debug");
    MemPushHeap(sDebugHeap);
    const char* unique = gUniqueNotifies.Add(msg);
    MemPopHeap();
    gUniqueNotifiesCritSec.Exit();
    if (show != nullptr && unique != nullptr && *show && !mNotifiesEnabled) {
        *show = false;
    }
}

// Reconstructed from eboot.elf at 0x35C4C0.
void Debug::Fail(const char*) {
    if (mDisabled) {
        return;
    }
    mDisabled = true;
    static long sFailureHeap = MemFindHeap("failure");
    MemPushHeap(sFailureHeap);
    if (PlatformDebuggerAttached()) {
        PlatformDebugBreak();
    }
    MemPopHeap();
    mDisabled = false;
}

// Reconstructed from eboot.elf at 0x35C540.
void Debug::FillCrucibleReport(CrucibleReport& report, bool withCallstack, void* stack,
                               void* context) {
    report.mChangelist = gAutobuildChangelist;
    report.mPlatform = kPlatformPS4;
    const char* configFile = SystemConfig() != nullptr ? SystemConfig()->mFile.Str() : "<unknown>";
    report.mConfigFile = StackString<128>(configFile).c_str();
    report.mUptimeMs = SystemMs();
    report.mValid = true;
    report.mLanguage = SystemLanguage();
    report.mUserLabel = StackString<128>(report.mUser.c_str()).c_str();
    FormatString version("%x_%03x");
    version << 0x5008u << 0x91u;
    report.mSdkVersion = StackString<64>(version.Str()).c_str();
    if (theCheatsManager != nullptr) {
        theCheatsManager->AppendLog(report.mCheats);
    }
    if (!withCallstack) {
        return;
    }
    DataAppendStackTrace(report.mDataStack);
    if (stack != nullptr) {
        AppendStackTrace(report.mCallstack, stack);
    } else {
        void* trace[50] = {};
        CaptureStackTrace(trace, context);
        AppendStackTrace(report.mCallstack, trace);
    }
}

// Reconstructed from eboot.elf at 0x35C950.
void Debug::Exit(int code, bool quit) {
    if (mExiting) {
        return;
    }
    mExiting = true;
    ThreadCallTerminate();
    for (auto it = mExitCallbacks.begin(); it != mExitCallbacks.end(); ++it) {
        (*it)();
    }
    mExitCallbacks.clear();
    if (quit) {
        exit(code);
    }
}

// Reconstructed from eboot.elf at 0x35CA00.
void Debug::RemoveExitCallback(ExitCallbackFunc* callback) {
    if (mExiting) {
        return;
    }
    for (auto it = mExitCallbacks.begin(); it != mExitCallbacks.end();) {
        if (*it == callback) {
            it = mExitCallbacks.erase(it);
        } else {
            ++it;
        }
    }
}

// Reconstructed from eboot.elf at 0x35CA80.
void Debug::Print(const char*) {}

// Reconstructed from eboot.elf at 0x35CA90.
void Debug::AddReflect(TextStream* stream) {
    ScopedCritSec lock(gDebugCritSec);
    mReflects.push_front(stream);
}

// Reconstructed from eboot.elf at 0x35CB10.
void Debug::RemoveReflect(TextStream* stream) {
    ScopedCritSec lock(gDebugCritSec);
    for (auto it = mReflects.begin(); it != mReflects.end();) {
        if (*it == stream) {
            it = mReflects.erase(it);
        } else {
            ++it;
        }
    }
}

// Reconstructed from eboot.elf at 0x35CBD0.
void HmxNotify(const char* msg, bool* show) {
    TheDebug.Notify(msg, show);
}

// Reconstructed from eboot.elf at 0x35CBF0.
void HmxNotifyUniqueStr(const char* msg, bool* show) {
    TheDebug.NotifyUniqueStr(msg, show);
}

// Reconstructed from eboot.elf at 0x35CC10.
void HmxWarn(const char* msg, bool* show) {
    TheDebug.Warn(msg, show);
}

// Reconstructed from eboot.elf at 0x35CC30.
void HmxFail(const char* msg) {
    TheDebug.Fail(msg);
}

// Reconstructed from eboot.elf at 0x35CC50.
void HmxExit(int code) {
    TheDebug.Exit(code, true);
    __builtin_unreachable();
}

// Reconstructed from eboot.elf at 0x35CD00.
HmxThrowOnFailObj::HmxThrowOnFailObj(bool enable) : mEnabled(enable) {
    if (!TheDebug.mNoTry && enable) {
        ++*tThreadTryCount;
    }
}

// Reconstructed from eboot.elf at 0x35CD90.
HmxThrowOnFailObj::~HmxThrowOnFailObj() {
    if (mEnabled && !TheDebug.mNoTry) {
        --*tThreadTryCount;
    }
}

// Reconstructed from eboot.elf at 0x366FD0.
void CrucibleErrorReport(CrucibleReport&, Debug::ModalType, const char*) {}
