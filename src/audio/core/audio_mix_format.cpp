#include "audio/core/audio_mix_format.h"

namespace rb4 {

AudioMixFormat g_audio_mix_format;

// Reconstructed from eboot.elf at 0xD3BA0.
double audio_get_sample_rate() {
    return g_audio_mix_format.sample_rate;
}

// Reconstructed from eboot.elf at 0xD3BC0.
void audio_set_mix_format(
    double sample_rate,
    std::int32_t buffer_length) {
    g_audio_mix_format.sample_rate = sample_rate;
    g_audio_mix_format.seconds_per_sample = 1.0 / sample_rate;
    g_audio_mix_format.buffer_length = buffer_length;
    g_audio_mix_format.buffers_per_second =
        static_cast<float>(sample_rate) / static_cast<float>(buffer_length);
    g_audio_mix_format.milliseconds_per_buffer =
        1000.0 / static_cast<double>(g_audio_mix_format.buffers_per_second);
}

}  // namespace rb4
