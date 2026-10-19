#pragma once

#include <cstddef>

#include "persistence/storage/StorageMgr.h"
#include "utl/containers/Vector.h"

// The PS4 storage manager (persistence/StorageMgr_PS4.o, 0x3B0AA0-0x3B1137).
// It initializes the user service and the save data library and hands out
// the 16 save data mount points under a file-static lock. The vtable at
// 0x18FCFE0 has StorageMgr's five slots; the object is 40 bytes.
class StorageMgr_PS4 : public StorageMgr {
public:
    // The save data mount points. Name not in the reference map.
    static constexpr int kNumMountPoints = 16;

    StorageMgr_PS4();  // 0x3B0AA0
    // Slots 0-1: 0x3B0C20, 0x3B0C50.
    ~StorageMgr_PS4() override;

    // Slot 2: reports the user's file with the name, when its save
    // directory can be created and it is not empty, then deletes the
    // references. The device and the flag are not used; the result is
    // kResultAccessDenied when the directory cannot be created.
    void Find(int user, StorageCallback* callback, int device, bool prompt, const char* name, void* context) override;  // 0x3B0C90
    // Slot 3: empty; the PS4 has one device and the callback is never
    // called.
    void PromptForDevice(int user, unsigned long size, StorageCallback* callback, void* context) override;  // 0x3B0E80

    // Lists nothing and returns zero.
    int FindInDirectory(int user, const char* directory, eastl::list<StorageRef_PS4*>& refs);  // 0x3B0E90
    // Takes the first free mount point, or -1. The map's spelling.
    int AquireMountPoint();  // 0x3B0EA0
    // Frees the mount point.
    void ReleaseMountPoint(int index);  // 0x3B1040

    // Whether each mount point is taken. Name not in the reference map.
    eastl::vector<bool> mMountPointsUsed;
};

static_assert(offsetof(StorageMgr_PS4, mMountPointsUsed) == 8);
static_assert(sizeof(StorageMgr_PS4) == 40);
