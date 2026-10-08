#pragma once

#include <cstdint>

namespace rb4 {

struct AudioMixFormat {
    double sample_rate = 0.0;
    double seconds_per_sample = 0.0;
    std::int32_t buffer_length = 0;
    float buffers_per_second = 0.0F;
    double milliseconds_per_buffer = 0.0;
};

extern AudioMixFormat g_audio_mix_format;

double audio_get_sample_rate();
void audio_set_mix_format(double sample_rate, std::int32_t buffer_length);

}  // namespace rb4
