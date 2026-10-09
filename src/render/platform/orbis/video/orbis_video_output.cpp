#include "render/platform/orbis/video/orbis_video_output.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <kernel/equeue.h>
#include <system_service.h>
#include <video_out.h>

#include "os/memory/MemMgr.h"
#include "utl/threading/Thread.h"
#include "utl/time/Timer.h"
#include "render/system/RndWindow.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4Factory.h"
#include "renderps4/textures/PS4Texture2D.h"
#include "render/platform/orbis/video/orbis_back_buffer.h"
#include "renderps4/video/PS4Window.h"

extern "C" {

std::int32_t sceGnmAddEqEvent(
    SceKernelEqueue queue,
    std::uint32_t event_id,
    void* user_data);
std::int32_t sceGnmDeleteEqEvent(
    SceKernelEqueue queue,
    std::uint32_t event_id);
std::int32_t sceGnmSubmitDone();

}

namespace rb4 {

namespace {

constexpr const char* kEventQueueName = "EOP QUEUE";
constexpr const char* kSubmitThreadName = "SubmitDoneThread";
constexpr std::uint32_t kGnmEventId = 64;
constexpr auto kSubmitThreadPriority = static_cast<Thread::ThreadPriority>(699);
constexpr float kSubmitDoneTimeoutMilliseconds = 1000.0F;
constexpr std::size_t kBackBufferCount = 2;
constexpr std::int64_t kInitialPreviousBuffer = 2;

struct OrbisSubmitWorkerState {
    std::size_t next_buffer = 0;
    std::int64_t previous_buffer = kInitialPreviousBuffer;
    std::uint64_t last_submit_check = 0;
    std::uint64_t pending_submit_ticks = 0;
};

template <typename Callback>
void for_each_output_texture(
    PS4Device& system,
    Callback callback) {
    auto* frame_owner = system.mMainWindow;
    const auto states = frame_owner->GetBufferCollections();
    for (std::size_t index = 0; index < states.mCount; ++index) {
        auto* texture = states.mCollections[index]->mBackBuffer;
        if (texture != nullptr) {
            callback(reinterpret_cast<PS4Texture2D&>(*texture));
        }
    }
}

std::uint32_t video_flip_mode(std::int32_t rate) {
    return rate == 0
        ? SCE_VIDEO_OUT_FLIP_MODE_HSYNC
        : SCE_VIDEO_OUT_FLIP_MODE_WINDOW_2;
}

}  // namespace

void orbis_video_output_open(PS4Device& system) {
    constexpr std::int32_t kSystemUserId = 255;
    const auto handle = sceVideoOutOpen(kSystemUserId, 0, 0, nullptr);
    system.mVideoOutHandle = handle;
}

void orbis_video_output_set_flip_rate(
    PS4Device& system,
    std::uint32_t rate) {
    sceVideoOutSetFlipRate(
        system.mVideoOutHandle, static_cast<std::int32_t>(rate));
}

void orbis_video_output_set_window_margins(
    PS4Device& system,
    std::uint32_t top,
    std::uint32_t bottom) {
    sceVideoOutSetWindowModeMargins(
        system.mVideoOutHandle,
        static_cast<int>(top),
        static_cast<int>(bottom));
}

void orbis_create_event_queue(
    PS4Device& system,
    const char* name) {
    SceKernelEqueue queue = nullptr;
    sceKernelCreateEqueue(&queue, name);
    system.mEventQueue = queue;
}

void orbis_register_gnm_event(
    PS4Device& system,
    std::uint32_t event_id) {
    sceGnmAddEqEvent(system.mEventQueue, event_id, nullptr);
}

void orbis_register_video_flip_event(PS4Device& system) {
    sceVideoOutAddFlipEvent(
        system.mEventQueue,
        system.mVideoOutHandle,
        nullptr);
}

void orbis_unregister_gnm_event(
    PS4Device& system,
    std::uint32_t event_id) {
    sceGnmDeleteEqEvent(system.mEventQueue, event_id);
}

void orbis_delete_event_queue(PS4Device& system) {
    sceKernelDeleteEqueue(system.mEventQueue);
}

void orbis_video_output_close(PS4Device& system) {
    sceVideoOutClose(system.mVideoOutHandle);
}

void orbis_hide_system_splash_screen() {
    sceSystemServiceHideSplashScreen();
}

bool orbis_wait_for_submit_events(
    PS4Device& system,
    OrbisSubmitEvent* events,
    std::size_t capacity,
    std::size_t& event_count) {
    constexpr SceKernelUseconds kWaitTimeoutMicroseconds = 1'000'000;
    std::array<SceKernelEvent, 4> kernel_events{};
    auto timeout = kWaitTimeoutMicroseconds;
    int kernel_event_count = 0;
    const auto wait_capacity = static_cast<int>(
        std::min(capacity, kernel_events.size()));
    const auto result = sceKernelWaitEqueue(
        system.mEventQueue,
        kernel_events.data(),
        wait_capacity,
        &kernel_event_count,
        &timeout);
    if (result != 0) {
        event_count = 0;
        return false;
    }

    event_count = 0;
    for (int index = 0; index < kernel_event_count; ++index) {
        const auto filter = sceKernelGetEventFilter(&kernel_events[index]);
        if (filter == SCE_KERNEL_EVFILT_VIDEO_OUT) {
            events[event_count++].type =
                OrbisSubmitEventType::kFlipComplete;
        } else if (filter == SCE_KERNEL_EVFILT_GNM) {
            events[event_count++].type = OrbisSubmitEventType::kEndOfPipe;
        }
    }
    return true;
}

void orbis_process_submit_timeout(
    PS4Device& system,
    OrbisSubmitWorkerState& state) {
    scePthreadMutexLock(&system.mSubmitCritSec.mCritSec);
    ++system.mSubmitCritSec.mEntryCount;
    state.last_submit_check = Hmx::Timer::GetCycleCounter();
    sceGnmSubmitDone();
    state.pending_submit_ticks = 0;
    --system.mSubmitCritSec.mEntryCount;
    scePthreadMutexUnlock(&system.mSubmitCritSec.mCritSec);
}

void orbis_process_flip_complete(PS4Device& system) {
    SceVideoOutFlipStatus status{};
    sceVideoOutGetFlipStatus(system.mVideoOutHandle, &status);

    const auto completed_buffer =
        static_cast<std::uint64_t>(status.flipArg);
    if (completed_buffer >= kBackBufferCount) {
        return;
    }

    for_each_output_texture(
        system,
        [completed_buffer](PS4Texture2D& texture) {
            texture.CompletePendingPresentation(
                static_cast<unsigned long>(completed_buffer));
        });
}

void orbis_process_end_of_pipe(
    PS4Device& system,
    OrbisSubmitWorkerState& state) {
    scePthreadMutexLock(&system.mSubmitCritSec.mCritSec);
    ++system.mSubmitCritSec.mEntryCount;

    auto& context = system.Context();
    bool submit_done = orbis_render_context_frame_submissions_complete(
        context, state.next_buffer);
    if (!submit_done) {
        const auto now = Hmx::Timer::GetCycleCounter();
        state.pending_submit_ticks += now - state.last_submit_check;
        state.last_submit_check = now;
        submit_done = static_cast<float>(
            Hmx::Timer::CyclesToMs(
                state.pending_submit_ticks)) >=
            kSubmitDoneTimeoutMilliseconds;
    }

    if (submit_done) {
        state.last_submit_check = Hmx::Timer::GetCycleCounter();
        sceGnmSubmitDone();
        state.pending_submit_ticks = 0;
    }

    for_each_output_texture(
        system,
        [&state](PS4Texture2D& texture) {
            texture.AddPendingPresentation(state.next_buffer);
        });

    system.mSubmitToken = 1;
    --system.mSubmitCritSec.mEntryCount;
    scePthreadMutexUnlock(&system.mSubmitCritSec.mCritSec);
    system.mSubmitCondition.Signal();

    const auto rate = system.mSettings->ActiveVSyncMode();
    if (rate != system.mFlipRate) {
        system.mFlipRate = rate;
        orbis_video_output_set_flip_rate(system, rate == 2);
    }

    sceVideoOutSubmitFlip(
        system.mVideoOutHandle,
        static_cast<std::int32_t>(state.next_buffer),
        video_flip_mode(rate),
        state.previous_buffer);
    state.previous_buffer = static_cast<std::int64_t>(state.next_buffer);
    state.next_buffer = (state.next_buffer + 1) % kBackBufferCount;
}

}  // namespace rb4

using namespace rb4;

// Reconstructed from eboot.elf at 0x8D7B20.
void PS4Device::_InitImpl(const RndInitParams*) {
    auto& system = *this;
    orbis_video_output_open(system);
    orbis_video_output_set_flip_rate(system, 0);
    orbis_video_output_set_window_margins(system, 1080, 0);

    orbis_create_event_queue(system, kEventQueueName);
    orbis_register_gnm_event(system, kGnmEventId);
    orbis_register_video_flip_event(system);

    _InitDefaultVertexBuffers();
    _InitIdentityInstanceBuffers();
    _InstallFactory(new PS4Factory);
    _InstallMainWindow(new PS4Window);
    orbis_create_render_context(system);

    system.mSubmitCondition.Init(system.mSubmitCritSec);
    system.mSubmitThread.Create(
        orbis_submit_done_thread_entry,
        &system,
        kSubmitThreadName,
        -1,
        kSubmitThreadPriority,
        0,
        0);
    system.mSubmitThreadRunning = true;
    system.mSubmitToken = 0;
    system.mSubmitThread.mThread.Start();
    orbis_wait_for_submit_thread(system);
    orbis_hide_system_splash_screen();
}

// Reconstructed from eboot.elf at 0x8D8040.
void PS4Device::_TerminateImpl() {
    auto& system = *this;
    mSubmitThreadRunning = false;
    mSubmitThread.mThread._Join();
    mSubmitCondition.Destroy();
    _DestroyMainWindow();
    _DestroyContexts();
    orbis_unregister_gnm_event(system, kGnmEventId);
    orbis_delete_event_queue(system);
    orbis_video_output_close(system);
}

namespace rb4 {

void orbis_create_render_context(PS4Device& system) {
    static_cast<void>(orbis_render_context_create(system));
}

void orbis_wait_for_submit_thread(PS4Device& system) {
    scePthreadMutexLock(&system.mSubmitCritSec.mCritSec);
    ++system.mSubmitCritSec.mEntryCount;
    while (system.mSubmitToken == 0) {
        system.mSubmitCondition.Wait();
    }
    --system.mSubmitCritSec.mEntryCount;
    scePthreadMutexUnlock(&system.mSubmitCritSec.mCritSec);
}

// Reconstructed from eboot.elf at 0x8D77E0.
std::int32_t orbis_submit_done_thread_entry(void* context) {
    auto& system = *static_cast<PS4Device*>(context);
    orbis_submit_done_thread_run(system);
    return 0;
}

// Reconstructed from eboot.elf at 0x8D7340.
void orbis_submit_done_thread_run(PS4Device& system) {
    system.Lock();
    if (system.mBeginFramePending) {
        system._FlushPendingBeginFrame();
    }
    system.Unlock();

    scePthreadMutexLock(&system.mSubmitCritSec.mCritSec);
    system.mSubmitToken = 1;
    scePthreadMutexUnlock(&system.mSubmitCritSec.mCritSec);
    system.mSubmitCondition.Signal();

    OrbisSubmitWorkerState state;
    state.last_submit_check = Hmx::Timer::GetCycleCounter();
    std::array<OrbisSubmitEvent, 4> events{};
    while (system.mSubmitThreadRunning) {
        std::size_t event_count = 0;
        if (!orbis_wait_for_submit_events(
                system, events.data(), events.size(), event_count)) {
            orbis_process_submit_timeout(system, state);
            continue;
        }

        for (std::size_t index = 0; index < event_count; ++index) {
            switch (events[index].type) {
            case OrbisSubmitEventType::kFlipComplete:
                orbis_process_flip_complete(system);
                break;
            case OrbisSubmitEventType::kEndOfPipe:
                orbis_process_end_of_pipe(system, state);
                break;
            }
        }
    }
}

}  // namespace rb4
