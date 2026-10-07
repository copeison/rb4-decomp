#pragma once

#include <atomic>
#include <cstdint>
#include <list>
#include <mutex>

namespace rb4 {

constexpr std::int32_t kAudioOutputBlockSamples = 128;

class AudioOutputListener {
public:
    virtual ~AudioOutputListener() = default;

    virtual void prepare_output_block(
        std::int32_t sample_count,
        std::uint32_t mix_sequence,
        std::uint32_t block_index,
        bool final_block,
        float sample_rate) = 0;

    virtual void process_output_block(
        std::int32_t sample_count,
        std::uint32_t mix_sequence,
        std::uint32_t block_index,
        bool final_block,
        float sample_rate) = 0;

    std::atomic<std::int32_t> process_guard{0};
};

struct AudioOutputBlockContext {
    std::uint32_t mix_sequence = 0;
    std::uint32_t block_index = 0;
    bool final_block = false;
};

// Semantic representation of the original mutex-protected EASTL lists.
struct AudioOutputDispatcher {
    std::recursive_mutex dispatch_mutex;
    std::int32_t dispatch_depth = 0;
    std::recursive_mutex pending_mutex;
    std::int32_t pending_dispatch_depth = 0;
    std::list<AudioOutputListener*> listeners;
    std::list<AudioOutputListener*> pending_listeners;
    std::int32_t sample_rate = 0;
    AudioOutputBlockContext current_block;
};

void audio_dispatch_output_blocks(
    AudioOutputDispatcher& dispatcher,
    std::uint32_t buffer_length,
    std::uint32_t mix_sequence);

}  // namespace rb4
