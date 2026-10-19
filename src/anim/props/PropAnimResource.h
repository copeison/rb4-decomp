#pragma once

#include <cstddef>

#include "anim/core/AnimResource.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BeatSource;
class Entity;
class PropAnimCom;
class PropKeysBaseCom;
class PropPath;
class PropRef;
class ResourceMetaData;

// A property animation, extension "propanim" (anim/PropAnimResource.o,
// 0x2FD210-0x2FFD5F): an entity resource whose root object holds the
// PropAnimCom that describes the animation and whose other objects each hold
// the PropKeys component that keys one property of the entity the animation
// plays on. The animation interface (AnimResource) is at offset 200. The
// vtable at 0x18F7190 has 45 slots, slots 33-44 being the animation
// interface's overriders, in their declaration order; the interface's
// vtable is at 0x18F7308. Its factory (0x2B1DC0) allocates 216 bytes. Field
// names are not in the reference map.
class PropAnimResource : public EntityResource, public AnimResource {
public:
    // The revision Save writes after the entity. Revision 2 replaced the
    // quaternion keys of a "rotation" property with rotation keys. Name not
    // in the reference map.
    static constexpr int kRev = 2;

    // Inlined into the factory.
    PropAnimResource() : mRev(0) {}
    // Slots 10-11: 0x2FD210 (a jump to EntityResource's destructor) and
    // 0x2FD220.
    ~PropAnimResource() override;

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x2FFAF0
    Symbol GetId() const override;                   // slot 1: 0x2FFB00
    bool IsA(Symbol type) const override;            // slot 2: 0x2FFBA0
    // Slot 5: the entity, then the revision. The map's
    // Save(BinStream&, bool).
    void Save(BinStream& stream, bool cached) override;  // 0x2FD340
    // Slot 13: a new entity with a PropAnimCom on its root.
    Entity* CreateEntity() override;  // 0x2FFAB0
    // Slot 14: reads the entity and, from entity revision 33, the
    // resource's revision. Before revision 2 the quaternion keys
    // (PropKeysQuatCom) are rebuilt as rotation keys (PropKeysRotationCom)
    // of the same property, each key's Euler angles computed from its
    // quaternion. The map's Load(BinStream&, bool) has this work.
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x2FD3A0

    // Slots 33-44: the animation interface. Its vtable's entries are thunks
    // that adjust by -200; most repeat the bodies (0x2FFBF0, 0x2FD880,
    // 0x2FD9B0, ...).
    Resource* AsResource() override;  // 0x2FFBD0
    // The root object of the resource's own entity; the argument is not
    // read.
    GameObject* GetObject(Entity* entity) const override;  // 0x2FD860
    // The PropAnimCom's "is_looping"; false without the component.
    bool IsLooping() const override;          // 0x2FD8A0
    void SetLooping(bool looping) override;  // 0x2FD9E0
    // The PropAnimCom's frame range; 0 for an unmeasured end and without
    // the component.
    float GetStartFrame() const override;  // 0x2FDA30
    float GetEndFrame() const override;    // 0x2FDA90
    // The timeline of the PropAnimCom's "time_units" (the frame-based
    // values 4-7 give their timeline 0-3); the default, 8, takes the
    // "default_timeline" of the AnimDriverCom on the root of the entity
    // resource the animation is inlined in. The map's GetUnits() const.
    TimeUnits GetUnits() const override;  // 0x2FDAF0
    // Frames and times on the timeline: 30 frames a second, 16 a beat, 1 a
    // unit for the frame-based units (AnimDriver::GetDefaultFramesPerUnit).
    // kNumTimeUnits takes the animation's own timeline. Between seconds
    // and beats an animation inlined in an RBSongResource converts through
    // the song's beat source; otherwise the time is not converted.
    float GetTimeFromFrame(float frame, TimeUnits units) const override;  // 0x2FDD70
    float GetFrameFromTime(float time, TimeUnits units) const override;   // 0x2FE140
    // No alignment.
    float GetTimeAlignment(TimeUnits units) const override;  // 0x2FFBE0
    // A property animation has no events.
    const PropArray<Event>* GetEvents(Entity* entity) const override;  // 0x2FE420
    // 0: the events are authored on the animation itself.
    float GetEventFrameOffset() const override;  // 0x2FE440

    // Sets the PropAnimCom's "time_units". The map's SetUnits(TimeUnits);
    // this build takes the raw value, which may be 4-8.
    void SetUnits(int units);  // 0x2FDD20
    // Measures the PropAnimCom's frame range again.
    void UpdateFrameRange();  // 0x2FE460
    // Replace and copy out the PropAnimCom's "target_order"; without the
    // component the order is left alone or emptied. Names not in the
    // reference map.
    void SetTargetOrder(const eastl::vector<GameObjectId>& order);  // 0x2FE480
    void GetTargetOrder(eastl::vector<GameObjectId>& order) const;  // 0x2FE4B0
    // The keys of the property, creating them with _AddKeys when the entity
    // has none.
    PropKeysBaseCom* AddKeys(PropertyType type, const PropRef& prop);  // 0x2FE4F0
    // The keys of the property: the existing ones, or new keys of the type
    // (and struct name) in a new object. Null without an entity, and when
    // other keys drive a property that contains this one or that this one
    // contains. The map's _AddKeys(PropertyType, Symbol const&,
    // PropRef const&); this build passes the symbol by value.
    PropKeysBaseCom* _AddKeys(PropertyType type, Symbol subType, const PropRef& prop);  // 0x2FE510
    // Adds keys for every property the other animation keys, each with one
    // key at frame 0 holding the other's value at its frame 0, and returns
    // them. No caller remains. The map's AddKeys(ResourcePtr<
    // PropAnimResource> const&, float, EntityPtr const&); this build reads
    // no frame and no entity.
    eastl::vector<PropKeysBaseCom*> AddKeys(const ResourcePtr<PropAnimResource>& source);  // 0x2FE7F0
    // Appends the key components of the entity's objects other than the
    // root, only those that drive a complete property unless
    // `includeIncomplete` is set. The map's HarvestAllKeys(eastl::vector<
    // PropKeysBaseCom*>&) const; this build adds the flag.
    void HarvestAllKeys(eastl::vector<PropKeysBaseCom*>& keys, bool includeIncomplete) const;  // 0x2FEAE0
    // The keys that drive the property, with their object's id, or null.
    PropKeysBaseCom* FindKeys(const PropRef& prop, GameObjectId& object) const;  // 0x2FEEB0
    // The first keys that drive a property of the object's component (any
    // component for the empty symbol), with their object's id, or null.
    PropKeysBaseCom* _FindNextKeys(GameObjectId object, Symbol com, GameObjectId& keysObject) const;  // 0x2FEFE0
    // Destroys every object but the root.
    void RemoveAllKeys();  // 0x2FF1B0
    // Destroys the keys of the property; false when there are none.
    bool RemoveKeys(const PropRef& prop);  // 0x2FF220
    // Destroys every key of the object's components and drops the object
    // from the target order; false when it had none.
    bool RemoveKeys(GameObjectId object);  // 0x2FF280
    // Destroys every key of the object's component; false when it had
    // none.
    bool RemoveKeys(GameObjectId object, Symbol com);  // 0x2FF410
    // Sets the keyed properties of the entity to their values at the
    // frame. The map's ApplyFrame(float, EntityPtr const&) const.
    void ApplyFrame(float frame, Entity* entity) const;  // 0x2FF4C0
    // Whether keys drive the property of the object's component (any
    // object for the invalid id, and any property for an empty path).
    // When `overlaps` is given, keys of a property that contains the path
    // or that the path contains set it and end the search. No caller
    // remains. The map's HasKeys(GameObjectId, Symbol, PropPath const&)
    // const; this build adds the flag.
    bool HasKeys(GameObjectId object, Symbol com, const PropPath& path, bool* overlaps) const;  // 0x2FF5B0
    // Moves every key by the offset; keys whose first key was at or before
    // `threshold` have that key moved to frame 0. Then measures the frame
    // range again. No caller remains. Name not in the reference map; the
    // evidence is weak.
    void OffsetKeys(float offset, float threshold);  // 0x2FF8F0
    // Whether the property has keys and exists in the entity. The map's
    // QueryValidPropTarget(PropRef const&, EntityPtr).
    bool QueryValidPropTarget(const PropRef& prop, Entity* entity) const;  // 0x2FFA50

    // The PropAnimCom on the entity's root, or null; without an entity the
    // error message is made and dropped. Name not in the reference map.
    PropAnimCom* GetPropAnimCom() const;  // 0x2FD8C0
    // The PropAnimCom's "time_units", the default (8) resolved as GetUnits
    // does unless `keepDefault` is set, but not reduced to a timeline. No
    // caller remains. Name not in the reference map; the evidence is weak.
    int _GetRawUnits(bool keepDefault) const;  // 0x2FDBB0
    // Whether the animation is inlined in an RBSongResource. No caller
    // remains. Name not in the reference map.
    bool _IsInSong() const;  // 0x2FDF80
    // The beat source of the song the animation is inlined in, or null:
    // the RBSongResource's song info component (0xF27480), its MIDI
    // resource (0xF216D0) and that resource's slot 19. Name not in the
    // reference map. Not reconstructed: the song classes of rb_audio are
    // not modelled.
    BeatSource* _GetSongBeatSource() const;  // 0x2FE040
    // The resource's path, after the path of the resource it is inlined
    // in ("%s:%s"), for error messages. No caller remains; the error paths
    // inline it. Name not in the reference map.
    const char* MakeErrorName() const;  // 0x2FE380

    // The class id, created on first use and inlined into its users, such
    // as UIScrollbarCom::_Init (0x8CD270) and AnimDriverCom::_FindDriver
    // (0x32A7D0). The local static is at 0x19F7BC8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("PropAnimResource");
        }
        return id;
    }

    // Registers the class as an entity resource. Emitted in
    // anim/AnimInit.o at 0x2A7B00, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B1DC0.
    static Resource* _Create();
    // Names the extension ("propanim").
    static void _Init(ResourceMetaData& metaData);  // 0x2FD240

    // Constructed by the static initializer at 0x2FFC70, which first sets a
    // shared header's null object id to -1 (0x19F9948).
    static ResourceMetaData sMetaData;  // 0x19F9950

    // The revision read with the entity; 0 for files older than entity
    // revision 33.
    int mRev;
};

static_assert(offsetof(PropAnimResource, mRev) == 208);
static_assert(sizeof(PropAnimResource) == 216);
