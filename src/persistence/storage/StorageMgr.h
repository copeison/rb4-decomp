#pragma once

#include "utl/containers/List.h"

class StorageRef_PS4;

// Receives the results of the storage operations. Every slot is an empty
// inline default; the binary emits the copies with SaveLoadMgr
// (0x3AEDA0-0x3AEDE0, 0x3AEE20).
class StorageCallback {
public:
    virtual ~StorageCallback() {}

    // Slot 2: StorageMgr::Find's files. The callback does not own them; Find
    // deletes them after the call.
    virtual void OnFindResult(void* context, int result, const eastl::list<StorageRef_PS4*>& refs) {  // 0x3AEDA0
        static_cast<void>(context);
        static_cast<void>(result);
        static_cast<void>(refs);
    }
    // Slot 3: the device the user picked, or -1.
    virtual void OnDevicePromptResult(void* context, int result, int device) {  // 0x3AEE20
        static_cast<void>(context);
        static_cast<void>(result);
        static_cast<void>(device);
    }
    // Slot 4: a read finished; `fileSize` is the file's size when it
    // succeeded.
    virtual void OnReadResult(void* context, int result, unsigned char* data, unsigned long size, unsigned long fileSize) {  // 0x3AEDB0
        static_cast<void>(context);
        static_cast<void>(result);
        static_cast<void>(data);
        static_cast<void>(size);
        static_cast<void>(fileSize);
    }
    // Slot 5.
    virtual void OnWriteResult(void* context, int result, unsigned char* data, unsigned long size) {  // 0x3AEDC0
        static_cast<void>(context);
        static_cast<void>(result);
        static_cast<void>(data);
        static_cast<void>(size);
    }
    // Slot 6.
    virtual void OnDeleteResult(void* context, int result) {  // 0x3AEDD0
        static_cast<void>(context);
        static_cast<void>(result);
    }
    // Slot 7: newer than the map, with no override or caller in this
    // build. Name not in the reference map; the evidence is weak: it is the
    // one request without a result slot, StorageMgr::DeleteContainer.
    virtual void OnDeleteContainerResult(void* context, int result) {  // 0x3AEDE0
        static_cast<void>(context);
        static_cast<void>(result);
    }
};

// The storage devices (persistence/StorageMgr.o): finds save files and
// asks the user for a device. StorageMgr_PS4 is the one implementation.
class StorageMgr {
public:
    virtual ~StorageMgr() {}

    // Slot 2: lists the user's files with the name and reports them to the
    // callback.
    virtual void Find(int user, StorageCallback* callback, int device, bool prompt, const char* name, void* context) = 0;
    // Slot 3: asks the user for a device with room for `size` bytes.
    virtual void PromptForDevice(int user, unsigned long size, StorageCallback* callback, void* context) = 0;
    // Slot 4: empty in this build; the map's body used a static
    // sDisabled. The binary emits it at 0x3B1090.
    virtual void DeleteContainer(int user, const char* name, int device) {
        static_cast<void>(user);
        static_cast<void>(name);
        static_cast<void>(device);
    }

    // Creates theStorageMgr.
    static void Init();  // 0x11B2C80
};

// The storage manager. At 0x1A6E2B0.
extern StorageMgr* theStorageMgr;
