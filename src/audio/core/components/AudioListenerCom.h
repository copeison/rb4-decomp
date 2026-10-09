#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "os/threading/CritSec.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class Entity;

// The component that makes its object the listener position
// (audio/AudioListenerCom.o, 0x392E0 to 0x39F5F). Its class id is
// "AudioListener". One listener is active at a time: the first to enter or
// poll, or the one whose scene entity reports it active. The active
// listener moves the sound manager's listener to its object every poll. The
// vtable at 0x18DF990 has 41 slots. The object is 88 bytes.
class AudioListenerCom : public Component {
public:
    // Inlined into the class's Init (0x4B70) and _Create (0xD280).
    AudioListenerCom()
        : mHardwareMapId(-1),
          mHardwareMapped(false),
          mHardwareMapping(nullptr),
          mHardwareMappingIndex(0) {
        mEntityActiveSink.mSink = nullptr;
        mEntityActiveSink.mSource = nullptr;
    }
    ~AudioListenerCom() override;  // slots 0-1: 0x392E0, 0x39380

    // Slot 2: handles "on_entity_active_changed" only; any other message is
    // unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x39B10
    Symbol GetId() const override;             // slot 4: 0x39C30
    Symbol GetClassName() const override;      // slot 5: 0x39C40
    int CurrentRev() const override;           // slot 7: 0x39C50
    bool IsA(Symbol type) const override;      // slot 8: 0x39C70
    Component* AsComponent() override;         // slot 9: 0x39CA0
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Not
    // reconstructed: the thread's imprint state is not modelled.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x39CB0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x39E10
    ComMetaData& _GetMetaData() override;       // slot 23: 0x39E20
    // Slot 31 at 0x39750: becomes the active listener when there is none,
    // and subscribes to its scene entity's "entity_active_changed" events.
    void _Enter() override;
    // Slot 33 at 0x399B0: the active listener moves the sound manager's
    // listener to its object.
    void _Poll() override;
    // Slots 36 and 38 at 0x39400 and 0x39410 run the game-mode slots.
    void _EditEnter() override;
    void _EditPoll() override;

    // Makes this the active listener. At 0x39AB0; no caller in this build.
    void MakeActive();
    // Becomes the active listener only when there is none. At 0x39890; no
    // caller in this build. Name not in the reference map.
    void MakeActiveIfNone();
    // Stops being the active listener. At 0x39360; no caller in this build.
    // Name not in the reference map.
    void MakeInactive();
    // Stores the pad's hardware map id. SoundManager::
    // SetJoypadEmitterPlatformId calls it. At 0x39AC0.
    void SetHardwareMapId(int id);
    // The "on_entity_active_changed" handler: an active scene makes the
    // listener active unless another is, and an inactive one drops it. At
    // 0x39AD0, inlined into Handle. Name not in the reference map.
    DataNode _OnEntityActiveChanged(bool active);

    // Registers the class. Inline in the map; this build emits it in
    // audio/SoundManager.o at 0x4B70. Not reconstructed: the registration
    // helpers it inlines are not modelled.
    static void Init();
    // The class factory. Emitted in audio/SoundManager.o at 0xD280.
    static Component* _Create();
    // Registers the class description; the class has no properties. Not
    // reconstructed: ComMetaData's string and list members are written
    // inline through helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x39420

    static Symbol sId;          // 0x19C7C80, "AudioListener"
    // The class symbol GameObject::CreateComponent takes; also
    // "AudioListener". Name not in the reference map.
    static Symbol sClassName;   // 0x19C7C88
    static PropRegistry sPropRegistry;  // 0x19C7C90
    static ComMetaData sMetaData;       // 0x19C7D30
    // Guards the active listener's poll. Name not in the reference map.
    static CritSec sActiveListenerLock;  // 0x19C7ED8
    // The active listener, or null. Name not in the reference map.
    static AudioListenerCom* sActiveListener;  // 0x19C7EE8

    // Field names are not in the reference map.
    // The pad's hardware map id, -1 for none.
    int mHardwareMapId;
    // SoundManager::HasJoypadEmitter reads it; no writer was found in this
    // build.
    bool mHardwareMapped;
    // Cleared by the constructor; no reader was found. The names follow the
    // map's _ClearHardwareMapping and are weak.
    void* mHardwareMapping;
    int mHardwareMappingIndex;
    // The subscription to the scene entity's "entity_active_changed" events.
    MsgSource::EventSinkElem mEntityActiveSink;
};

static_assert(offsetof(AudioListenerCom, mHardwareMapId) == 0x18);
static_assert(offsetof(AudioListenerCom, mHardwareMapped) == 0x1C);
static_assert(offsetof(AudioListenerCom, mHardwareMapping) == 0x20);
static_assert(offsetof(AudioListenerCom, mHardwareMappingIndex) == 0x28);
static_assert(offsetof(AudioListenerCom, mEntityActiveSink) == 0x30);
static_assert(sizeof(AudioListenerCom) == 0x58);

// The nearest entity, from the entity itself up through the objects that
// instance it, whose resource is a "RndSceneResource"; null when there is
// none. Emitted in this object and called by game code. Name not in the
// reference map.
Entity* FindSceneEntity(Entity* entity);  // 0x398B0
