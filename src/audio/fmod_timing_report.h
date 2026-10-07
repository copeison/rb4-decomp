#pragma once

#include <map>
#include <vector>

#include "fmod_mix_callback.h"

namespace rb4 {

struct FmodAudioState;

struct AudioTimingPercentages {
    double average = 0.0;
    double maximum = 0.0;
};

struct FmodAudioTimingReport {
    AudioTimingPercentages engine_mix;
    AudioTimingPercentages fmod_mix;
    AudioTimingPercentages buffer_set;
    std::map<AudioTimingKey, double> source_average;
    std::map<AudioTimingKey, double> source_maximum;
    std::vector<AudioTimingKey> source_keys;
};

void fmod_audio_consume_timing_report(
    FmodAudioState& state,
    FmodAudioTimingReport& report,
    bool append_source_keys = true);

}  // namespace rb4
