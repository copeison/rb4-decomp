#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// The component that takes the UI manager's input for a scene and moves
// the focus between its UIInputReceiverComs ("Com that handles all input
// from the UI Manager"; ui/UIInputCom.o). Its enter and exit transitions
// play through the scene's StateGraphDriverCom. Only the members the
// layout uses are declared and reconstructed. The object is 408 bytes.
class UIInputCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x891BA0. Not
    // reconstructed: the registration helpers it inlines are not modelled.
    static void Init();

    // Starts the exit animation on the scene root's state graph driver.
    void Exit();  // 0x89ED90
    // Whether the driver still runs the enter or the exit animation.
    bool IsEntering() const;  // 0x89EE10
    bool IsExiting() const;   // 0x89EE90

    static Symbol sId;         // 0x1AC8758, "UIInputCom"
    // The class symbol GameObject::CreateComponent takes; also
    // "UIInputCom". Name not in the reference map.
    static Symbol sClassName;  // 0x1AC8760

    unsigned char mOpaque[368];  // Not modelled.
    // "enter_animation" and "exit_animation": the state graphs run when the
    // scene enters and exits. Field names are not in the reference map.
    ResourcePath mEnterAnimation;
    ResourcePath mExitAnimation;
};

static_assert(offsetof(UIInputCom, mEnterAnimation) == 392);
static_assert(offsetof(UIInputCom, mExitAnimation) == 400);
static_assert(sizeof(UIInputCom) == 408);
