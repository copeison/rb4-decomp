// ui/UI.o: UI::Init and UI::Terminate. The registrations Init calls are the
// classes' inline Init functions, which the binary emits in this object
// (0x891A30 to 0x894D1F) with their factories after Terminate.
#include "ui/core/UI.h"

#include "os/debug/Debug.h"
#include "ui/input/UIInputCom.h"
#include "ui/input/UIInputReceiverCom.h"
#include "ui/layout/UILayoutCom.h"
#include "ui/layout/UILayoutResource.h"
#include "ui/lists/UIListCom.h"
#include "ui/lists/UIListDtaProviderCom.h"
#include "ui/lists/UIListEditorDataCom.h"
#include "ui/lists/UIListItemCom.h"
#include "ui/manager/UIMgr.h"
#include "ui/widgets/UIComboBoxEnumCom.h"
#include "ui/widgets/UIComboBoxFileCom.h"
#include "ui/widgets/UIComboBoxSymCom.h"
#include "ui/widgets/UIScrollbarCom.h"
#include "ui/widgets/UISliderCom.h"

// Reconstructed from eboot.elf at 0x891990.
void UI::Init() {
    UIMgr::Init();
    UILayoutResource::Init();
    UIInputCom::Init();
    UIInputReceiverCom::Init();
    UILayoutCom::Init();
    UIListCom::Init();
    UIListDtaProviderCom::Init();
    UIListItemCom::Init();
    UIScrollbarCom::Init();
    UISliderCom::Init();
    UIComboBoxEnumCom::Init();
    UIComboBoxFileCom::Init();
    UIComboBoxSymCom::Init();
    UIListEditorDataCom::Init();
    TheDebug.AddExitCallback(Terminate);
}

// Reconstructed from eboot.elf at 0x894D20.
void UI::Terminate() {
    if (theUI == nullptr) {
        return;
    }
    theUI->Terminate();
    delete theUI;
    theUI = nullptr;
}
