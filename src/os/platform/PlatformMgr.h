#pragma once

#include <cstdint>

#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// The engine's platform enumeration. Name in the reference map; the
// enumerator names are not. Values 1, 2, 4 and 6 have empty names in this
// build's platform table.
enum HxPlatform : std::uint32_t {
    kPlatformNone = 0,
    kPlatformPC = 3,
    kPlatformXB1 = 5,
    kPlatformPS4 = 7,
    kPlatformAndroid = 8,
    kPlatformIOS = 9,
    kPlatformOSX = 10,
    kPlatformTVOS = 11,
    kPlatformNX = 12,
    kNumPlatforms = 13,
};

// The graphics APIs a platform's configuration can select. Names not in the
// reference map.
enum HxGfxApi : std::uint32_t {
    kGfxApiNull = 0,
    kGfxApiDX11 = 1,
    kGfxApiPS4 = 2,
    kGfxApiMetal = 3,
    kGfxApiVulkan = 4,
    kGfxApiNX = 5,
    kGfxApiGLES3 = 6,
    kNumGfxApis = 7,
};

// The platform's short name ("ps4"), or the empty symbol for a platform
// without one.
Symbol PlatformSymbol(HxPlatform platform);  // 0x363030

// The graphics API's short name ("ps4"), or "" when out of range. Name not
// in the reference map; the binary places it in entity/Resource.o, whose
// GetCachedResourcePath uses it.
Symbol GfxApiSymbol(HxGfxApi api);  // 0x1AE4D0

// The platform ids listed in the platform_mgr config block's
// supported_platforms. The map has PlatformMgr::GetSupportedPlatforms()
// const; this build's function takes no manager object.
eastl::vector<int> GetSupportedPlatforms();  // 0x3641B0

#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"

struct SceSaveDataDirName;
struct SceSaveDataTitleId;

// The platform services (os/PlatformMgr.o and os/PlatformMgr_PS4.o). Only
// the slots and members other reconstructed code calls are declared; the
// class is not reconstructed. The vtable is at 0x18FB748.
class PlatformMgr : public SyncMsgSource {
public:
    PlatformMgr();  // 0x36EB30
    ~PlatformMgr() override;  // slots 0-1: 0x36ECA0, 0x36ED50
    DataNode Handle(DataArray* msg, bool warn) override;  // slot 2: 0x3651B0
    virtual void PreInit();  // slot 7: 0x36E850
    virtual void Init();     // slot 8: 0x36F650
    virtual void Poll();     // slot 9: 0x36FA30
    // Slot 10: the user the application started with, or -1. Weak
    // evidence for the name, taken from the map's GetDefaultUser.
    virtual int GetDefaultUser() const;  // 0x36F580
    // Slot 11: the initial user, else the first signed-in user, else -1.
    // Weak evidence for the name, taken from the map's GetAnyUser.
    virtual int GetAnyUser() const;  // 0x370BB0
    // Slot 12: whether the user is among the signed-in users.
    virtual bool IsSignedIn(unsigned int user) const;  // 0x370C30
    // Slot 13: whether the user is signed into the network.
    virtual bool IsSignedIntoLive(int user) const;  // 0x370CB0
    // Slot 14: empty on the PS4. ProfileMgr calls it with itself once the
    // application resumes. Name not in the reference map; the evidence is
    // weak.
    virtual void RefreshSignedInUsers(MsgSink* requester);  // 0x363800

    // Creates and deletes data://write.test once, recording whether the
    // developer data volume is writable. Name not in the reference map.
    void CheckDataWritable();  // 0x372440
    // Fills the per-controller lag table, kJoypadExtraLagOffsets. Name not
    // in the reference map.
    static void InitJoypadExtraLags();  // 0x390060

    // Shows the system save data dialog for an error. StorageRef_PS4 passes
    // mode 1, the user, no title, the save directory, the error type (4
    // for a full disk, 6 for broken data) and the blocks the mount needed.
    void ShowSaveDataDialog(int mode, int user, SceSaveDataTitleId* titleId, SceSaveDataDirName* dirName, int errorType, unsigned long requiredBlocks);  // 0x373120
};

// The platform manager, set by PreInit. At 0x19FE0B0. The PS4 half keeps
// copies at 0x19FE980 and 0x19FE988.
extern PlatformMgr* ThePlatformMgr;

// A user signed in: (target "user_signed_in" user). Inline in the map's
// build; the binary keeps the array constructor at 0x33CEC0.
class UserSignedInMsg : public Message<1> {
public:
    explicit UserSignedInMsg(DataArray* msg) : Message<1>(msg) {}
    ~UserSignedInMsg() override {}

    // The message type, built on first use (0x19FBAA0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("user_signed_in");
        }
        return t;
    }

    // Name not in the reference map.
    int GetUser() const {
        return mData->Int(2);
    }
};

// A user signed out: (target "user_signed_out" user). Inline in the map's
// build; the binary keeps the array constructor at 0x33D000.
class UserSignedOutMsg : public Message<1> {
public:
    explicit UserSignedOutMsg(DataArray* msg) : Message<1>(msg) {}
    ~UserSignedOutMsg() override {}

    // The message type, built on first use (0x19FBAB0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("user_signed_out");
        }
        return t;
    }

    // Name not in the reference map.
    int GetUser() const {
        return mData->Int(2);
    }
};

// The application was suspended or resumed: (target
// "app_suspend_state_changed" suspended). Inline in the map's build; the
// binary emits the array constructor at 0x3B41B0, the destructors at
// 0x3B4140 and 0x3B42D0 and the vtable at 0x18FD300 with ProfileMgr.
class AppSuspendStateChangedMsg : public Message<1> {
public:
    // The type check of the map's build survives as the evaluation of the
    // message's type node.
    explicit AppSuspendStateChangedMsg(DataArray* msg) : Message<1>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg) == Event());
    }
    ~AppSuspendStateChangedMsg() override {}

    // The message type, built on first use (0x1A6E380).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("app_suspend_state_changed");
        }
        return t;
    }

    // Name not in the reference map.
    bool IsSuspended() const {
        return mData->Int(2) != 0;
    }
};

// The signed-in identity changed: (target "platform_identity_changed").
// Newer than the map; the binary emits the array constructor at 0x3B42F0
// with ProfileMgr and the vtable at 0x18FB308. Name not in the reference
// map, taken from the event.
class PlatformIdentityChangedMsg : public Message<0> {
public:
    explicit PlatformIdentityChangedMsg(DataArray* msg) : Message<0>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg) == Event());
    }
    ~PlatformIdentityChangedMsg() override {}

    // The message type, built on first use (0x19FE2E0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("platform_identity_changed");
        }
        return t;
    }
};

// The storage devices changed: "storage_changed". Only the event is used in
// this build.
class StorageChangedMsg {
public:
    // The message type, built on first use (0x1A6E268).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("storage_changed");
        }
        return t;
    }
};

// Creates the platform manager, a function-local static at 0x19FE9D0, and
// runs its PreInit. Name not in the reference map.
void PlatformMgrPreInit();  // 0x36E7E0

// The component that exposes the platform to scripts ("Allows access to the
// current platform"). Only its registration is declared.
class PlatformMgrCom {
public:
    static void Init();  // 0x3694A0
};
