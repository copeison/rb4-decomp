#pragma once

#include <cstddef>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class Entity;
class GameObject;
class Resource;

// The interface of an animation resource (anim/AnimResource.o,
// 0x32C760-0x32CF92): its frame range, looping and timeline, the
// conversions between frames and times, and its events. The animation
// resources inherit it after their resource base: PropAnimResource and
// CharAnimResource at offset 200 (vtables 0x18F7308 and 0x18F5468),
// CharClipResource at 48 (0x18F68D0). Its own vtable is at 0x18FA340 (18
// slots); it has no destructor. AnimDriver and PlaybackInfo play an
// animation through it. Names of slots 0 to 3, 14 to 17 and their helpers
// are not in the reference map.
class AnimResource {
public:
    // How a character animation combines with the ones before it (the map's
    // AnimResource::BlendMode; the enumerator names follow the allowed
    // values' labels).
    enum BlendMode {
        // "Additive": adds its weight and joint data; all are normalized
        // at the end.
        kBlendAdditive = 0,
        // "Layered": blends over the previous data, superseding older clips
        // with the same blend order and clips with a lower one.
        kBlendLayered = 1,
    };

    // An event authored in an animation: its frame and its name, which a
    // playback sends as "anim_event" when it passes the frame
    // (PlaybackInfo::SendEvents). Anim::Init registers the struct as
    // "AnimResource::Event"; the map has CharAnimCom::AnimEvent in its
    // place. Field names are not in the reference map.
    struct Event {
        float mFrame;
        Symbol mName;
    };

    // Slots 0-1: the resource the interface belongs to. Slot 1, emitted
    // at 0x2E1310, calls slot 0.
    virtual Resource* AsResource() = 0;
    virtual const Resource* AsResource() const {
        return const_cast<AnimResource*>(this)->AsResource();
    }
    // Slot 2: the object of the entity the animation's events come from: a
    // property animation's own root, a clip's CharClipCom object in the
    // entity. Null here.
    virtual GameObject* GetObject(Entity* entity) const;  // 0x32CF90
    // Slot 3: the character data the animation holds (a CharAnim), or
    // null; the character resources' GetBlendMode and GetSubTreeIndex read
    // it. Null here, in a body folded with others at 0x2FFC00. The map's
    // GetSignature() const has its place; the evidence for the name is
    // weak.
    virtual void* GetCharData() const {
        return nullptr;
    }
    // Slots 4-5.
    virtual bool IsLooping() const = 0;
    virtual void SetLooping(bool looping) = 0;
    // Slots 6-7: the first and last frames.
    virtual float GetStartFrame() const = 0;
    virtual float GetEndFrame() const = 0;
    // Slot 8: the timeline the animation is authored on.
    virtual TimeUnits GetUnits() const = 0;
    // Slots 9-10: how a character animation blends and the blend-tree slot
    // it plays in, both from its character data (0xB bytes and 0xC bytes in
    // CharAnimResource, as the map has them). The defaults, emitted in
    // anim/PropAnimResource.o at 0x2FFC10 and 0x2FFC20 as the map has them,
    // return 0.
    virtual BlendMode GetBlendMode() const {
        return kBlendAdditive;
    }
    virtual unsigned long GetSubTreeIndex() const {
        return 0;
    }
    // Slots 11-12: the time of a frame and the frame at a time, on the
    // timeline.
    virtual float GetTimeFromFrame(float frame, TimeUnits units) const = 0;
    virtual float GetFrameFromTime(float time, TimeUnits units) const = 0;
    // Slot 13: the period a playback on the timeline keeps in phase with
    // its clock, or 0; a clip on "beat aligned" units aligns to one or
    // four beats.
    virtual float GetTimeAlignment(TimeUnits units) const = 0;
    // Slot 14: the animation's events as played in the entity, or null.
    // Name not in the reference map.
    virtual const PropArray<Event>* GetEvents(Entity* entity) const = 0;
    // Slots 15-16: the range the events and the editor use; the playback
    // range here, a clip's own range in its source animation. Names not in
    // the reference map; the evidence is weak.
    virtual float GetAuthoredStartFrame() const;  // 0x32CF10
    virtual float GetAuthoredEndFrame() const;    // 0x32CF20
    // Slot 17: the frame of the source animation the playback's frame 0
    // is; a clip's start, 0 for the others. Name not in the reference map.
    virtual float GetEventFrameOffset() const = 0;

    // The playback frame of an event authored at `frame`. Name not in the
    // reference map.
    float EventFrameToFrame(float frame) const;  // 0x32CF30
    // The event frame of a playback frame. Name not in the reference map.
    float FrameToEventFrame(float frame) const;  // 0x32CF60

    // The labels of BlendMode for a "blend_mode" property. Name not in the
    // reference map.
    static eastl::vector<AllowedValue<int>> GetBlendModeAllowedValues();  // 0x32C760
};

static_assert(offsetof(AnimResource::Event, mName) == 8);
static_assert(sizeof(AnimResource::Event) == 16);

// The animation resource classes a node can play, as the allowed values
// of StateNodeAnimate's and StateNodeSetAnimFrame's "anim_type":
// PropAnimResource, CharAnimResource and CharClipResource. Name not in the
// reference map; the binary emits it before ui/UI.o, and its object is not
// identified.
eastl::vector<AllowedValue<Symbol>> GetAnimTypeAllowedValues();  // 0x891390
