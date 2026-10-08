#pragma once

#include <cstddef>
#include <cstdint>
#include <_pthread.h>

namespace rb4 {

using EngineThreadCallback = std::int32_t (*)(void* context);

constexpr std::size_t kEngineProcessorCount = 6;
constexpr std::uint32_t kMinimumEngineThreadStackSize = 0x20000;

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

struct EngineThread;

struct EngineThreadWrapperInvocation {
    EngineThreadCallback callback;
    void* context;
    std::int32_t result;
    std::uint8_t reserved_20[4];
    EngineThread* owner;
};

struct EngineThread {
    const void* vtable;
    EngineThreadRuntime runtime;
    EngineThreadWrapperInvocation invocation;
};

static_assert(sizeof(EngineThreadInvocation) == 24);
static_assert(sizeof(EngineThreadRuntime) == 96);
static_assert(offsetof(EngineThreadRuntime, priority) == 16);
static_assert(offsetof(EngineThreadRuntime, stack_size) == 20);
static_assert(offsetof(EngineThreadRuntime, affinity_mask) == 24);
static_assert(offsetof(EngineThreadRuntime, name) == 32);
static_assert(offsetof(EngineThreadRuntime, invocation) == 64);
static_assert(sizeof(EngineThreadWrapperInvocation) == 32);
static_assert(offsetof(EngineThreadWrapperInvocation, owner) == 24);
static_assert(sizeof(EngineThread) == 136);
static_assert(offsetof(EngineThread, runtime) == 8);
static_assert(offsetof(EngineThread, invocation) == 104);

void engine_thread_configure_runtime(
    EngineThreadRuntime& runtime,
    EngineThreadCallback callback,
    void* context,
    const char* name,
    std::int64_t processor,
    std::int32_t priority,
    std::uint32_t stack_size,
    SceKernelCpumask affinity_mask);
void engine_thread_configure(
    EngineThread& thread,
    EngineThreadCallback callback,
    void* context,
    const char* name,
    std::int64_t processor,
    std::int32_t priority,
    std::uint32_t stack_size,
    SceKernelCpumask affinity_mask);

void engine_thread_start(EngineThreadRuntime& runtime);
std::int32_t engine_thread_join(EngineThreadRuntime& runtime);

}  // namespace rb4
