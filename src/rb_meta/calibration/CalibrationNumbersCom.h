#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class DataArray;
class UIComboBoxEnumCom;
class UISelectMsg;

// The screen that enters the calibration numbers by hand ("Sets up the
// combo boxes for entering calibration numbers manually."; author Don
// Wurster; rb_meta/CalibrationNumbersCom.o, 0xDB0280-0xDB1A2F with the
// static initializer at 0xDB1930). Its class symbol is "CalibrationNumbers".
// Entering fills two enum combo boxes, each in an instanced scene, with the
// offsets from -300 to 300 ms in steps of 5 and selects the system options'
// current offsets; selecting "confirm_button" stores the chosen offsets,
// marks the console calibrated, logs it for every participating profile and
// goes back to the automatic calibration. The vtable at 0x1981370 has 41
// slots. The object is 48 bytes.
class CalibrationNumbersCom : public Component {
public:
    // The combo boxes found on entering; the imprint's copy starts without
    // them. The map names the type (its constructor at 0x44F6C0 in the
    // map's build); the field names are not in the reference map.
    struct RuntimeData {
        RuntimeData() : mAudioComboBox(nullptr), mVideoComboBox(nullptr) {}

        UIComboBoxEnumCom* mAudioComboBox;
        UIComboBoxEnumCom* mVideoComboBox;
    };

    CalibrationNumbersCom();  // 0xDB0280
    // Copies the properties for an imprint and starts afresh. Inlined into
    // _Imprint (0xDB17E0). Not in the reference map.
    CalibrationNumbersCom(const CalibrationNumbersCom& other)
        : Component(other), mAudio(other.mAudio), mVideo(other.mVideo), mRuntime() {}
    // Slots 0-1: 0xDB0800, 0xDB1740.
    ~CalibrationNumbersCom() override;

    // Slot 2: "ui_select" goes to OnMsg; other messages are unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xDB1620
    Symbol GetId() const override;          // slot 4: 0xDB1760
    Symbol GetClassName() const override;   // slot 5: 0xDB1770
    int CurrentRev() const override;        // slot 7: 0xDB1780
    bool IsA(Symbol type) const override;   // slot 8: 0xDB17A0
    Component* AsComponent() override;      // slot 9: 0xDB17D0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xDB17E0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xDB1910
    ComMetaData& _GetMetaData() override;       // slot 23: 0xDB1920
    // Slot 29: true. The map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0xDB0810
    // Slot 31: gives the scene's RBUIInputCom the input, finds and fills
    // the combo boxes and selects the current offsets. The map's
    // _Enter(ObjPtr const&).
    void _Enter() override;  // 0xDB0850

    // On "confirm_button", stores the selected offsets (the negated
    // entries), marks the console calibrated, logs the calibration for the
    // participating profiles and goes to kLayoutCalibrationAuto. Handled
    // whatever was selected. Inlined into Handle; the binary keeps an
    // uncalled copy.
    DataNode OnMsg(const UISelectMsg& msg);  // 0xDB1410

    // The offset in milliseconds of a combo box entry.
    static float _CalibrationForValue(int value);  // 0xDB0820
    // The combo box entry of an offset: the whole milliseconds, rounded
    // toward zero to a multiple of 5.
    static int _ValueForCalibration(float calibration);  // 0xDB0830

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB638A0.
    static void Init();
    // The class factory, emitted at 0xBA0AE0; the placement factory beside
    // it (0xBA0B10) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xDB02C0

    // "CalibrationNumbers".
    static Symbol sId;  // 0x1B25980
    // Also "CalibrationNumbers". Name not in the reference map.
    static Symbol sClassName;           // 0x1B25988
    static PropRegistry sPropRegistry;  // 0x1B25990
    static ComMetaData sMetaData;       // 0x1B25A30

    // Field names are not in the reference map.
    // "audio": the instanced object whose scene holds the audio combo box.
    GameObjectId mAudio;
    // "video": the instanced object whose scene holds the video combo box.
    GameObjectId mVideo;
    RuntimeData mRuntime;
};

static_assert(offsetof(CalibrationNumbersCom, mAudio) == 24);
static_assert(offsetof(CalibrationNumbersCom, mVideo) == 28);
static_assert(offsetof(CalibrationNumbersCom, mRuntime) == 32);
static_assert(sizeof(CalibrationNumbersCom) == 48);
