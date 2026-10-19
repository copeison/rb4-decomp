#pragma once

#include <cstddef>
#include <functional>

#include "anim/core/AnimResource.h"
#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

class CharAnim;
class CharAnimCom;
class CharAnimRaw;
class GameObject;

// A character animation, extension "charanim" (anim/CharAnimResource.o,
// 0x2DFE90-0x2E14DF): an entity resource whose root object holds the
// CharAnimCom with the animation's settings and events, and the compressed
// CharAnim. A cached file holds the CharAnim; otherwise the animation is
// built after the entity loads, from the raw animation and bone mask the
// component names. The animation interface (AnimResource) is at offset 200.
// The vtable at 0x18F52D8 has 48 slots, slots 33-47 being the animation
// interface's overriders, in their declaration order; the interface's
// vtable is at 0x18F5468. The object is 224 bytes.
class CharAnimResource : public EntityResource, public AnimResource {
public:
    // Called with each animation built from its raw animation (the map's
    // RegisterCacheVisitor argument type).
    using CacheVisitor = std::function<void(CharAnimResource*, CharAnim*, CharAnimRaw*)>;

    CharAnimResource();  // 0x2DFE90
    // Slots 10-11: 0x2DFF00, 0x2DFF60.
    ~CharAnimResource() override;

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x2E11E0
    Symbol GetId() const override;                   // slot 1: 0x2E11F0
    bool IsA(Symbol type) const override;            // slot 2: 0x2E1290
    // Slot 5: the entity, then, in a cached file, the animation.
    void Save(BinStream& stream, bool cached) override;  // 0x2E0D90
    // Slot 6: without an entity or an animation.
    bool Fail() const override;  // 0x2E12C0
    // Slot 13: a new entity with a CharAnimCom on its root.
    Entity* CreateEntity() override;  // 0x2E11A0
    // Slot 14: drops the animation and reads the entity.
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x2E0140
    // Slot 15: finds the root's CharAnimCom and reads the animation from a
    // cached file, or builds it: loads the bone mask and the raw animation
    // the component names, and the pose the animation is relative to (its
    // own first frame, another animation's raw animation, or the bind pose
    // of the mask's skeleton), then lets CharAnim::Create compress it and
    // tells the cache visitor. Nothing is built while caching is disabled
    // or without a bone mask. The map's Load(BinStream&, bool) has this
    // work.
    bool _PostLoad(BinStream& stream, bool cached) override;  // 0x2E01A0

    // Slots 33-47: the animation interface. Its vtable's entries are thunks
    // (0x2E1300, 0x2E0E30, 0x2E1320, 0x2E0E60, ...) that adjust by -200.
    Resource* AsResource() override;  // 0x2E12E0
    // The root object of the resource's own entity; the argument is not
    // read.
    GameObject* GetObject(Entity* entity) const override;  // 0x2E0E10
    // The CharAnim.
    void* GetCharData() const override;  // 0x2E12F0
    // "is_looping".
    bool IsLooping() const override;          // 0x2E0E50
    void SetLooping(bool looping) override;  // 0x2E0E70
    // Frame 0, and the last keyframe.
    float GetStartFrame() const override;  // 0x2E0E90
    float GetEndFrame() const override;    // 0x2E0EB0
    // Beats for any "playback_units" but real-time, else seconds.
    TimeUnits GetUnits() const override;  // 0x2E0EF0
    // Through the beat track on the beat timeline, else the frame rate.
    // kNumTimeUnits takes the animation's own units.
    float GetTimeFromFrame(float frame, TimeUnits units) const override;  // 0x2E0F30
    float GetFrameFromTime(float time, TimeUnits units) const override;   // 0x2E0FD0
    // On the beat timeline, 1, 4 or 2 beats for the beat, downbeat and
    // two-beat aligned units; 0 otherwise.
    float GetTimeAlignment(TimeUnits units) const override;  // 0x2E1070
    // The CharAnimCom's "events".
    const PropArray<Event>* GetEvents(Entity* entity) const override;  // 0x2E1120
    // From the CharAnim, which takes them from its bone mask.
    BlendMode GetBlendMode() const override;          // 0x2E1160
    unsigned long GetSubTreeIndex() const override;  // 0x2E1180
    // 0: the events are authored on the animation itself.
    float GetEventFrameOffset() const override;  // 0x2E1140

    // Replaces the animation with an empty one for an exporter; the old
    // one is not freed. No caller remains.
    void InitForExport();  // 0x2DFED0

    // The class id, created on first use and inlined into its users, such
    // as StateNodeAnimate::_OnResourcesLoaded (0x70EAB0). The local static
    // is at 0x19F7C88.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharAnimResource");
        }
        return id;
    }

    // Registers the class as an entity resource. The binary emits it in
    // anim/AnimInit.o at 0x2ADD10, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B5E00.
    static Resource* _Create();
    // Names the extension ("charanim") and the category ("Animations") and
    // reads "disable_char_anim_caching" from the "anim" config.
    static void _Init(ResourceMetaData& metaData);  // 0x2DFFC0
    // Sets the visitor told about each animation built. The caller is at
    // 0x106D705.
    static void RegisterCacheVisitor(CacheVisitor visitor);  // 0x2DFE20
    // Whether building animations is disabled. No caller remains.
    static bool IsCachingDisabled();  // 0x2E0130

    // Constructed by the static initializer at 0x2E1480, which first sets a
    // shared header's null object id to -1 (0x19F86E0).
    static ResourceMetaData sMetaData;  // 0x19F86E8
    static CacheVisitor sCacheVisitor;  // 0x19F8740

    // Field names are not in the reference map.
    // The CharAnimCom on the entity's root.
    CharAnimCom* mCom;
    CharAnim* mAnim;
};

static_assert(offsetof(CharAnimResource, mCom) == 208);
static_assert(offsetof(CharAnimResource, mAnim) == 216);
static_assert(sizeof(CharAnimResource) == 224);
