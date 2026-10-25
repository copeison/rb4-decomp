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

// Writes the platform's copy of a path into `buffer` and returns it: the
// platform folder (PlatformSymbol) in front unless the path is local, and,
// when `resourceExt` is set, "_<platform>" after the extensions of
// registered resource types ("dta" becomes "dta_<platform>"). Name not in
// the reference map; the evidence for it is weak.
char* MakePlatformPath(const char* path, bool resourceExt, char* buffer);  // 0x364E40

#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"

#include <functional>

class OnlineID;
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
    // Slot 15: whether the user is among the logged-in users the user
    // service lists. Name not in the reference map.
    virtual bool IsUserLoggedIn(int user) const;  // 0x370D00
    // Slot 16: the user's network availability from the NP state, 0 to 3.
    // Weak evidence for the name, taken from the map's
    // GetOnlineRestriction(int) const.
    virtual int GetOnlineRestriction(int user) const;  // 0x370D80
    // Slots 17-18: asynchronous network availability and privilege checks
    // that call back when done. Names not in the reference map; the
    // callback signatures are not recovered, and the evidence for both is
    // weak.
    virtual void CheckOnlineAvailability(int user, const std::function<void(int)>& callback);  // 0x370E30
    virtual void CheckOnlinePrivilege(int user, bool showDialog, const std::function<void(int)>& callback);  // 0x371000
    // Slot 19: the user's account id, or an unset id.
    virtual void GetOnlineID(int user, OnlineID* id) const;  // 0x371170
    // Slot 20: exports "platform_identity_changed". Name not in the
    // reference map.
    virtual void ExportPlatformIdentityChanged();  // 0x363930
    // Slot 21: the user service's name for the user.
    virtual const char* GetName(int user) const;  // 0x3711E0

    // Refreshes the network state (vtable +384) and returns the cached
    // link flag (+128). The evidence for the map's name is weak.
    bool IsEthernetCableConnected();  // 0x363760
    // Whether every product the marker stands for is installed. The map's
    // name; the match rests on the signature, so it is weak.
    bool IsMarkerInstalled(Symbol marker) const;  // 0x364760

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

// The system overlay showed or hid: (target "system_overlay_status_changed"
// showing). Inline; RBOvershellCom.o emits the array constructor at
// 0xBF6A80. The vtable is at 0x18FB9A8.
class SystemOverlayStatusChangedMsg : public Message<1> {
public:
    // The type check of the map's build survives as the evaluation of the
    // message's type node.
    explicit SystemOverlayStatusChangedMsg(DataArray* msg) : Message<1>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }
    ~SystemOverlayStatusChangedMsg() override {}

    // The message type, built on first use (0x19FEBA0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("system_overlay_status_changed");
        }
        return t;
    }

    // Name not in the reference map.
    bool IsShowing() const {
        return mData->Int(2) != 0;
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

// The server connection changed: (target "connection_status_changed"
// connected). Newer than the map; the binary keeps the array constructor at
// 0xD597E0. Name not in the reference map, taken from the event.
class ConnectionStatusChangedMsg : public Message<1> {
public:
    explicit ConnectionStatusChangedMsg(DataArray* msg) : Message<1>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x19FEBE0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("connection_status_changed");
        }
        return t;
    }

    // Whether the server is reachable (node 2). Name not in the reference
    // map.
    bool IsConnected() const {
        return mData->Int(2) != 0;
    }
};

// A pad's entitlements arrived: (target "platform_entitlements_retrieved"
// success pad entitlements). Newer than the map; RBProfileMgr.o emits the
// array constructor at 0xD62750. Name not in the reference map, taken from
// the event.
class PlatformEntitlementsRetrievedMsg : public Message<3> {
public:
    explicit PlatformEntitlementsRetrievedMsg(DataArray* msg) : Message<3>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x19FEB80).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("platform_entitlements_retrieved");
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
