#pragma once

#include <cstddef>

#include "rb_game/core/Defines.h"

// The options a player plays with (the band users' GetGameplayOptions):
// the lefty flags, guitar solo mode, drum fill logic, vocal improv,
// breakneck speed and the in-game, vocal stem and microphone volumes. An
// interface without a destructor: GameplayOptions implements it, and the
// local band user's online-session view forwards it. Newer than the map,
// whose RBGameplayOptions holds these values itself; the class name is not
// in the reference map, and the method names are the map's RBGameplayOptions
// names where the bodies match them, the others inferred. 32 slots.
class IGameplayOptions {
public:
    // Slot 0: restores the default options. The map's Clear.
    virtual void Clear() = 0;
    // Slots 1-2: the lefty flags of the guitar and the drums.
    virtual bool IsLefty(ControllerType type) const = 0;
    virtual void SetLefty(ControllerType type, bool lefty) = 0;
    // Slots 3-5: the guitar solo mode; slot 5 switches mode 0 to 1 and any
    // other mode to 0, and returns the new mode. Name of slot 5 not in the
    // reference map.
    virtual int GetGuitarSoloMode() const = 0;
    virtual void SetGuitarSoloMode(int mode) = 0;
    virtual int ToggleGuitarSoloMode() = 0;
    // Slots 6-7: the drum fill logic.
    virtual int GetDrumFillLogic() const = 0;
    virtual void SetDrumFillLogic(int logic) = 0;
    // Slots 8-9: vocal improvisation.
    virtual bool IsVocalImprovEnabled() const = 0;
    virtual void SetVocalImprovEnabled(bool enabled) = 0;
    // Slots 10-16: the breakneck speed step: its index, whether its
    // multiplier is not 1, the multiplier, the index as a fraction of the
    // last step, and setting, raising and lowering the index. Names of
    // slots 10, 13, 15 and 16 not in the reference map.
    virtual unsigned int GetBreakneckSpeedModifierIndex() const = 0;
    virtual bool IsBreakneckSpeedEnabled() const = 0;
    virtual float GetBreakneckSpeedModifier() const = 0;
    virtual float GetBreakneckSpeedModifierNormalized() const = 0;
    virtual void SetBreakneckSpeedModifierIndex(unsigned int index) = 0;
    virtual bool RaiseBreakneckSpeedModifier() = 0;
    virtual bool LowerBreakneckSpeedModifier() = 0;
    // Slots 17-20: the in-game sound volume step.
    virtual float GetIGSVolumeDecibels() const = 0;
    virtual float GetIGSVolumeNormalized() const = 0;
    virtual bool RaiseIGSVolume() = 0;
    virtual bool LowerIGSVolume() = 0;
    // Slots 21-24: the vocal stem volume step.
    virtual float GetVocalStemVolumeDecibels() const = 0;
    virtual float GetVocalStemVolumeNormalized() const = 0;
    virtual bool RaiseVocalStemVolume() = 0;
    virtual bool LowerVocalStemVolume() = 0;
    // Slots 25-28: a microphone's volume step.
    virtual float GetMicVolumeDecibels(int mic) const = 0;
    virtual float GetMicVolumeNormalized(int mic) const = 0;
    virtual bool RaiseMicVolume(int mic) = 0;
    virtual bool LowerMicVolume(int mic) = 0;
    // Slots 29-31: a later on-by-default option the gameplay player reads,
    // its setter, and a toggle that returns the new value. Names not in the
    // reference map; what the option controls is not recovered, so the
    // evidence is weak.
    virtual bool IsGameplayFlagEnabled() const = 0;
    virtual void SetGameplayFlagEnabled(bool enabled) = 0;
    virtual bool ToggleGameplayFlag() = 0;
};

static_assert(sizeof(IGameplayOptions) == 8);

// The gameplay options' values, with the volume steps read from the
// "sound", "track_graphics" and "breakneck_multipliers" config. In this
// build RBGameplayOptions adds the saving on top of this class (as its
// second base), and the local band users keep one for their profile-less
// state. The class name is not in the reference map. The vtable at
// 0x1968880 has 38 slots; the object is 184 bytes. Declared only: the class
// is not reconstructed (constructor 0xA4C280, destructor 0x92C580).
class GameplayOptions : public IGameplayOptions {
public:
    GameplayOptions();  // 0xA4C280

    // Slot 0: restores the default options. The map's Clear.
    void Clear() override;  // 0xA4D050
    // Slots 1-2: the lefty flags of the guitar and the drums.
    bool IsLefty(ControllerType type) const override;  // 0xA4D090
    void SetLefty(ControllerType type, bool lefty) override;  // 0xA4D0B0
    // Slots 3-5: the guitar solo mode; slot 5 switches mode 0 to 1 and any
    // other mode to 0, and returns the new mode. Name of slot 5 not in the
    // reference map.
    int GetGuitarSoloMode() const override;  // 0xA4D0F0
    void SetGuitarSoloMode(int mode) override;  // 0xA4D100
    int ToggleGuitarSoloMode() override;  // 0xA4D120
    // Slots 6-7: the drum fill logic.
    int GetDrumFillLogic() const override;  // 0xA4D150
    void SetDrumFillLogic(int logic) override;  // 0xA4D160
    // Slots 8-9: vocal improvisation.
    bool IsVocalImprovEnabled() const override;  // 0xA4D180
    void SetVocalImprovEnabled(bool enabled) override;  // 0xA4D190
    // Slots 10-16: the breakneck speed step: its index, whether its
    // multiplier is not 1, the multiplier, the index as a fraction of the
    // last step, and setting, raising and lowering the index. Names of
    // slots 10, 13, 15 and 16 not in the reference map.
    unsigned int GetBreakneckSpeedModifierIndex() const override;  // 0xA4D1B0
    bool IsBreakneckSpeedEnabled() const override;  // 0xA4D1C0
    float GetBreakneckSpeedModifier() const override;  // 0xA4D1E0
    float GetBreakneckSpeedModifierNormalized() const override;  // 0xA4D210
    void SetBreakneckSpeedModifierIndex(unsigned int index) override;  // 0xA4D260
    bool RaiseBreakneckSpeedModifier() override;  // 0xA4D290
    bool LowerBreakneckSpeedModifier() override;  // 0xA4D2D0
    // Slots 17-20: the in-game sound volume step.
    float GetIGSVolumeDecibels() const override;  // 0xA4D2F0
    float GetIGSVolumeNormalized() const override;  // 0xA4D300
    bool RaiseIGSVolume() override;  // 0xA4D340
    bool LowerIGSVolume() override;  // 0xA4D390
    // Slots 21-24: the vocal stem volume step.
    float GetVocalStemVolumeDecibels() const override;  // 0xA4D3D0
    float GetVocalStemVolumeNormalized() const override;  // 0xA4D3E0
    bool RaiseVocalStemVolume() override;  // 0xA4D420
    bool LowerVocalStemVolume() override;  // 0xA4D470
    // Slots 25-28: a microphone's volume step.
    float GetMicVolumeDecibels(int mic) const override;  // 0xA4D4B0
    float GetMicVolumeNormalized(int mic) const override;  // 0xA4D4D0
    bool RaiseMicVolume(int mic) override;  // 0xA4D520
    bool LowerMicVolume(int mic) override;  // 0xA4D580
    // Slots 29-31: a later on-by-default option the gameplay player reads,
    // its setter, and a toggle that returns the new value. Names not in the
    // reference map; what the option controls is not recovered, so the
    // evidence is weak.
    bool IsGameplayFlagEnabled() const override;  // 0xA4D5C0
    void SetGameplayFlagEnabled(bool enabled) override;  // 0xA4D5D0
    bool ToggleGameplayFlag() override;  // 0xA4D5F0
    // Slots 32-33: 0x9A51D0, 0xA4D620.
    virtual ~GameplayOptions();
    // Slots 34-37: change hooks, the first for every option, then for the
    // in-game, vocal stem and microphone volumes. Names not in the
    // reference map.
    virtual void _OnOptionsChanged();  // 0xA4D640
    virtual void _OnIGSVolumeChanged();  // 0xA4D650
    virtual void _OnVocalStemVolumeChanged();  // 0xA4D660
    virtual void _OnMicVolumeChanged(int mic);  // 0xA4D670

    // The option values and the config's volume tables; not modelled.
    unsigned char mOptions[176];
};

static_assert(sizeof(GameplayOptions) == 184);
