#pragma once

#include <cstddef>

#include "persistence/storage/StorageMgr.h"
#include "utl/streams/MemStream.h"

class Profile;
class StorageRef_PS4;

namespace SaveLoad {

// The results SaveLoadMgr exports with its status messages. Names from
// ErrorToString's table.
enum Error : int {
    kNoError = 0,
    kNoActiveProfile = 1,
    kNoDevice = 2,
    kMultipleDevices = 3,
    kNoSaveNeeded = 4,
    kIncompatibleVersion = 5,
    kDeviceFull = 6,
    kUnknownError = 15,
};

// The error's enumerator name, or "(unknown error code)". In
// persistence/SaveLoadMgr.o.
const char* ErrorToString(int error);  // 0x3ABB80

}  // namespace SaveLoad

// Receives the results of a ProfileStorageClient's operations. The
// interface's own vtable is not emitted; SaveLoadMgr implements it at
// offset 40.
class ProfileStorageClientCallback {
public:
    virtual ~ProfileStorageClientCallback() {}
    // Slot 2.
    virtual void OnProfileSaveResult(SaveLoad::Error error) = 0;
    // Slot 3.
    virtual void OnProfileLoadResult(SaveLoad::Error error) = 0;
    // Slot 4.
    virtual void OnProfileDeleteResult(SaveLoad::Error error) = 0;
};

// Saves and loads one profile at a time through its StorageRef_PS4
// (persistence/ProfileStorageClient.o, 0x3AEEE0-0x3AFB70). The profile is
// serialized into a scratch stream inside a RevisionStream at the
// profile's save revision, encrypted; the store's background read and
// write move the scratch bytes, and the results reach the callback on the
// main thread. The vtable at 0x18FCEF0 has StorageCallback's eight slots;
// the object is 120 bytes.
class ProfileStorageClient : public StorageCallback {
public:
    explicit ProfileStorageClient(ProfileStorageClientCallback* callback);  // 0x3AEEE0
    // Slots 0-1: 0x3AFAB0, 0x3AFB10. Inline in the map's build.
    ~ProfileStorageClient() override {}

    // Slot 2: the find Load started; reads the one save found.
    void OnFindResult(void* context, int result, const eastl::list<StorageRef_PS4*>& refs) override;  // 0x3AF750
    // Slot 4: loads the read bytes on the main thread.
    void OnReadResult(void* context, int result, unsigned char* data, unsigned long size, unsigned long fileSize) override;  // 0x3AF830
    // Slot 5.
    void OnWriteResult(void* context, int result, unsigned char* data, unsigned long size) override;  // 0x3AF980
    // Slot 6.
    void OnDeleteResult(void* context, int result) override;  // 0x3AF9B0

    // Whether a find, read or save is pending or the store is busy.
    bool IsBusy() const;  // 0x3AF650
    // Saves the profile on the main thread when it is dirty or `force` is
    // set.
    SaveLoad::Error Save(bool force);  // 0x3AF090
    void SetDevice(int device);  // 0x3AEF40
    // Finds the profile's save (`find`) or reads it from the store.
    SaveLoad::Error Load(bool find);  // 0x3AF370
    // Reloads the last loaded save over unsaved changes.
    void Restore();  // 0x3AF4A0
    SaveLoad::Error Delete();  // 0x3AF5D0
    // Forgets the profile when it is the client's.
    void ClearProfile(Profile* profile);  // 0x3AF080

    // The main thread's halves of OnReadResult and Save. The map's
    // signatures.
    void _PerformLoad(const int& result);  // 0x3AF8F0
    void _PerformSave(const int& unused);  // 0x3AF1D0
    // Serializes the profile into the scratch stream and marks it clean.
    void _SaveProfileToScratch();  // 0x3AF6A0
    // Loads the profile from the scratch stream. A block that is too old or
    // has no header counts as loaded, keeping the defaults.
    SaveLoad::Error _LoadProfileFromScratch();  // 0x3AF500
    // Saves into the scratch stream and writes it to the store; the body of
    // _PerformSave. No caller in this build. Name not in the reference map;
    // the evidence is weak.
    void _SaveProfile();  // 0x3AF9E0
    // Returns _LoadProfileFromScratch's result. No caller in this build.
    // Name not in the reference map; the evidence is weak.
    SaveLoad::Error _LoadProfile();  // 0x3AF9D0
    // The profile's store, named after its save file when it is idle and
    // unnamed.
    StorageRef_PS4* _GetStore() const;  // 0x3AEF70

    // Field names are not in the reference map.
    MemStream mScratch;
    // Set once the profile was saved or loaded through the client.
    bool mLoaded;
    // Pending finds, reads and saves, counted across threads.
    int mPendingFinds;
    int mPendingReads;
    int mPendingSaves;
    // The profile being saved or loaded. SaveLoadMgr sets it directly.
    Profile* mProfile;
    ProfileStorageClientCallback* mCallback;
};

static_assert(offsetof(ProfileStorageClient, mScratch) == 8);
static_assert(offsetof(ProfileStorageClient, mLoaded) == 88);
static_assert(offsetof(ProfileStorageClient, mPendingFinds) == 92);
static_assert(offsetof(ProfileStorageClient, mPendingReads) == 96);
static_assert(offsetof(ProfileStorageClient, mPendingSaves) == 100);
static_assert(offsetof(ProfileStorageClient, mProfile) == 104);
static_assert(offsetof(ProfileStorageClient, mCallback) == 112);
static_assert(sizeof(ProfileStorageClient) == 120);
