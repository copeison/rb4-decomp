#pragma once

#include <cstddef>
#include <cstdint>
#include <_pthread.h>

namespace rb4 {

using EngineThreadCallback = std::int32_t (*)(void* context);

struct EngineThreadInvocation {
    EngineThreadCallback callback;
    void* context;
    std::int32_t result;
    std::uint8_t reserved_20[4];
};

struct EngineThreadRuntime {
    ScePthread thread;
    std::int64_t processor;
    std::int32_t priority;
    std::uint32_t stack_size;
    SceKernelCpumask affinity_mask;
    char name[32];
    EngineThreadInvocation invocation;
    std::uint64_t trailing_state;
};

static_assert(sizeof(EngineThreadInvocation) == 24);
static_assert(sizeof(EngineThreadRuntime) == 96);
static_assert(offsetof(EngineThreadRuntime, priority) == 16);
static_assert(offsetof(EngineThreadRuntime, stack_size) == 20);
static_assert(offsetof(EngineThreadRuntime, affinity_mask) == 24);
static_assert(offsetof(EngineThreadRuntime, name) == 32);
static_assert(offsetof(EngineThreadRuntime, invocation) == 64);

void engine_thread_start(EngineThreadRuntime& runtime);
std::int32_t engine_thread_join(EngineThreadRuntime& runtime);

}  // namespace rb4
