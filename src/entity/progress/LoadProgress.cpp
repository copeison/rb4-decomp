#include "entity/progress/LoadProgress.h"

// The object's static initializer (0x12DAF0) also sets the int at
// 0x19E3CB8 to -1; nothing else refers to it.

// The listener list at 0x19E3CC0.
eastl::list<LoadProgressListener*> gLoadProgressListeners;
// The lock at 0x19E3CE0.
CritSec gLoadProgressCrit;

// Reconstructed from eboot.elf at 0x12D640.
void AddLoadProgressListener(LoadProgressListener* listener) {
    ScopedCritSec lock(gLoadProgressCrit);
    gLoadProgressListeners.push_back(listener);
}

// Reconstructed from eboot.elf at 0x12D6C0.
void RemoveLoadProgressListener(LoadProgressListener* listener) {
    ScopedCritSec lock(gLoadProgressCrit);
    auto it = gLoadProgressListeners.begin();
    while (it != gLoadProgressListeners.end() && *it != listener) {
        ++it;
    }
    if (it != gLoadProgressListeners.end()) {
        gLoadProgressListeners.erase(it);
    }
}

// Reconstructed from eboot.elf at 0x12D760.
void ClearLoadProgressListeners() {
    ScopedCritSec lock(gLoadProgressCrit);
    gLoadProgressListeners.clear();
}

// Reconstructed from eboot.elf at 0x12D7F0.
void LoadProgressBegin(const char* task, const char* description, unsigned int total, unsigned int done) {
    ScopedCritSec lock(gLoadProgressCrit);
    for (LoadProgressListener* listener : gLoadProgressListeners) {
        if (listener->OnBegin(task, description, total, done)) {
            break;
        }
    }
}

// Reconstructed from eboot.elf at 0x12D890.
void LoadProgressSet(const char* task, unsigned int step) {
    ScopedCritSec lock(gLoadProgressCrit);
    for (LoadProgressListener* listener : gLoadProgressListeners) {
        if (listener->OnSetProgress(task, step)) {
            break;
        }
    }
}

// Reconstructed from eboot.elf at 0x12D930.
void LoadProgressStep(const char* task, unsigned int steps) {
    ScopedCritSec lock(gLoadProgressCrit);
    for (LoadProgressListener* listener : gLoadProgressListeners) {
        if (listener->OnStep(task, steps)) {
            break;
        }
    }
}

// Reconstructed from eboot.elf at 0x12D9D0.
void LoadProgressEnd(const char* task) {
    ScopedCritSec lock(gLoadProgressCrit);
    for (LoadProgressListener* listener : gLoadProgressListeners) {
        if (listener->OnEnd(task)) {
            break;
        }
    }
}

// Reconstructed from eboot.elf at 0x12DA50.
void LoadProgressMessage(const char* task, const char* message) {
    ScopedCritSec lock(gLoadProgressCrit);
    for (LoadProgressListener* listener : gLoadProgressListeners) {
        if (listener->OnMessage(task, message)) {
            break;
        }
    }
}
