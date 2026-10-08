#include "core/threading/engine_thread.h"

#include <cstdint>
#include <sys/sched.h>

namespace rb4 {

namespace {

void* engine_thread_trampoline(void* argument) {
    auto& invocation =
        *static_cast<EngineThreadInvocation*>(argument);
    invocation.result = invocation.callback(invocation.context);
    return reinterpret_cast<void*>(
        static_cast<std::intptr_t>(invocation.result));
}

}  // namespace

// Reconstructed from eboot.elf at 0x25C430.
void engine_thread_start(EngineThreadRuntime& runtime) {
    ScePthreadAttr attributes;
    scePthreadAttrInit(&attributes);
    scePthreadAttrSetdetachstate(&attributes, 0);
    if (runtime.stack_size != 0) {
        scePthreadAttrSetstacksize(&attributes, runtime.stack_size);
    }

    SceKernelSchedParam scheduling{runtime.priority};
    scePthreadAttrSetinheritsched(&attributes, 0);
    scePthreadAttrSetschedpolicy(&attributes, SCHED_RR);
    scePthreadAttrSetschedparam(&attributes, &scheduling);
    scePthreadAttrSetaffinity(&attributes, runtime.affinity_mask);
    scePthreadCreate(
        &runtime.thread,
        &attributes,
        engine_thread_trampoline,
        &runtime.invocation,
        runtime.name);
    scePthreadRename(runtime.thread, runtime.name);
    scePthreadAttrDestroy(&attributes);
}

// Reconstructed from eboot.elf at 0x25C530.
std::int32_t engine_thread_join(EngineThreadRuntime& runtime) {
    if (runtime.thread == nullptr) {
        return 0;
    }

    void* result = nullptr;
    scePthreadJoin(runtime.thread, &result);
    runtime.thread = nullptr;
    return runtime.invocation.result;
}

}  // namespace rb4
