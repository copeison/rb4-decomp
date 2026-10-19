#pragma once

#include <cstddef>

#include "anim/drivers/AnimDriver.h"
#include "entity/props/PropRef.h"
#include "utl/containers/Vector.h"

class PropKeysBaseCom;

// The driver of one animated property (the map's PropDriver, with a
// TPropDriver<T> for each key component class, such as
// TPropDriver<PropKeysFloatCom>): the playbacks and key components that
// set the property, blended by the playbacks' weights each poll. Only the
// interface PropAnimDriver calls is declared; the classes are not
// reconstructed.
class PropDriver {
public:
    // Slots 0-1.
    virtual ~PropDriver() {}
    // Slot 2: the number of key components the driver plays.
    virtual unsigned long GetNumPropKeysComs() const = 0;
    // Slot 3: plays the keys under the playback on the entity. The map's
    // Add(PlaybackInfo const*, PropKeysBaseCom const*); this build adds the
    // entity.
    virtual void Add(const PlaybackInfo* info, const PropKeysBaseCom* keys, Entity* entity) = 0;
    // Slot 4: drops the playback's keys.
    virtual void Remove(const PlaybackInfo* info) = 0;
    // Slot 5: sets the property from the playbacks. The map's
    // Poll(ObjPtr const&): the entity's root object.
    virtual void Poll(GameObject* root) = 0;

    // The property driven. Name not in the reference map.
    PropRef mProp;
};

static_assert(offsetof(PropDriver, mProp) == 8);

// The driver of property animations (anim/PropAnimDriver.o,
// 0x2FCAC0-0x2FD209): each playback of a PropAnimResource feeds the
// property drivers of the properties its keys drive, one driver per
// property, kept sorted by the property. The vtable is at 0x18F7158. Field
// names are not in the reference map.
class PropAnimDriver : public AnimDriver {
public:
    PropAnimDriver();  // 0x2FCAC0
    // Slots 0-1: 0x2FCB10 and 0x2FCBE0. Deletes the property drivers.
    ~PropAnimDriver() override;
    // Slot 2: starts the property animation and adds its keys to the
    // drivers of their properties, creating the missing drivers.
    AnimInstID StartAnim(Resource* resource, const AnimOptions& options) override;  // 0x2FCD00
    // Slot 3: drops the playback from the property drivers and deletes the
    // ones left without keys.
    void _OnStopAnim(PlaybackInfo& info) override;  // 0x2FCFC0
    // Slot 4: polls every property driver.
    void _Poll(GameObject* root) override;  // 0x2FCCB0

    // The driver of the property, or null. No caller was found.
    PropDriver* _FindPropDriver(const PropRef& prop);  // 0x2FD070

    // The property drivers, owned and sorted by their properties.
    eastl::vector<PropDriver*> mPropDrivers;
};

static_assert(offsetof(PropAnimDriver, mPropDrivers) == 64);
static_assert(sizeof(PropAnimDriver) == 96);
