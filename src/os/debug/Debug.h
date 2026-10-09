#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"
#include "utl/containers/List.h"
#include "utl/text/Str.h"
#include "utl/text/TextStream.h"

class CrucibleReport;

// The engine's debug channel (os/Debug.o): a TextStream that logs what is
// printed to it, plus the notify, warn and fail services and the exit
// callbacks. This build's release bodies keep only the bookkeeping; the
// printing, logging and modal dialogs of the map's build are compiled out.
// The vtable is at 0x18FAF08.
class Debug : public TextStream {
public:
    // The severity of a modal message. The enumerator names are not in the
    // reference map; they follow the dialog titles Modal uses ("", "WARN",
    // "NOTIFY", "FAIL").
    enum ModalType {
        kModalNone = 0,
        kModalWarn = 1,
        kModalNotify = 2,
        kModalFail = 3,
    };

    // A message for Modal: its type, its text and the stack trace it was
    // raised from. Poll builds one on the stack. Name not in the reference
    // map, whose Modal(ModalType&, char const*) takes the parts.
    struct ModalMessage {
        ModalType mType;
        FixedString* mText;
        void* mStackTrace;
    };

    // Called with each message Modal shows. The map's
    // SetModalCallback(void (*)(ModalType&, FixedString&, bool)) takes
    // another signature; this build passes the type, the text and the
    // report's call stack and attachment.
    typedef void ModalCallbackFunc(ModalType type, const char* msg, FixedString& callstack,
                                   FixedString& attachment);
    // Never called in this build; the type follows the map's
    // SetModalQuestionCallback(int (*)(FixedString&, FixedString&)).
    typedef int ModalQuestionCallbackFunc(FixedString& question, FixedString& answer);
    typedef void ExitCallbackFunc();

    // Inlined into the static initializer at 0x35CF70.
    Debug();  // 0x35BA60
    ~Debug() override;                    // slots 0-1: 0x35BB60, 0x35BC00
    // Empty in this build.
    void Print(const char* str) override;  // slot 2: 0x35CA80

    // Reads the no_try option. The log the map's build starts from the
    // "log" option is compiled out.
    void Init();  // 0x35BCB0
    // Empty in this build.
    void StartLog(const char* file, bool append);  // 0x3767C0
    // Shows the pending failure and notify messages.
    void Poll();  // 0x35BD00
    // Reports a failure. This build's release body only runs the fail
    // callbacks on the "failure" heap, guarded against reentry.
    void Fail(const char* msg);  // 0x35C4C0
    // The release bodies only clear the caller's show flag once a message
    // was shown while notifies are disabled. Notify and Warn are folded
    // into one body.
    void Warn(const char* msg, bool* show);    // 0x35C3B0
    void Notify(const char* msg, bool* show);  // 0x35C3B0
    // As Notify, for a message not seen before; each one is remembered.
    void NotifyUniqueStr(const char* msg, bool* show);  // 0x35C3D0
    // Counts the calling thread's try scopes; no_try disables them.
    void SetTry(bool enable);  // 0x35C110
    bool IsInTry() const;      // 0x35C1A0
    // Shows the message through the modal callback and the platform dialog
    // and exits. The map has Modal(ModalType&, char const*).
    void Modal(ModalMessage& msg, CrucibleReport& report, bool quiet);  // 0x35BF30
    // Fills the report with the build and platform details, the cheats used
    // and the call stack: `stack` when given, otherwise the caller's,
    // captured with `context`. Name not in the reference map; no caller
    // remains in this build.
    void FillCrucibleReport(CrucibleReport& report, bool withCallstack, void* stack,
                            void* context);  // 0x35C540
    // Runs the exit callbacks once and, when `quit` is set, exits.
    void Exit(int code, bool quit);  // 0x35C950

    // Inlined into its users, for example Rnd::Init at 0x402CE0. Name not in
    // the reference map, which has RemoveExitCallback(void (*)()).
    void AddExitCallback(ExitCallbackFunc* callback) {
        mExitCallbacks.push_front(callback);
    }
    void RemoveExitCallback(ExitCallbackFunc* callback);  // 0x35CA00
    // Adds and removes a stream that receives a copy of what is printed;
    // the console overlay registers its output (0x6E1910, 0x6E1930). Names
    // not in the reference map.
    void AddReflect(TextStream* stream);     // 0x35CA90
    void RemoveReflect(TextStream* stream);  // 0x35CB10

    // The map gives these in the order SetModalCallback,
    // SetModalQuestionCallback; which member each one sets is inferred
    // from the callback Modal calls, so the pairing is weak.
    void SetModalQuestionCallback(ModalQuestionCallbackFunc* callback);  // 0x35BC90
    void SetModalCallback(ModalCallbackFunc* callback);                  // 0x35BCA0

    // Set while Fail runs so that a failure inside it is ignored. The map's
    // SetDisabled(bool) suggests the name. Name not in the reference map.
    bool mDisabled;
    // Set by Exit before the exit callbacks run; resources are no longer
    // released once it is set (Resource::ReleaseRef, 0x1ADEF0). Name not in
    // the reference map.
    bool mExiting;
    // The no_try option; try scopes are not counted when it is set. Name not
    // in the reference map.
    bool mNoTry;
    // Zeroed by the constructor and not otherwise used in this build; the
    // map's StartLog and StopLog suggest the log stream. Name not in the
    // reference map, weak.
    TextStream* mLog;
    // Names not in the reference map.
    eastl::list<TextStream*> mReflects;
    ModalQuestionCallbackFunc* mModalQuestionCallback;
    ModalCallbackFunc* mModalCallback;
    eastl::list<ExitCallbackFunc*> mExitCallbacks;
    // A captured call stack; zeroed by the constructor. Name not in the
    // reference map.
    void* mStackTrace[50];
    // Starts set. While it is clear, a shown notify clears the caller's show
    // flag so that it is not shown again; the map's DisableCurrentNotify
    // suggests its use. Name not in the reference map.
    bool mNotifiesEnabled;
};

static_assert(offsetof(Debug, mExiting) == 9);
static_assert(offsetof(Debug, mLog) == 0x10);
static_assert(offsetof(Debug, mReflects) == 0x18);
static_assert(offsetof(Debug, mModalCallback) == 0x40);
static_assert(offsetof(Debug, mExitCallbacks) == 0x48);
static_assert(offsetof(Debug, mStackTrace) == 0x68);
static_assert(offsetof(Debug, mNotifiesEnabled) == 0x1F8);
static_assert(sizeof(Debug) == 0x200);

// The global debug stream, at 0x19FDB40.
extern Debug TheDebug;

// The details a failure report carries, filled by
// Debug::FillCrucibleReport. The map's Crucible reporting has no matching
// type; name and field names not in the reference map.
class CrucibleReport {
public:
    // Copied into mUserLabel; not otherwise set in this build.
    StackString<32> mUser;
    // The build's changelist (the constant at 0x1245E64, 1299228).
    int mChangelist;
    int mPlatform;  // HxPlatform, kPlatformPS4.
    // The system configuration's file, or "<unknown>".
    StackString<128> mConfigFile;
    int mUptimeMs;
    bool mValid;
    Symbol mLanguage;
    StackString<128> mUserLabel;
    // The SDK version, "5008_091".
    StackString<64> mSdkVersion;
    // The cheats used, as CheatsManager::AppendLog writes them.
    StackString<256> mCheats;
    // The script call stack, from DataAppendStackTrace, which is empty in
    // this build.
    StackString<512> mDataStack;
    StackString<2048> mCallstack;
    // Passed to the modal callback; not filled in this build.
    StackString<2048> mAttachment;
};

static_assert(offsetof(CrucibleReport, mChangelist) == 0x38);
static_assert(offsetof(CrucibleReport, mConfigFile) == 0x40);
static_assert(offsetof(CrucibleReport, mUptimeMs) == 0xD8);
static_assert(offsetof(CrucibleReport, mLanguage) == 0xE0);
static_assert(offsetof(CrucibleReport, mUserLabel) == 0xE8);
static_assert(offsetof(CrucibleReport, mSdkVersion) == 0x180);
static_assert(offsetof(CrucibleReport, mCheats) == 0x1D8);
static_assert(offsetof(CrucibleReport, mDataStack) == 0x2F0);
static_assert(offsetof(CrucibleReport, mCallstack) == 0x508);
static_assert(offsetof(CrucibleReport, mAttachment) == 0xD20);
static_assert(sizeof(CrucibleReport) == 0x1538);

// Sends the report to Crucible. Empty in this build. The map has
// CrucibleErrorReport(Debug::ModalType, char const*, DataPoint&).
void CrucibleErrorReport(CrucibleReport& report, Debug::ModalType type, const char* msg);  // 0x366FD0

// Counts a try scope on the calling thread for its lifetime.
class HmxThrowOnFailObj {
public:
    explicit HmxThrowOnFailObj(bool enable);  // 0x35CD00
    ~HmxThrowOnFailObj();                     // 0x35CD90

private:
    bool mEnabled;  // Name not in the reference map.
};

// The wrappers over TheDebug.
void HmxNotify(const char* msg, bool* show);           // 0x35CBD0
void HmxNotifyUniqueStr(const char* msg, bool* show);  // 0x35CBF0
void HmxWarn(const char* msg, bool* show);             // 0x35CC10
void HmxFail(const char* msg);                         // 0x35CC30
[[noreturn]] void HmxExit(int code);                   // 0x35CC50

// Whether modal dialogs are suppressed, from the no_modal option that
// core_initialize (0x219AA0) reads. At 0x19E76FC; the map places it in
// utl/Base.o.
extern bool gHmxNoModal;
