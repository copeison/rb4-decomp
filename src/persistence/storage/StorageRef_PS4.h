#pragma once

#include <cstddef>
#include <functional>

#include "persistence/storage/StorageRef.h"
#include "utl/text/Str.h"
#include "utl/threading/ThreadCall.h"

class StorageCallback;
struct SceSaveDataMountPoint;

// A save file inside the title's save data directory "slot017"
// (persistence/StorageRef_PS4.o, 0x3B1140-0x3B28E0). Each operation mounts
// the directory at one of StorageMgr_PS4's mount points, works on
// "savedataN:/<name>" through the engine's file functions and unmounts it
// again; reads and writes run on the ThreadCall thread and report through
// ThreadDone. The vtable at 0x18FD018 has 17 slots, with ThreadCallback's
// at 0x18FD0B0 (offset 8); the object is 144 bytes.
class StorageRef_PS4 : public StorageRef, public ThreadCallback {
public:
    // What the reference is doing; it also selects the mount mode. Names
    // not in the reference map.
    enum State : int {
        kStateIdle = 0,
        kStateReading = 1,
        kStateWriting = 2,
    };

    StorageRef_PS4();  // 0x3B1140
    // Copies the owner, the cached size and the names; the other state
    // starts idle.
    StorageRef_PS4(const StorageRef_PS4& other);  // 0x3B11D0
    // Slots 0-1: 0x3B2060, 0x3B20D0; ThreadCallback thunks 0x3B2170,
    // 0x3B21E0. Inline in the map's build.
    ~StorageRef_PS4() override {}
    // Copies the owner, the cached size, the mount flag and the name; the
    // title is cleared.
    StorageRef_PS4& operator=(const StorageRef_PS4& other);  // 0x3B1310

    // Slot 2.
    const char* Name() override;  // 0x3B1350
    // Slot 3.
    void SetName(const char* name, const char* displayName) override;  // 0x3B1360
    // Slot 4: the file's size, measured once and cached.
    unsigned long Size() override;  // 0x3B1430
    // Slot 5: always kAnyDevice; the PS4 has one device. Inline in the
    // map's build.
    int Device() override {  // 0x3B2130
        return -1;
    }
    // Slot 6: ignored. Inline in the map's build.
    void SetDevice(int device) override {  // 0x3B2140
        static_cast<void>(device);
    }
    // Slots 7-8. Inline in the map's build.
    int Owner() override {  // 0x3B2150
        return mOwner;
    }
    void SetOwner(int owner) override {  // 0x3B2160
        mOwner = owner;
    }
    // Slot 9: queues the write on the ThreadCall thread.
    void Write(unsigned char* data, unsigned long size, StorageCallback* callback, void* context) override;  // 0x3B1560
    // Slot 10: queues the read.
    void Read(unsigned char* data, unsigned long size, StorageCallback* callback, void* context) override;  // 0x3B1540
    // Slot 11: deletes the file at once. Neither argument is used.
    void Delete(StorageCallback* callback, void* context) override;  // 0x3B1580
    // Slot 12.
    bool Exists() override;  // 0x3B16A0
    // Slot 13.
    bool IsValid() const override;  // 0x3B1770
    // Slot 14.
    bool IsBusy(bool wait) const override;  // 0x3B1790
    // Slot 15 (ThreadCallback slot 2, thunk 0x3B1960): runs the queued
    // read or write on the call thread.
    int ThreadStart() override;  // 0x3B17A0
    // Slot 16 (ThreadCallback slot 3, thunk 0x3B1A00): reports the result
    // to the callback on the polling thread.
    void ThreadDone(int result) override;  // 0x3B1990

    // Creates the save directory, titled with the display name, when it
    // does not exist yet. A full disk or broken data shows the system
    // dialog and fails.
    bool InitMount();  // 0x3B1A70
    // Mounts the directory at a free mount point read-only (1) or for
    // writing (2), and builds the mount point and the file's path.
    bool _MountSaveDir(unsigned int mountMode);  // 0x3B1CC0
    // Unmounts the directory and frees the mount point.
    void _UnmountSaveDir();  // 0x3B1EF0
    // Sets the directory's title to the display name. Inlined into
    // InitMount.
    void _SetSaveDataParams(SceSaveDataMountPoint* mountPoint);  // 0x3B1C30
    // Measures the file with the directory mounted.
    unsigned long _DetermineSize();  // 0x3B1460
    // The call thread's halves of Read and Write. kResultSuccess or
    // kResultGeneralError.
    int _ThreadRead();   // 0x3B17C0
    int _ThreadWrite();  // 0x3B1890
    // Runs the operation with the directory mounted for the current state.
    // Inlined into its users. Name not in the reference map.
    void _WithMountedSaveDir(const std::function<void()>& operation);  // 0x3B1640

    // Field names are not in the reference map.
    State mState;
    int mOwner;
    String mName;
    // The directory's title.
    String mDisplayName;
    // "savedataN:/<name>" while mounted.
    String mPath;
    // The cached file size; zero until measured.
    unsigned long mSize;
    // Skips InitMount. Only operator= copies it; nothing in this build
    // sets it. The evidence for the name is weak.
    bool mMountInitialized;
    // The pending read or write.
    void* mContext;
    StorageCallback* mCallback;
    unsigned char* mBuffer;
    unsigned long mBufferSize;
    // StorageMgr_PS4's mount point, or -1.
    int mMountIndex;
    // "savedataN:" while mounted.
    String mMountPoint;
};

static_assert(offsetof(StorageRef_PS4, mState) == 16);
static_assert(offsetof(StorageRef_PS4, mOwner) == 20);
static_assert(offsetof(StorageRef_PS4, mName) == 24);
static_assert(offsetof(StorageRef_PS4, mPath) == 56);
static_assert(offsetof(StorageRef_PS4, mSize) == 72);
static_assert(offsetof(StorageRef_PS4, mMountInitialized) == 80);
static_assert(offsetof(StorageRef_PS4, mContext) == 88);
static_assert(offsetof(StorageRef_PS4, mBufferSize) == 112);
static_assert(offsetof(StorageRef_PS4, mMountIndex) == 120);
static_assert(offsetof(StorageRef_PS4, mMountPoint) == 128);
static_assert(sizeof(StorageRef_PS4) == 144);
