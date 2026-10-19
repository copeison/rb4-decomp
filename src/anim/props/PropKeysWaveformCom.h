#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "utl/text/Symbol.h"

// The keys of a waveform property in a property animation
// (anim/PropKeysWaveformCom.o, 0x325320-0x326BDF): one class per waveform
// type, registered as "PropKeysWaveformCom_kWaveformFloat" and
// "PropKeysWaveformCom_kWaveformColor". The keys interpolate linearly by
// default; the driver plays the last playback's keys, blending a key's
// waveform into the next one's through the waveform's second resource with
// the eased position. Each instance's StaticKeysType, StaticKeysSubType,
// StaticCreatePropDriver, Register, CreatePropDriver and StaticKeyTypeName
// lie at 0x325320, +0x10, +0xB0, +0x180, +0x260 and +0x330 for float and
// 0x340 further on for Hmx::Color. The other methods are emitted with the
// instances in anim/AnimInit.o; the addresses given are float's, whose
// vtable is 0x18F4120 (76 slots; 0x18F45B0 for Hmx::Color). The objects
// are 320 bytes.
template <class T>
class PropKeysWaveformCom : public PropKeysTypeCom<Waveform<T>> {
public:
    PropKeysWaveformCom() {
        this->mInterpolation = KeyTrackBase::kInterpolationLinear;
    }
    // Slots 0-1: 0x2CD4E0, 0x2CDE90.
    ~PropKeysWaveformCom() override {}

    Symbol GetId() const override;          // slot 4: 0x2CDF20
    Symbol GetClassName() const override;   // slot 5: 0x2CDF30
    int CurrentRev() const override;        // slot 7: 0x2CDF40
    bool IsA(Symbol type) const override;   // slot 8: 0x2CDF60
    Component* AsComponent() override;      // slot 9: 0x2CDF90
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2CDFA0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2CE1D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2CE1E0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x325580
    PropertyType GetKeysType() const override;      // slot 55: 0x2CE570
    Symbol GetKeysSubType() const override;         // slot 56: 0x2CE580
    // Slot 57: anything but Spline. The map has ValidInterpolations().
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x2CE590
    // Slot 58: true for the colour waveforms.
    bool IsColorKeys() const override;  // 0x2CE5A0
    // Slots 73-75: a waveform node, the node's waveform, the default
    // waveform (a copy of a local static made from the type's default
    // resource).
    DataNode _ValueToDataNode(const Waveform<T>& value, bool forEditor) const override;  // 0x2CF040
    Waveform<T> _ValueFromDataNode(const DataNode& value) const override;                 // 0x2CF060
    Waveform<T> _DefaultValue() const override;                                           // 0x2CF080

    // The waveform type.
    static PropertyType StaticKeysType();  // 0x325320
    // The empty symbol, created on first use.
    static const Symbol& StaticKeysSubType();  // 0x325330
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x3253D0
    // Registers the class as the keys of the type's properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x3254A0
    // "TKey<kWaveformFloat>" or "TKey<kWaveformColor>"; no caller was found
    // (the registration names both structs "TKey<Waveform<C>>"). Name not
    // in the reference map.
    static const char* StaticKeyTypeName();  // 0x325650

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AD3B0 (float) and 0x2AD860 (Hmx::Color), which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2CD370 for float.
    static Component* _Create();
    // The registry builder. Emitted in anim/AnimInit.o at 0x2CCD80 for
    // float.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);

    // "PropKeysWaveformCom_<type>", formatted by the guarded template
    // initializers. Name not in the reference map but sId's.
    static Symbol sId;          // 0x19F78E8 for float
    static Symbol sClassName;   // 0x19F7648
    static PropRegistry sPropRegistry;  // 0x19F7830
    static ComMetaData sMetaData;       // 0x19F7658

private:
    // Formats the class id. Inlined into the initializers. Name not in the
    // reference map.
    static const char* _MakeClassName();
};

// The waveform keys' driver plays the last playback: its keys' waveform at
// the frame, blending into the next key's (0x3259A0 and 0x325AD0 for
// float), set when the playback is not paused and the keys are not empty.
template <>
void TPropDriver<PropKeysWaveformCom<float>>::_Poll(GameObject* root);  // 0x3259A0
template <>
void TPropDriver<PropKeysWaveformCom<Hmx::Color>>::_Poll(GameObject* root);

static_assert(sizeof(PropKeysWaveformCom<float>) == 320);
