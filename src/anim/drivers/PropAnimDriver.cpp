// anim/PropAnimDriver.o (0x2FCAC0 to 0x2FD209). The binary also emits the
// driver vector's insertion here (0x2FD0D0) and two AnimDriver statics.
#include "anim/drivers/PropAnimDriver.h"

#include "anim/core/AnimOptions.h"
#include "anim/props/PropAnimResource.h"
#include "anim/props/PropKeysCom.h"
#include "os/memory/MemMgr.h"

// Reconstructed from eboot.elf at 0x2FCAC0.
PropAnimDriver::PropAnimDriver() : mPropDrivers() {}

// Reconstructed from eboot.elf at 0x2FCB10. The deleting destructor is at
// 0x2FCBE0.
PropAnimDriver::~PropAnimDriver() {
    for (PropDriver* driver : mPropDrivers) {
        delete driver;
    }
    mPropDrivers.clear();
}

// Reconstructed from eboot.elf at 0x2FCCB0.
void PropAnimDriver::_Poll(Entity* entity) {
    for (PropDriver* driver : mPropDrivers) {
        driver->Poll(entity);
    }
}

// Reconstructed from eboot.elf at 0x2FCD00. The keys are gathered with the
// temporary heap enabled.
AnimInstID PropAnimDriver::StartAnim(Resource* resource, const AnimOptions& options) {
    if (resource == nullptr || resource->Fail()) {
        return AnimInstID::NullID();
    }
    PropAnimResource* const anim = static_cast<PropAnimResource*>(resource);
    PlaybackInfo* const info = _StartAnim(anim, options);
    eastl::vector<PropKeysBaseCom*> keys;
    unsigned int saved = 0;
    MemPushTemp(saved, true, true);
    anim->GetPropKeys(keys, false);
    MemPopTemp(saved);
    mPropDrivers.reserve(keys.size());
    for (PropKeysBaseCom* const key : keys) {
        // The first driver whose property is not before the key's.
        PropDriver** position = mPropDrivers.begin();
        long count = static_cast<long>(mPropDrivers.size());
        while (count > 0) {
            const long half = static_cast<unsigned long>(count) >> 1;
            if (position[half]->mProp < key->mProp) {
                position += half + 1;
                count -= half + 1;
            } else {
                count = half;
            }
        }
        PropDriver* driver;
        if (position == mPropDrivers.end() || !((*position)->mProp == key->mProp)) {
            driver = key->CreatePropDriver();
            mPropDrivers.insert(position, driver);
        } else {
            driver = *position;
        }
        driver->Add(info, key, mEntity);
    }
    return GetInstID(*info);
}

// Reconstructed from eboot.elf at 0x2FCFC0.
void PropAnimDriver::_OnStopAnim(PlaybackInfo& info) {
    for (PropDriver** it = mPropDrivers.begin(); it != mPropDrivers.end();) {
        PropDriver* const driver = *it;
        driver->Remove(&info);
        if (driver->GetNumPropKeysComs() == 0) {
            delete driver;
            mPropDrivers.erase(it);
        } else {
            ++it;
        }
    }
}

// Reconstructed from eboot.elf at 0x2FD070.
PropDriver* PropAnimDriver::_FindPropDriver(const PropRef& prop) {
    for (PropDriver* driver : mPropDrivers) {
        if (prop == driver->mProp) {
            return driver;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x2FD1F0.
float AnimDriver::GetDefaultFramesPerUnit(TimeUnits units) {
    static const float kFramesPerUnit[] = {30.0f, 16.0f, 30.0f, 30.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    return kFramesPerUnit[units];
}

// Reconstructed from eboot.elf at 0x2FD200.
int AnimDriver::GetBaseUnits(int units) {
    return units < 4 ? units : units - 4;
}
