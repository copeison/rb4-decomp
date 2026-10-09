#pragma once

#include <cstddef>
#include <functional>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/dsp/TempoListener.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/generators/CompositeGenerator.h"
#include "audio/core/music/SongPos.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "math/transform/Transform.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class AudioEmitterCom;

// The emitter interface of the audio emitter component (vtable 0x18DF628).
// The component's RuntimeData holds it at +0x228 in the component, with a
// back pointer to the component after it; generators, play requests and the
// sound manager keep this interface as their emitter. The slots forward to
// the component's members, whose names come from the map's
// AudioEmitterCom, or read the component's properties directly. The class
// is newer than the map. Name not in the reference map: the class id the
// component registers, and SoundManager's GetDefaultAudioEmitter, which
// returns it, suggest it; the evidence is weak.
class AudioEmitter {
public:
    // Slot 0 at 0x379D0: the component's CompositeGenerator, the parent of
    // every sound the emitter plays. Name not in the reference map.
    virtual AudioGenerator* GetCompositeGenerator();
    // Slot 1 at 0x379E0: adds a tempo listener and sends it the current
    // tempo. AudioGenerator::RegisterTempoListener forwards here.
    virtual void RegisterTempoListener(TempoListener* listener);
    // Slot 2 at 0x379F0: false when the listener was not registered. Name
    // not in the reference map.
    virtual bool UnregisterTempoListener(TempoListener* listener);
    // Slots 3-10 at 0x37A00 and 0x37AB0 through 0x37B10 return the new
    // handle. Slot 3 inlines the component's PlaySound.
    virtual unsigned int PlaySound(PlayArgs& args);
    virtual unsigned int PlaySound(Symbol name);
    virtual unsigned int PrepareSound(Symbol name);
    virtual unsigned int PlayMusic(PlayMusicArgs& args);
    virtual unsigned int PlayMusic(
        Symbol name, MusicSyncOptions sync, MusicTimelineMapping mapping, MusicUnmutePoint unmute);
    virtual unsigned int PlayMusic(Symbol name, Symbol sync);
    virtual unsigned int PrepareMusic(Symbol name, MusicSyncOptions sync);
    virtual unsigned int PrepareMusic(Symbol name, Symbol sync);
    // Slots 11-14 at 0x37B20 through 0x37B80 stop, kill, pause and continue
    // the composite generator.
    virtual void StopAllSounds();
    virtual void KillAllSounds();
    virtual void PauseAllSounds();
    virtual void ContinueAllSounds();
    // Slot 15 at 0x37BA0: the handle of the music the emitter follows.
    virtual unsigned int GetMasterMusic();
    // Slot 16 at 0x37BB0: the emitter's mix group; null in this build. When
    // present, the FMOD generators route through its channel group. Name not
    // in the reference map.
    virtual void* GetMixGroup();
    // Slot 17 at 0x37BC0: the component's world transform, or the listener's
    // for a 2D emitter. Name not in the reference map.
    virtual const Transform& GetWorldXfm();
    // Slot 18 at 0x37BE0: plays a dialog line through the component. The
    // last argument is an object reference (the map's ObjPtr). Name not in
    // the reference map.
    virtual unsigned int PlayDialog(
        Symbol name,
        bool interruptible,
        const std::function<void(AudioEmitter*, Symbol, void*)>& sink,
        const void* object);
    // Slot 19 at 0x37CB0: the same for a prepared request. Name not in the
    // reference map.
    virtual unsigned int PlayDialog(DialogPlayArgs& args);
    // Slots 20-23 at 0x37CC0 through 0x37D00 forward to the component's
    // dialog members. Names not in the reference map.
    virtual bool IsDialogPlaying();
    virtual bool IsDialogUninterruptible();
    virtual bool SetDialogInterruptible(bool interruptible);
    virtual void SetAllowDialogOverlap(bool allow);
    // Slots 24-27 at 0x37D20 through 0x37D50 read and write the "is_2D"
    // property; a 2D emitter is not positioned. Names not in the reference
    // map.
    virtual bool Is2D();
    virtual bool Is3D();
    virtual void Set2D(bool is2D);
    virtual void Set3D(bool is3D);
    // Slot 28 at 0x37D60: the owning component. Name not in the reference
    // map.
    virtual Component* GetComponent();

    // The owning component. Name not in the reference map.
    AudioEmitterCom* mComponent;
};

static_assert(sizeof(AudioEmitter) == 16);

// The "tempo_aware_buses" struct of the emitter: the FMOD buses that receive
// the tempo of the music the emitter plays. The map names the type through
// its element, TempoAwareBusProps::BusName. Field names follow the
// properties; they are not in the reference map.
struct TempoAwareBusProps {
    // The "buses" array element.
    struct BusName {
        Symbol mBus;           // "bus"
        Symbol mPreloadedBus;  // "preloaded_bus"
    };

    bool mUsePreloadedBank;  // "use_preloaded_bank"
    PropArray<BusName> mBuses;
};

static_assert(sizeof(TempoAwareBusProps::BusName) == 16);
static_assert(offsetof(TempoAwareBusProps, mBuses) == 8);
static_assert(sizeof(TempoAwareBusProps) == 48);

// The component that plays sounds at its object's position
// (audio/AudioEmitterCom.o, 0x31D70 to 0x392DF). Its class id is
// "AudioEmitter", with "AudioEmitterCom" as an alias. The sounds it plays are
// children of its CompositeGenerator; the music it plays can drive the
// tempo listeners and tempo-aware FMOD buses registered with it. A named
// emitter takes one of the game's emitter names, under which the static
// registry finds it. The vtable at 0x18DF2E8 has 59 slots: Component's 41
// and the 18 below. The object is 568 bytes.
class AudioEmitterCom : public Component {
public:
    // The members that are not properties (constructed at 0x37710, copied at
    // 0x37F70). Field names are not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x37710
        // Copies the values, not the generator or the listeners. Not
        // reconstructed.
        RuntimeData(const RuntimeData& other);  // 0x37F70

        // The playing music's position and its content position, and the
        // section name: the "bar", "beat", "tick" and "section" properties.
        SongPos mSongPos;
        SongPos mContentPos;
        Symbol mSectionName;
        // The tempo and speed sent to the tempo listeners; 120 until music
        // plays. Weak names: the poll reads them from music generator slots
        // that are not reconstructed.
        float mTempo;
        float mSpeed;
        // The handle of the music the emitter follows.
        unsigned int mMasterMusic;
        // The object's world transform, copied by _Poll(Transform const&).
        Transform mWorldXfm;
        CompositeGenerator mComposite;
        // Guards the tempo listener list; created by the constructor.
        CritSec* mTempoListenersLock;
        LinkedListSizeTracked::List<TempoListener, &TempoListener::mNode> mTempoListeners;
        // The listener component of a joypad emitter (SoundManager::
        // InitJoypadEmitters).
        Component* mListener;
        // Set while the master music plays; the poll resets the positions
        // once it stops.
        bool mMusicPlaying;
        // The current dialog line's handle and whether it may be
        // interrupted.
        unsigned int mDialogHandle;
        bool mDialogInterruptible;
        // The tempo-aware bus paths, whether each bus has been bound, and
        // the tempo listeners the FMOD bus interface returned for them.
        eastl::vector<Symbol> mTempoAwareBuses;
        eastl::vector<bool> mTempoAwareBusesBound;
        eastl::vector<TempoListener*> mBusTempoListeners;
        // The render target of the emitter's sounds; empty selects the
        // default.
        Symbol mRenderTarget;
        AudioEmitter mEmitter;
    };

    AudioEmitterCom();            // 0x31D80
    ~AudioEmitterCom() override;  // slots 0-1: 0x31E20, 0x320F0

    // Component slots.
    // Slot 2. Not reconstructed.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x35B40
    Symbol GetId() const override;             // slot 4: 0x37030
    Symbol GetClassName() const override;      // slot 5: 0x37040
    int CurrentRev() const override;           // slot 7: 0x37050
    bool IsA(Symbol type) const override;      // slot 8: 0x37070
    Component* AsComponent() override;         // slot 9: 0x370A0
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Not
    // reconstructed: the thread's imprint state is not modelled.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x370B0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x37240
    ComMetaData& _GetMetaData() override;       // slot 23: 0x37250
    // Slot 29 at 0x33300: points the composite generator at the emitter.
    bool _OnResourcesLoaded() override;
    void _Enter() override;                      // slot 31: 0x33350
    void _Exit(DestroyType type) override;       // slot 32: 0x335E0
    // Slot 33 at 0x33540: polls with the object's TransCom world transform.
    // The map's _Poll(ObjPtr const&).
    void _Poll() override;
    // Slots 36-38 at 0x33320 through 0x33340 run the game-mode slots.
    void _EditEnter() override;
    void _EditExit(DestroyType type) override;
    void _EditPoll() override;

    // Slot 41 at 0x34950: false in this build; no caller was found. Name
    // not in the reference map: it is taken to be the component's side of
    // the interface's GetMixGroup, which is null in this build. Weak.
    virtual bool HasMixGroup() const;
    // Slot 42 at 0x34760: plays the music request, following the master
    // music unless the request names its own.
    virtual unsigned int PlayMusic(PlayMusicArgs& args);
    // Slots 43-46 at 0x34960, 0x34B40, 0x34B90 and 0x34D60 build a music
    // request; the Symbol forms convert the sync name.
    virtual unsigned int PlayMusic(
        Symbol name, MusicSyncOptions sync, MusicTimelineMapping mapping, MusicUnmutePoint unmute);
    virtual unsigned int PlayMusic(Symbol name, Symbol sync);
    virtual unsigned int PrepareMusic(Symbol name, MusicSyncOptions sync);
    virtual unsigned int PrepareMusic(Symbol name, Symbol sync);
    virtual unsigned int GetMasterMusic();          // slot 47: 0x37260
    virtual SongPos GetCurrentMusicSongPos();       // slot 48: 0x37270
    virtual SongPos GetCurrentMusicContentPos();    // slot 49: 0x37290
    virtual Symbol GetCurrentMusicSectionName();    // slot 50: 0x372B0
    // Slot 51 at 0x34D90: plays a dialog line, stopping the current one
    // unless overlap is allowed; a line that cannot interrupt the current
    // one is refused. An object reference that names another emitter is
    // not handled. Name not in the reference map. Not reconstructed.
    virtual unsigned int PlayDialog(
        Symbol name,
        bool interruptible,
        const std::function<void(AudioEmitter*, Symbol, void*)>& sink,
        const void* object);
    // Slot 52 at 0x35110: the same for a prepared request. Name not in the
    // reference map.
    virtual unsigned int PlayDialog(DialogPlayArgs& args);
    // Slot 53 at 0x372C0: whether the dialog handle names a generator. Name
    // not in the reference map.
    virtual bool IsDialogPlaying();
    // Slot 54 at 0x372F0: a dialog line plays that may not be interrupted.
    // Name not in the reference map.
    virtual bool IsDialogUninterruptible();
    // Slot 55 at 0x35230: sets the line's flag when the handle names a
    // dialog generator. Name not in the reference map.
    virtual bool SetDialogInterruptible(bool interruptible);
    // Slot 56 at 0x37310: the "allow_dialog_overlap" property. Name not in
    // the reference map.
    virtual void SetAllowDialogOverlap(bool allow);
    virtual void AddGenerator(AudioGenerator* generator);  // slot 57: 0x37320
    // Slot 58 at 0x33760: moves the emitter, polls the composite generator,
    // binds the tempo-aware buses and follows the master music's tempo and
    // position. The map's _Poll(Transform const&).
    virtual bool _Poll(const Transform& xfm);

    // Plays the request on this emitter. A music request (format 1) goes to
    // PlayMusic; any other is played by the sound manager with this emitter
    // and render target. At 0x34210.
    unsigned int PlaySound(PlayArgs& args);
    // Plays the request with this emitter and, unless it names one, its
    // render target. No caller in this build; the other plays inline it. At
    // 0x342C0. Name not in the reference map.
    unsigned int _PlayOnEmitter(PlayArgs& args);
    // Plays the named sound on this emitter, through a request when the
    // emitter has a render target. At 0x34360.
    unsigned int PlaySound(Symbol name);
    // The same, started paused. At 0x34560.
    unsigned int PrepareSound(Symbol name);
    // Whether the composite generator has children. At 0x35B30.
    bool IsPlaying() const;
    void StopAllSounds();      // 0x33750
    void KillAllSounds();      // 0x35320
    void PauseAllSounds();     // 0x35330
    void ContinueAllSounds();  // 0x35340
    bool SetParameter(Symbol name, float value);   // 0x35350
    bool GetParameter(Symbol name, float& value);  // 0x35360
    // The "is_2D" property. Names not in the reference map.
    bool Is2D() const;          // 0x352E0
    bool Is3D() const;          // 0x352F0
    void Set2D(bool is2D);      // 0x35300
    void Set3D(bool is3D);      // 0x35310
    // Adds the listener, under TempoListener's lock and the emitter's, and
    // sends it the current tempo. At 0x34120.
    void RegisterTempoListener(TempoListener* listener);
    // False when the listener was not in the list. At 0x35370. Name not in
    // the reference map.
    bool UnregisterTempoListener(TempoListener* listener);
    // Makes the handle the master music when the request is a music request
    // and the generator is music, and tells the tempo listeners its tempo.
    // At 0x34860.
    void _SetupEmitterMusic(unsigned int handle, const PlayMusicArgs& args);
    // Releases the tempo-aware buses' bindings and rebuilds the bus list
    // from the property; the poll binds the loaded buses. At 0x33C70. Name
    // not in the reference map.
    void _UpdateTempoAwareBuses();
    // Adds and removes the emitter under its emitter name in
    // sNamedEmitters. Names not in the reference map.
    void _RegisterNamedEmitter();    // 0x33370
    void _UnregisterNamedEmitter();  // 0x33620

    // Registers the class: the metadata heap, _Init, the factories and the
    // class's ComMetaData. Inline in the map; this build emits it in
    // audio/SoundManager.o at 0x42F0. Not reconstructed: the registration
    // helpers it inlines are not modelled.
    static void Init();
    // The class factory. Emitted in audio/SoundManager.o at 0xCF80.
    static Component* _Create();
    // Registers the properties and the class description. Not
    // reconstructed: the property metadata's attributes are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x32110

    static Symbol sId;          // 0x19C7768, "AudioEmitter"
    // The class symbol GameObject::CreateComponent takes; also
    // "AudioEmitter". Name not in the reference map.
    static Symbol sClassName;   // 0x19C7770
    static PropRegistry sPropRegistry;  // 0x19C7780
    static ComMetaData sMetaData;       // 0x19C7820
    // The named emitters by emitter name, and their lock. Names not in the
    // reference map.
    static CritSec sNamedEmittersLock;  // 0x19C7720
    static eastl::map<Symbol, eastl::vector<AudioEmitterCom*>> sNamedEmitters;  // 0x19C7730

    // Field names are not in the reference map.
    // Placed in the Component base's tail padding.
    bool mIsNamedEmitter;  // "is_named_emitter"
    Symbol mEmitterName;   // "emitter_name"
    bool mIs2D;            // "is_2D"
    bool mAllowDialogOverlap;  // "allow_dialog_overlap"
    // Not initialized by the constructor; _Imprint copies it. No reader was
    // found.
    unsigned long mReserved;
    // Set to the empty symbol by the constructor; no reader was found.
    Symbol mReservedName;
    TempoAwareBusProps mTempoAwareBusProps;  // "tempo_aware_buses"
    RuntimeData mRuntime;
};

// Adds an empty entry for the emitter name to AudioEmitterCom::
// sNamedEmitters, which named emitters then join; SoundManager reads the
// names from its "game_wide_emitter_names" configuration. At 0x35450. Name
// not in the reference map.
void AddGameWideEmitterName(Symbol name);
// The emitter that last joined the name's entry, or null. At 0x35820. Name
// not in the reference map.
AudioEmitterCom* GetGameWideEmitter(Symbol name);

static_assert(offsetof(AudioEmitterCom, mIsNamedEmitter) == 0x16);
static_assert(offsetof(AudioEmitterCom, mEmitterName) == 0x18);
static_assert(offsetof(AudioEmitterCom, mIs2D) == 0x20);
static_assert(offsetof(AudioEmitterCom, mAllowDialogOverlap) == 0x21);
static_assert(offsetof(AudioEmitterCom, mReservedName) == 0x30);
static_assert(offsetof(AudioEmitterCom, mTempoAwareBusProps) == 0x38);
static_assert(offsetof(AudioEmitterCom, mRuntime) == 0x68);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mContentPos) == 0x1C);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mSectionName) == 0x38);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mTempo) == 0x40);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mMasterMusic) == 0x48);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mWorldXfm) == 0x4C);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mComposite) == 0x80);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mTempoListenersLock) == 0x120);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mTempoListeners) == 0x128);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mListener) == 0x140);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mMusicPlaying) == 0x148);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mDialogHandle) == 0x14C);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mDialogInterruptible) == 0x150);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mTempoAwareBuses) == 0x158);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mTempoAwareBusesBound) == 0x178);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mBusTempoListeners) == 0x198);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mRenderTarget) == 0x1B8);
static_assert(offsetof(AudioEmitterCom::RuntimeData, mEmitter) == 0x1C0);
static_assert(sizeof(AudioEmitterCom) == 0x238);
