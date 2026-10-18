#pragma once

#include "entity/core/Component.h"

// The component that throws a dialog up on top of the current screen
// (rb_meta/DialogMgrCom.o). Dialogs requested while one shows or the UI is
// in a transition wait in a queue under a lock. Only the queue's poll is
// declared; the class is not reconstructed.
class DialogMgrCom : public Component {
public:
    // Shows the first queued dialog once no dialog is showing or pending
    // and the UI is not in a transition (UIMgr::IsInTransition), and drops
    // it from the queue. The map's DialogMgrCom::Poll(); called once per
    // frame by App::RunOneFrame.
    static void Poll();  // 0xADE120
};
