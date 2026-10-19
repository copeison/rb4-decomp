// entity/Waveform.o (part): baking the float curves into samples.
#include "entity/waveform/Waveform.h"

// Reconstructed from eboot.elf at 0x1CE140. The last sample repeats the one
// before it.
template <>
void WaveformResource<float>::GenerateTable(unsigned long curve) {
    eastl::vector<float>& samples = mSamples[curve];
    if (mType == 3 || (curve == 0 && mType == 2)) {
        samples.resize(256);
        for (unsigned long i = 0; i != 255; ++i) {
            samples.mpBegin[i] = 0.0f;
            ValueTangent<float> value = {0.0f, 0.0f, 0.0f, true};
            mTracks[curve].SampleValueAtFrame(static_cast<float>(i) * (1.0f / 254.0f), value);
            samples.mpBegin[i] = value.mValue;
            const float sample = samples.mpBegin[i];
            samples.mpBegin[i] = mValueRange[1] < sample ? mValueRange[1] : (sample > mValueRange[0] ? sample : mValueRange[0]);
        }
        samples.mpEnd[-1] = samples.mpBegin[254];
    } else if (samples.mpBegin != samples.mpEnd) {
        samples.clear();
        samples.set_capacity(0);
    }
}
