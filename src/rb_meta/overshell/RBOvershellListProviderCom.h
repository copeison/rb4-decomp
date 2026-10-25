#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "rb_meta/overshell/RBOvershellDefines.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class RBOvershellSlotCom;
class UIListCom;

// The list provider of an overshell slot (rb_meta/RBOvershellListProviderCom.o,
// static initializer 0xC031E0; constructor 0xBFB100, vtable 0x1972068):
// fills the slot's UIListCom with the rows of the slot's view. Declared
// only: just the class id and the fields RBOvershellSlotCom writes are
// modelled. The object is about 2.5 KB.
class RBOvershellListProviderCom : public Component {
public:
    // "RBOvershellListProviderCom".
    static Symbol sId;  // 0x1B034C8

    // Shows the confirmation's actions as the list's rows and focuses
    // `focus`. Name not in the reference map.
    void SetConfirmActions(const eastl::vector<Symbol>& actions, Symbol focus);  // 0xBFE890
    // Empties the list's rows. Name not in the reference map.
    void ClearListData();  // 0xBFC750

    // Field names are not in the reference map. The start of the run-time
    // state: the list the provider fills (the constructor hands it to
    // StaticFocus::SetData; the evidence for the name is weak) and a field
    // that is not identified.
    UIListCom* mList;
    void* mRuntimeOpaque;  // Not modelled.
    // Set by RBOvershellSlotCom::_Enter to the slot that owns the provider.
    RBOvershellSlotCom* mSlotCom;
    // Copied from the slot by RBOvershellSlotCom::SetActiveStatus.
    OvershellActiveStatus mActiveStatus;
};

static_assert(offsetof(RBOvershellListProviderCom, mSlotCom) == 40);
static_assert(offsetof(RBOvershellListProviderCom, mActiveStatus) == 48);
