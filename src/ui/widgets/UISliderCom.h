#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "os/joypads/Joypad.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class DataArray;
class UIButtonDownMsg;
class UIKeyboardKeyMsg;

// A generic slider, knob or dial with a number of steps ("Generic slider
// component - use to build a slider, knob, dial, etc."; ui/UISliderCom.o,
// 0x8CDAF0-0x8CEF9F with the static initializer at 0x8CEEE0). The pad and
// the keyboard move it a step at a time in its direction, and it exports
// "slider_step_changed". The vtable at 0x195E6F0 has 41 slots. The object
// is 48 bytes.
class UISliderCom : public Component {
public:
    // The axis the buttons move the slider along. The enumerator names are
    // the property's allowed values; the type name is not in the reference
    // map.
    enum SliderDirection : int {
        kSliderVertical = 0,
        kSliderHorizontal = 1,
    };

    UISliderCom();  // 0x8CDAF0
    // The imprint's copy starts at the first step. Inlined into _Imprint.
    // Not in the reference map.
    UISliderCom(const UISliderCom& other)
        : Component(other),
          mCurrentStep(0),
          mCurrentPct(0.0F),
          mNumSteps(other.mNumSteps),
          mDirection(other.mDirection),
          mLocToken(other.mLocToken) {}
    ~UISliderCom() override;  // slots 0-1: 0x8CE520, 0x8CEAF0

    // Slot 2: "ui_button_down" and "ui_key" move the slider; other
    // messages are unhandled. Not reconstructed: the UI messages
    // (ui/UIMsg.o) are not modelled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x8CE8E0
    Symbol GetId() const override;         // slot 4: 0x8CEB10
    Symbol GetClassName() const override;  // slot 5: 0x8CEB20
    int CurrentRev() const override;       // slot 7: 0x8CEB30
    bool IsA(Symbol type) const override;  // slot 8: 0x8CEB50
    Component* AsComponent() override;     // slot 9: 0x8CEB80
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8CEB90
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8CECC0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8CECD0
    // Slots 33 and 38: keep the percentage in step.
    void _Poll() override;      // 0x8CE5C0
    void _EditPoll() override;  // 0x8CE590

    void SetCurrentStep(unsigned int step);  // 0x8CE530
    // The current step as a fraction of the last; 0 for one step.
    void _UpdateCurrentPercent();  // 0x8CE560
    // Unhandled without steps; otherwise the button's action moves the
    // slider. Not reconstructed.
    DataNode OnMsg(const UIButtonDownMsg& msg);  // 0x8CE5F0
    // Moves the slider a step for an action along its direction, exporting
    // "slider_step_changed" when the step changes. Not reconstructed: the
    // exported message is not modelled.
    DataNode _HandleUIAction(const JoypadAction& action);  // 0x8CE670
    // The arrow keys move the slider. Not reconstructed.
    DataNode OnMsg(const UIKeyboardKeyMsg& msg);  // 0x8CE820

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x893660
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x895230) is InitComponentClass's.
    static Component* _Create();  // 0x895200
    // Describes the class, exports "slider_step_changed" and registers its
    // properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8CDB40

    static Symbol sId;                  // 0x1AC9E10, "UISliderCom"
    // Also "UISliderCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC9E18
    static PropRegistry sPropRegistry;  // 0x1AC9E20
    static ComMetaData sMetaData;       // 0x1AC9EC0

    // Field names are not in the reference map.
    // "current_step", which "init_current_step" also sets.
    unsigned int mCurrentStep;
    // "current_pct": the step from 0 to 1.
    float mCurrentPct;
    unsigned int mNumSteps;          // "num_steps"
    SliderDirection mDirection;      // "direction"
    Symbol mLocToken;                // "loc_token"
};

static_assert(offsetof(UISliderCom, mCurrentStep) == 24);
static_assert(offsetof(UISliderCom, mCurrentPct) == 28);
static_assert(offsetof(UISliderCom, mNumSteps) == 32);
static_assert(offsetof(UISliderCom, mDirection) == 36);
static_assert(offsetof(UISliderCom, mLocToken) == 40);
static_assert(sizeof(UISliderCom) == 48);
