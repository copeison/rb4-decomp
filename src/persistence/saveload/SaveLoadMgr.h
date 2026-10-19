#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"
#include "persistence/saveload/ProfileStorageClient.h"
#include "persistence/saveload/SystemOptionsStorageClient.h"
#include "persistence/storage/StorageMgr.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class DataArray;
class Profile;
class ProfileMgr;

// Lists the files StorageMgr finds; its find result walks them without
// using them. SaveLoadMgr::DumpFiles keeps one as a function-local static
// at 0x19BDC18. The vtable is at 0x18FCDC0. Inline in the map's build.
class FileDumper : public StorageCallback {
public:
    ~FileDumper() override {}  // Slots 0-1: 0x3AE950, 0x3AEDF0.

    // Slot 2.
    void OnFindResult(void* context, int result, const eastl::list<StorageRef_PS4*>& refs) override {  // 0x3AEE00
        static_cast<void>(context);
        static_cast<void>(result);
        for (const eastl::ListNodeBase* node = refs.mNode.mpNext; node != &refs.mNode; node = node->mpNext) {
        }
    }
};

// Saves and loads the profiles and the system options and reports progress
// to scripts (persistence/SaveLoadMgr.o, 0x3ABB80-0x3AEEDF). It runs one
// profile operation at a time, queueing the others, exports
// "<operation>_start" and "<operation>_end" messages with an error and a pad
// on the main thread, and handles the device prompts. theSaveLoadMgr holds
// the one instance, which scripts reach as the "saveload_mgr" global. The
// vtable at 0x18FCC80 has 14 slots, with StorageCallback's at 0x18FCD00
// (offset 32), ProfileStorageClientCallback's at 0x18FCD50 (offset 40) and
// SystemOptionsStorageClientCallback's at 0x18FCD88 (offset 48); the object
// is 464 bytes.
class SaveLoadMgr : public SyncMsgSource,
                    public StorageCallback,
                    public ProfileStorageClientCallback,
                    public SystemOptionsStorageClientCallback {
public:
    // A status message: its type, the error and the pad (-1 for the system
    // options). The map names the type; the field names are not in the
    // reference map.
    struct MsgParams {
        Symbol mMsg;
        SaveLoad::Error mError;
        int mPadNum;
    };

    // A queued profile operation. The map names the types; the field names
    // are not in the reference map.
    struct SaveProfile {
        Profile* mProfile;
        bool mForce;
    };
    struct LoadProfile {
        Profile* mProfile;
        bool mFind;
    };

    // What a device prompt was for: its context. Names not in the reference
    // map.
    enum DevicePrompt {
        kPromptChooseDevice = 0,
        kPromptSaveNewDevice = 1,
        kPromptSaveSystemOptions = 2,
        kPromptLoadSystemOptions = 3,
    };

    // Reads the "viper_autosave" config and subscribes to the storage and
    // profile removal events.
    explicit SaveLoadMgr(ProfileMgr* profileMgr);  // 0x3ABC70
    // Slots 0-1: 0x3AC160, 0x3AC3D0; thunks 0x3AC3A0/0x3AC3F0 (offset 32),
    // 0x3AC3B0/0x3AC420 (40) and 0x3AC3C0/0x3AC450 (48).
    ~SaveLoadMgr() override;
    // Slot 2: "on_profile_about_to_be_removed" and "save_all".
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x3AEAD0
    // Slot 7 (thunk 0x3AE250): the device the user chose for the prompt.
    void OnDevicePromptResult(void* context, int result, int device) override;  // 0x3ADD50
    // Slots 8-10 (thunks 0x3AE3A0, 0x3AE4F0, 0x3AE640): export
    // "save_end", "load_end" and "delete_end" for the client's profile.
    void OnProfileSaveResult(SaveLoad::Error error) override;    // 0x3AE260
    void OnProfileLoadResult(SaveLoad::Error error) override;    // 0x3AE3B0
    void OnProfileDeleteResult(SaveLoad::Error error) override;  // 0x3AE500
    // Slots 11-13 (thunks 0x3AE770, 0x3AE8A0, 0x3AE8C0): export
    // "saveopt_end" and "loadopt_end"; a deletion is not reported.
    void OnSysOptionsSaveResult(SaveLoad::Error error) override;  // 0x3AE650
    void OnSysOptionsLoadResult(SaveLoad::Error error) override;  // 0x3AE780
    void OnSysOptionsDeleteResult(SaveLoad::Error error) override;  // 0x3AE8B0

    // Creates theSaveLoadMgr and sets the "saveload_mgr" global.
    static void Init(ProfileMgr* profileMgr);  // 0x3ABBA0

    // Saves every dirty profile, or every profile when forced.
    void SaveAll(bool force);  // 0x3AC480
    // Saves the profile, or queues it while another operation runs. Without
    // autosave only a forced save runs.
    void Save(Profile* profile, bool force);  // 0x3AC510
    // Exports a status message on the main thread.
    void ExportStatus(const Symbol& msg, int padNum, SaveLoad::Error error);  // 0x3AC830
    // Saves the client's profile and exports "save_start"/"save_end".
    void ExecuteSave(bool force);  // 0x3AC8E0
    // Saves the profile with a client of its own and waits for the result.
    SaveLoad::Error SaveSync(Profile* profile, bool force);  // 0x3ACBA0
    void UpdateIsBusy();  // 0x3ACCB0
    // Asks the pad's user for a device for a new save.
    void SaveNewDevice(Profile* profile);  // 0x3ACCE0
    // Saves the profile to the device chosen by the prompt.
    void ConfirmSaveNewDevice(Profile* profile);  // 0x3ACD30
    // Overwrites the system options on the device chosen by the prompt.
    void ConfirmOptionsOverwrite();  // 0x3ACD80
    // Saves the system options and exports "saveopt_start"/"saveopt_end".
    void SaveSystemOptions(bool force);  // 0x3ACDE0
    // Loads the profile, or queues it while another operation runs, and
    // exports "load_start"/"load_end".
    void Load(Profile* profile, bool find);  // 0x3AD080
    void RestoreFromLastSave(Profile* profile);  // 0x3AD470
    void SaveSystemOptionsPrompt();  // 0x3AD480
    // Finds and loads the system options and exports "loadopt_start" and
    // "loadopt_end".
    void AutoloadSystemOptions();  // 0x3AD4B0
    void LoadSystemOptionsPrompt();  // 0x3AD700
    void DeleteSystemOptions();  // 0x3AD730
    // Deletes the profile's save and exports "delete_start"/"delete_end".
    void DeleteSave(Profile* profile);  // 0x3AD740
    // Exports "choosedevice_start" and asks the pad's user for a device.
    void ChooseDevice(Profile* profile);  // 0x3ADA30
    void ToggleAutosave();  // 0x3ADB40
    // Exports the status message as (type error pad).
    void ExportMsg(const MsgParams& params);  // 0x3ADB50
    // Runs the queued load, else the queued save, once both clients are
    // idle.
    void Poll();  // 0x3AE960
    // Lists the pad's files through a FileDumper.
    void DumpFiles(Profile* profile);  // 0x3AE8D0
    // Forgets the profile in the client and the queues. Inlined into
    // Handle.
    void _OnProfileAboutToBeRemoved(Profile* profile);

    // Field names are not in the reference map.
    // Whether unforced profile saves run ("viper_autosave" "enable").
    bool mAutosave;
    bool mIsBusy;
    // The device a confirmation prompt waits on, or kAnyDevice.
    int mPendingDevice;
    // Set to the empty symbol by the constructor and never read. The
    // evidence for the name is weak.
    Symbol mPendingMsg;
    ProfileStorageClient mProfileClient;
    SystemOptionsStorageClient mSysOptionsClient;
    ProfileMgr* mProfileMgr;
    eastl::vector<SaveProfile> mPendingSaves;
    eastl::vector<LoadProfile> mPendingLoads;
    // The platform's "storage_changed" as "on_storage_changed", and the
    // profile manager's "profile_about_to_be_removed" as
    // "on_profile_about_to_be_removed".
    MsgSource::EventSinkElem mStorageChangedSink;
    MsgSource::EventSinkElem mProfileRemovedSink;
    // The lock SyncMsgSource exports under.
    CritSec mCritSec;
};

static_assert(sizeof(SaveLoadMgr::MsgParams) == 16);
static_assert(sizeof(SaveLoadMgr::SaveProfile) == 16);
static_assert(offsetof(SaveLoadMgr, mAutosave) == 56);
static_assert(offsetof(SaveLoadMgr, mPendingDevice) == 60);
static_assert(offsetof(SaveLoadMgr, mPendingMsg) == 64);
static_assert(offsetof(SaveLoadMgr, mProfileClient) == 72);
static_assert(offsetof(SaveLoadMgr, mSysOptionsClient) == 192);
static_assert(offsetof(SaveLoadMgr, mProfileMgr) == 296);
static_assert(offsetof(SaveLoadMgr, mPendingSaves) == 304);
static_assert(offsetof(SaveLoadMgr, mPendingLoads) == 336);
static_assert(offsetof(SaveLoadMgr, mStorageChangedSink) == 368);
static_assert(offsetof(SaveLoadMgr, mProfileRemovedSink) == 408);
static_assert(offsetof(SaveLoadMgr, mCritSec) == 448);
static_assert(sizeof(SaveLoadMgr) == 464);

// The save manager. At 0x1A6E0A8.
extern SaveLoadMgr* theSaveLoadMgr;
