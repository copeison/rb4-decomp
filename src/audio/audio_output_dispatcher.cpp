#include "audio_output_dispatcher.h"

namespace rb4 {

namespace {

void promote_pending_listeners(AudioOutputDispatcher& dispatcher) {
    std::lock_guard<std::recursive_mutex> lock(dispatcher.pending_mutex);
    ++dispatcher.pending_dispatch_depth;
    dispatcher.listeners.splice(
        dispatcher.listeners.end(), dispatcher.pending_listeners);
    --dispatcher.pending_dispatch_depth;
}

void prepare_output_block(
    AudioOutputDispatcher& dispatcher,
    std::uint32_t mix_sequence,
    std::uint32_t block_index,
    bool final_block) {
    const float sample_rate = static_cast<float>(dispatcher.sample_rate);
    for (auto* listener : dispatcher.listeners) {
        listener->prepare_output_block(
            kAudioOutputBlockSamples,
            mix_sequence,
            block_index,
            final_block,
            sample_rate);
    }
}

void process_output_block(
    AudioOutputDispatcher& dispatcher,
    std::uint32_t mix_sequence,
    std::uint32_t block_index,
    bool final_block) {
    for (auto* listener : dispatcher.listeners) {
        listener->process_guard.store(0);
    }

    const float sample_rate = static_cast<float>(dispatcher.sample_rate);
    for (auto* listener : dispatcher.listeners) {
        if (listener->process_guard.load() <= 0 &&
            listener->process_guard.fetch_add(1) == 0) {
            listener->process_output_block(
                kAudioOutputBlockSamples,
                mix_sequence,
                block_index,
                final_block,
                sample_rate);
        }
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x1127B90.
void audio_output_dispatcher_set_sample_rate(
    AudioOutputDispatcher& dispatcher,
    std::int32_t sample_rate) {
    dispatcher.sample_rate = sample_rate;
}

// Reconstructed from eboot.elf at 0x1127880.
void audio_dispatch_output_blocks(
    AudioOutputDispatcher& dispatcher,
    std::uint32_t buffer_length,
    std::uint32_t mix_sequence) {
    std::lock_guard<std::recursive_mutex> lock(dispatcher.dispatch_mutex);
    ++dispatcher.dispatch_depth;
    promote_pending_listeners(dispatcher);

    std::uint32_t remaining_samples = buffer_length;
    std::uint32_t block_index = 0;
    while (remaining_samples != 0) {
        const bool final_block =
            remaining_samples == kAudioOutputBlockSamples;
        prepare_output_block(
            dispatcher, mix_sequence, block_index, final_block);

        dispatcher.current_block = {
            mix_sequence,
            block_index,
            final_block,
        };
        process_output_block(
            dispatcher, mix_sequence, block_index, final_block);

        ++block_index;
        remaining_samples -= kAudioOutputBlockSamples;
    }

    --dispatcher.dispatch_depth;
}

}  // namespace rb4
