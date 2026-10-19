#pragma once

#include <cstddef>

#include "anim/core/AnimResource.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// The component on the root object of a character animation's entity
// (anim/CharAnimCom.o, 0x2D8150-0x2DD2EF; class id "CharAnim", alias
// "CharAnimCom"): the animation's playback units, its looping, whether it
// plays relative to the bind pose or another animation, its events and the
// raw animation and bone mask it was built from. CharAnimResource reads
// these properties when it loads or caches the animation. The vtable at
// 0x18F4BE8 has 41 slots. The object is 112 bytes. The map's methods take
// the object (ObjPtr const&) first; this build's do not.
class CharAnimCom : public Component {
public:
    // What "relative" plays the animation relative to. Names not in the
    // reference map; the allowed values' labels name them.
    enum Relative {
        // "Not Relative": played back normally.
        kRelativeNone = 0,
        // "Bind Relative": additively, relative to the bind pose.
        kRelativeBind = 1,
        // "Other Relative": additively, relative to the first frame of
        // "relative_anim".
        kRelativeOther = 2,
    };

    CharAnimCom();  // 0x2D8150
    // Copies the properties for an imprint; the animation pointer is not
    // copied. Inlined into _Imprint (0x2D9CE0).
    CharAnimCom(const CharAnimCom& other);
    // Slots 0-1: 0x2D81D0 and 0x2D82A0.
    ~CharAnimCom() override;

    Symbol GetId() const override;          // slot 4: 0x2D9C60
    Symbol GetClassName() const override;   // slot 5: 0x2D9C70
    int CurrentRev() const override;        // slot 7: 0x2D9C80
    bool IsA(Symbol type) const override;   // slot 8: 0x2D9CA0
    Component* AsComponent() override;      // slot 9: 0x2D9CD0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2D9CE0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2D9E60
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2D9E70
    // Slot 29: takes the animation interface of the entity's
    // CharAnimResource and sorts the events by frame. The map's
    // _LoadResources(ObjPtr const&) has this place.
    bool _OnResourcesLoaded() override;  // 0x2D9AB0
    // Slots 31 and 33: empty.
    void _Enter() override;  // 0x2D9BC0
    void _Poll() override;   // 0x2D9BD0
    // Slot 38: polls as in the game mode (slot 33).
    void _EditPoll() override;  // 0x2D9E80

    // The relative mode, and in `path` the animation it is relative to
    // ("relative_anim" for kRelativeOther, else the empty path).
    int GetRelative(ResourcePath& path) const;  // 0x2D8320
    // The animation interface _OnResourcesLoaded found; no caller remains.
    // Name not in the reference map.
    AnimResource* GetAnim() const;  // 0x2D9BE0
    // The index of the last event named `name` before `index`, or the
    // event count.
    unsigned long FindPrevEvent(Symbol name, unsigned long index) const;  // 0x2D9BF0
    // The index of the first event named `name` from `index` on, or the
    // event count.
    unsigned long FindNextEvent(Symbol name, unsigned long index) const;  // 0x2D9C20

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2AE440.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B5F00 (the
    // placement constructor at 0x2B5F30).
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2D8350

    // "CharAnim", constructed by the static initializer at 0x2DD220.
    static Symbol sId;  // 0x19F83B0
    // Also "CharAnim". Name not in the reference map.
    static Symbol sClassName;           // 0x19F83B8
    static PropRegistry sPropRegistry;  // 0x19F83C0
    static ComMetaData sMetaData;       // 0x19F8460
    // "rawanim": the sub-object class of "anim_source_file", which the
    // revision handler fixes in paths saved before revision 2.
    static Symbol kSubObjectResource;  // 0x19F8608

    // Field names are not in the reference map; the properties name them.
    // "playback_units": a CharAnim::Units.
    int mPlaybackUnits;
    // "relative": a Relative.
    int mRelative;
    // "relative_anim".
    ResourcePath mRelativeAnim;
    // "is_looping".
    bool mIsLooping;
    // "events", sorted by frame when the resources load.
    PropArray<AnimResource::Event> mEvents;
    // "anim_source_file": the raw animation (a CharAnimRawResource).
    ResourcePath mAnimSourceFile;
    // "bone_mask_file": a CharBoneMaskResource.
    ResourcePath mBoneMaskFile;
    // The animation interface of the entity's resource, or null; the event
    // views convert frames through it. Set by _OnResourcesLoaded only.
    AnimResource* mAnim;
};

static_assert(offsetof(CharAnimCom, mPlaybackUnits) == 24);
static_assert(offsetof(CharAnimCom, mRelative) == 28);
static_assert(offsetof(CharAnimCom, mRelativeAnim) == 32);
static_assert(offsetof(CharAnimCom, mIsLooping) == 40);
static_assert(offsetof(CharAnimCom, mEvents) == 48);
static_assert(offsetof(CharAnimCom, mAnimSourceFile) == 88);
static_assert(offsetof(CharAnimCom, mBoneMaskFile) == 96);
static_assert(offsetof(CharAnimCom, mAnim) == 104);
static_assert(sizeof(CharAnimCom) == 112);
