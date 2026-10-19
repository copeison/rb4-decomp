#pragma once

#include "utl/messages/MsgSink.h"

// The downloadable content manager (os/ContentMgr.o, os/ContentMgr_PS4.o).
// Only the slots SystemInit, SystemPoll and SystemTerminate call are
// declared; the class is not reconstructed. The PS4 vtable is at 0x18FB618.
class ContentMgr : public MsgSink {
public:
    ~ContentMgr() override;  // slots 0-1
    DataNode Handle(DataArray* msg, bool warn) override;  // slot 2
    virtual void Init();       // slot 4: 0x36C370
    virtual void Poll();       // slot 5: 0x36C870
    virtual void Terminate();  // slot 6: 0x36C810

    // Whether the cheat that fails every content mount is on. The binary
    // inlines it as a read of the flag at +0x30 (StorageRef_PS4's mount at
    // 0x3B1CF7); the flag is not modelled.
    bool AreAllMountsGoingToFailDueToCheat();
};

// The PS4 content manager, a ContentMgr_PS4 at 0x19FE7A0.
extern ContentMgr TheContentMgr;
