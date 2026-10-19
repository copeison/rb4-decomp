#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The audio clip component (class id "AudioClip"; its registration is at
// 0x25D0, the vtable at 0x18DDAB8, the constructor at 0x1C590). A clip
// plays its sound when triggered, possibly waiting for a musical start
// ("default_start_config"), and reports its state ("standard_state").
// Newer than the reference map. Only what the state graph's
// StateNodeTriggerAudioClip uses is declared; the class is not
// reconstructed and its 336-byte layout is not modelled, so the three
// state flags are read at their offsets. Declared for the state graph
// nodes. SoundManager.cpp declares a local AudioClipCom with only Init.
class AudioClipCom : public Component {
public:
    // Registers the class. Called by SoundManager::_InitComponents.
    static void Init();  // 0x25D0

    // Slot 41 at 0xC6A0 in the base: the clip's text for the editor,
    // "<unknown>" unless a subclass names its sound. Name not in the
    // reference map; the evidence is weak.
    virtual const char* GetDescription() const;

    // Starts the clip, ending a paused play first. Name not in the
    // reference map; the evidence is weak.
    bool Trigger();  // 0x1CA20
    // Stops the clip and clears its play state. Name not in the reference
    // map; the evidence is weak.
    bool Stop();  // 0x1CA80

    // The first byte of "default_start_config": the "start_position" mode;
    // nonzero waits for a musical time to start. Name not in the reference
    // map.
    bool WaitsForStart() const {
        return reinterpret_cast<const unsigned char*>(this)[24] != 0;
    }
    // Set by Trigger and cleared by Stop; the clip is triggered or playing.
    // Name not in the reference map.
    bool IsActive() const {
        return reinterpret_cast<const unsigned char*>(this)[304] != 0;
    }
    // Set once a clip that waits for its start has started. Name not in the
    // reference map.
    bool HasStarted() const {
        return reinterpret_cast<const unsigned char*>(this)[320] != 0;
    }

    // "AudioClip"; the base class symbol GameObject::GetBaseCom matches.
    static Symbol sClassName;  // 0x19C65D8
};
