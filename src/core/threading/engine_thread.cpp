#include "core/threading/engine_thread.h"

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <sys/sched.h>

#include "core/threading/engine_thread_adapters.h"
#include "core/threading/thread_affinity.h"

namespace rb4 {

namespace {

void* engine_thread_trampoline(void* argument) {
    auto& invocation =
        *static_cast<EngineThreadInvocation*>(argument);
    invocation.result = invocation.callback(invocation.context);
    return reinterpret_cast<void*>(
        static_cast<std::intptr_t>(invocation.result));
}

std::int32_t engine_thread_wrapper_entry(void* argument) {
    auto& invocation =
        *static_cast<EngineThreadWrapperInvocation*>(argument);
    engine_thread_register_current(invocation.owner->runtime.name);
    invocation.result = invocation.callback(invocation.context);
    return invocation.result;
}

}  // namespace

// Reconstructed from eboot.elf at 0x25C3E0.
void engine_thread_configure_runtime(
    EngineThreadRuntime& runtime,
    EngineThreadCallback callback,
    void* context,
    const char* name,
    std::int64_t processor,
    std::int32_t priority,
    std::uint32_t stack_size,
    SceKernelCpumask affinity_mask) {
    runtime.invocation.callback = callback;
    runtime.invocation.context = context;
    runtime.invocation.result = 0;
    runtime.trailing_state = 0;
    runtime.processor = processor;
    runtime.priority = priority;
    runtime.stack_size =
        std::max(stack_size, kMinimumEngineThreadStackSize);
    runtime.affinity_mask = affinity_mask;
    std::snprintf(runtime.name, sizeof(runtime.name), "%s", name);
}

// Reconstructed from eboot.elf at 0x259210 and 0x2593A0.
void engine_thread_configure(
    EngineThread& thread,
    EngineThreadCallback callback,
    void* context,
    const char* name,
    std::int64_t processor,
    std::int32_t priority,
    std::uint32_t stack_size,
    SceKernelCpumask affinity_mask) {
    thread.invocation.callback = callback;
    thread.invocation.context = context;
    thread.invocation.result = 0;
    thread.invocation.owner = &thread;

    affinity_mask = thread_affinity_build_cpu_mask(
        processor, affinity_mask);

    engine_thread_configure_runtime(
        thread.runtime,
        engine_thread_wrapper_entry,
        &thread.invocation,
        name,
        processor,
        priority,
        stack_size,
        affinity_mask);
}

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
