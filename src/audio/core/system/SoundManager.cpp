#include "audio/core/generators/AudioGenerator.h"

// Only the generator lookups of audio/SoundManager.o are reconstructed.

namespace {

// Handle fields as AudioGenerator::GetNewHandle packs them. Names not in the
// reference map.
constexpr unsigned int kHandleManagerShift = 24;
constexpr unsigned int kHandleManagerMask = 0x7F;
constexpr unsigned int kHandleIndexShift = 14;
constexpr unsigned int kHandleIndexMask = 0x3FF;

}  // namespace

// Reconstructed from eboot.elf at 0x5FB0. Zero and all-ones are never live
// handles.
AudioGenerator* SoundManager::LockIfOwned(unsigned int handle) {
    if (handle + 1 < 2) {
        return nullptr;
    }
    int managerIndex = static_cast<int>((handle >> kHandleManagerShift) & kHandleManagerMask);
    if (managerIndex >= static_cast<int>(mManagers.size())) {
        return nullptr;
    }
    return mManagers[managerIndex]->LockIfOwned(handle, (handle >> kHandleIndexShift) & kHandleIndexMask);
}

// Reconstructed from eboot.elf at 0x8090.
AudioGeneratorManager* SoundManager::_GetManager(Symbol id) {
    for (AudioGeneratorManager* manager : mManagers) {
        if (manager->GetId() == id) {
            return manager;
        }
    }
    return nullptr;
}
