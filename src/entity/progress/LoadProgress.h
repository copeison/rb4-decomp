#pragma once

#include "os/threading/CritSec.h"
#include "utl/containers/List.h"

// Receives the progress of long tasks: entity loads ("EntityLoad"),
// character animation loads and the renderer's generated textures
// ("SkinDiffusion", "HairReflectance", "LightProbeCaptureAll"). The
// object (0x12D640-0x12DBD6) sits between the entity module's PollSort
// and PropInfo objects and is not in the reference map, which has none of
// these names; all names here are inferred. Nothing in this build
// registers a listener. A listener returns true to keep the report from
// the listeners after it.
class LoadProgressListener {
public:
    // Slot 0: a task starts with `total` steps, of which `done` are done.
    virtual bool OnBegin(const char* task, const char* description, unsigned int total, unsigned int done) = 0;
    // Slot 1: the task reached the step.
    virtual bool OnSetProgress(const char* task, unsigned int step) = 0;
    // Slot 2: the task advanced by `steps`.
    virtual bool OnStep(const char* task, unsigned int steps) = 0;
    // Slot 3: the task ended.
    virtual bool OnEnd(const char* task) = 0;
    // Slot 4: a message about the task. No sender remains, so the
    // argument types are weakly supported.
    virtual bool OnMessage(const char* task, const char* message) = 0;
};

// The listeners, in registration order, and their lock, at 0x19E3CC0 and
// 0x19E3CE0. The size doubles as the check whether progress is reported.
extern eastl::list<LoadProgressListener*> gLoadProgressListeners;
extern CritSec gLoadProgressCrit;

// Whether any listener is registered; callers skip their reports
// otherwise. Inlined into every caller.
inline bool HasLoadProgressListeners() {
    return gLoadProgressListeners.size() != 0;
}

void AddLoadProgressListener(LoadProgressListener* listener);     // 0x12D640
void RemoveLoadProgressListener(LoadProgressListener* listener);  // 0x12D6C0
void ClearLoadProgressListeners();                                // 0x12D760

// Report to the listeners in turn through slots 0-4.
void LoadProgressBegin(const char* task, const char* description, unsigned int total, unsigned int done);  // 0x12D7F0
void LoadProgressSet(const char* task, unsigned int step);   // 0x12D890
void LoadProgressStep(const char* task, unsigned int steps);  // 0x12D930
void LoadProgressEnd(const char* task);                       // 0x12D9D0
void LoadProgressMessage(const char* task, const char* message);  // 0x12DA50
