#pragma once

#include <cstddef>

#include "persistence/saveload/ProfileStorageClient.h"
#include "persistence/storage/StorageMgr.h"
#include "utl/streams/MemStream.h"

class ProfileMgr;
class StorageRef_PS4;

// Receives the results of a SystemOptionsStorageClient's operations. The
// interface's own vtable is not emitted; SaveLoadMgr implements it at
// offset 48.
class SystemOptionsStorageClientCallback {
public:
    virtual ~SystemOptionsStorageClientCallback() {}
    // Slot 2.
    virtual void OnSysOptionsSaveResult(SaveLoad::Error error) = 0;
    // Slot 3.
    virtual void OnSysOptionsLoadResult(SaveLoad::Error error) = 0;
    // Slot 4.
    virtual void OnSysOptionsDeleteResult(SaveLoad::Error error) = 0;
};

// Saves and loads the profile manager's system options through their
// StorageRef_PS4 (persistence/SystemOptionsStorageClient.o,
// 0x3AFBE0-0x3B0A30), as ProfileStorageClient does for a profile but at
// revision 0, unencrypted and without counting pending work. The vtable at
// 0x18FCF68 has StorageCallback's eight slots; the object is 104 bytes.
class SystemOptionsStorageClient : public StorageCallback {
public:
    SystemOptionsStorageClient(SystemOptionsStorageClientCallback* callback, ProfileMgr* profileMgr);  // 0x3AFBE0
    // Slots 0-1: 0x3B0970, 0x3B09D0. Inline in the map's build.
    ~SystemOptionsStorageClient() override {}

    // Slot 2: the find Load started; reads the one save found.
    void OnFindResult(void* context, int result, const eastl::list<StorageRef_PS4*>& refs) override;  // 0x3B03F0
    // Slot 4: loads the read bytes on the main thread.
    void OnReadResult(void* context, int result, unsigned char* data, unsigned long size, unsigned long fileSize) override;  // 0x3B04A0
    // Slot 5.
    void OnWriteResult(void* context, int result, unsigned char* data, unsigned long size) override;  // 0x3B07A0
    // Slot 6.
    void OnDeleteResult(void* context, int result) override;  // 0x3B07D0

    bool IsBusy() const;  // 0x3AFC20
    void SetDevice(int device);  // 0x3AFD70
    // Saves the options on the main thread when they are dirty or `force`
    // is set, giving the store the best device when it has none.
    SaveLoad::Error Save(bool force);  // 0x3AFDA0
    // Finds the options' save (`find`) or reads it from the store.
    SaveLoad::Error Load(bool find);  // 0x3B0150
    SaveLoad::Error Delete();  // 0x3B0240

    // The main thread's halves of Save and OnReadResult. The map's
    // signatures.
    void PerformSave(const int& unused);  // 0x3AFFF0
    void PerformLoad(const int& result);  // 0x3B0660
    // Serializes the options into the scratch stream and marks them clean.
    void SaveOptionsToScratch();  // 0x3B02A0
    // Loads the options from the scratch stream. A block that is too old or
    // has no header counts as loaded.
    SaveLoad::Error LoadOptionsFromScratch();  // 0x3B0330
    // A second copy of LoadOptionsFromScratch's body. No caller in this
    // build. Name not in the reference map; the evidence is weak.
    SaveLoad::Error LoadOptions();  // 0x3B07F0
    // Saves into the scratch stream and writes it to the store; the body of
    // PerformSave. No caller in this build. Name not in the reference map;
    // the evidence is weak.
    void SaveOptions();  // 0x3B08B0
    // The options' store, named after the system save file when it is
    // idle.
    StorageRef_PS4* GetStore() const;  // 0x3AFC40

    // Field names are not in the reference map.
    MemStream mScratch;
    SystemOptionsStorageClientCallback* mCallback;
    ProfileMgr* mProfileMgr;
};

static_assert(offsetof(SystemOptionsStorageClient, mScratch) == 8);
static_assert(offsetof(SystemOptionsStorageClient, mCallback) == 88);
static_assert(offsetof(SystemOptionsStorageClient, mProfileMgr) == 96);
static_assert(sizeof(SystemOptionsStorageClient) == 104);
