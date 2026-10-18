// ui/UIInputCom.o (its property registration at 0x89F430 and its static
// initializer at 0x8A4820). Only the class symbols and the transition
// functions the layout uses are reconstructed.
#include "ui/input/UIInputCom.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "stategraph/drivers/StateGraphDriverCom.h"

namespace {

// The null object id of a shared header, which the static initializer sets
// to -1 (0x1AC8748); Exit activates the state graph for no object. Name not
// in the reference map.
GameObjectId gNoObject = {0xFFFFFFFFU};

// The state graph driver on the root of the component's entity, or null.
// Inlined into the transition functions. Name not in the reference map.
StateGraphDriverCom* RootStateGraphDriver(const Component* com) {
    return com->mObject->mEntity->GetRoot()->GetCom<StateGraphDriverCom>();
}

}  // namespace

// The object's statics, in the order of its static initializer.
Symbol UIInputCom::sId("UIInputCom");
Symbol UIInputCom::sClassName("UIInputCom");

// Reconstructed from eboot.elf at 0x89ED90.
void UIInputCom::Exit() {
    StateGraphDriverCom* const driver = RootStateGraphDriver(this);
    if (driver != nullptr && mExitAnimation.mPath != Symbol()) {
        driver->Activate(mExitAnimation, gNoObject);
    }
}

// Reconstructed from eboot.elf at 0x89EE10.
bool UIInputCom::IsEntering() const {
    const StateGraphDriverCom* const driver = RootStateGraphDriver(this);
    if (driver == nullptr || mEnterAnimation.mPath == Symbol()) {
        return false;
    }
    return driver->IsActive(mEnterAnimation);
}

// Reconstructed from eboot.elf at 0x89EE90.
bool UIInputCom::IsExiting() const {
    const StateGraphDriverCom* const driver = RootStateGraphDriver(this);
    if (driver == nullptr || mExitAnimation.mPath == Symbol()) {
        return false;
    }
    return driver->IsActive(mExitAnimation);
}
