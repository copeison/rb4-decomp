#pragma once

class StorageCallback;

// A file on a storage device: a save container's data file on the PS4
// (StorageRef_PS4). The interface has no members of its own; the platform
// class implements every slot. ResultToString and the two constants are in
// persistence/StorageMgr.o and StorageRef_PS4.o.
class StorageRef {
public:
    // The results the operations report to their StorageCallback. Names
    // from ResultToString's table.
    enum Result {
        kResultSuccess = 0,
        kResultGeneralError = -1,
        kResultAccessDenied = -2,
        kResultInvalidDevice = -3,
        kResultDeviceFull = -4,
    };

    virtual ~StorageRef() {}

    // Slot 2: the file's name.
    virtual const char* Name() = 0;
    // Slot 3: names the file and the title the system shows for it.
    virtual void SetName(const char* name, const char* displayName) = 0;
    // Slot 4: the file's size in bytes.
    virtual unsigned long Size() = 0;
    // Slot 5.
    virtual int Device() = 0;
    // Slot 6.
    virtual void SetDevice(int device) = 0;
    // Slot 7: the user the file belongs to.
    virtual int Owner() = 0;
    // Slot 8.
    virtual void SetOwner(int owner) = 0;
    // Slot 9: writes the bytes in the background, then reports to the
    // callback with the context.
    virtual void Write(unsigned char* data, unsigned long size, StorageCallback* callback, void* context) = 0;
    // Slot 10: reads into the buffer in the background, then reports.
    virtual void Read(unsigned char* data, unsigned long size, StorageCallback* callback, void* context) = 0;
    // Slot 11.
    virtual void Delete(StorageCallback* callback, void* context) = 0;
    // Slot 12.
    virtual bool Exists() = 0;
    // Slot 13: whether the file has an owner and a name.
    virtual bool IsValid() const = 0;
    // Slot 14: whether a read or write is under way.
    virtual bool IsBusy(bool wait) const = 0;

    // The result's enumerator name, or "(unknown error code)".
    static const char* ResultToString(int result);  // 0x11B2CB0

    // The device and user that match any. Both are -1, read from memory by
    // their users.
    static const int kAnyDevice;  // 0x127B378
    static const int kAnyUser;    // 0x127B374
};
