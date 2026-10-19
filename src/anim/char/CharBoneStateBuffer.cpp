// anim/CharBoneStateBuffer.o (0x3362C0 to 0x3368F4).
#include "anim/char/CharBoneStateBuffer.h"

#include <string.h>

#include "math/rotation/Quat.h"
#include "math/vector/Vector3.h"

// Reconstructed from eboot.elf at 0x3362C0.
CharBoneStateBuffer::CharBoneStateBuffer() : mFormat(), mData(nullptr), mWeights(nullptr) {}

// Reconstructed from eboot.elf at 0x3362E0.
CharBoneStateBuffer::~CharBoneStateBuffer() {
    delete[] mData;
    mData = nullptr;
}

// Reconstructed from eboot.elf at 0x336310.
void CharBoneStateBuffer::Init(const CharBoneStateSetFormat& format) {
    delete[] mData;
    mData = nullptr;
    mFormat = format;
    const unsigned int dataSize = format.mDataEnds[CharBoneChannel::kNumTypes - 1];
    const unsigned long size = dataSize + format.mBoneNames.size() * sizeof(float);
    mData = new unsigned char[size];
    mWeights = reinterpret_cast<float*>(mData + dataSize);
    memset(mData, 0, size);
}

// Reconstructed from eboot.elf at 0x336490.
void CharBoneStateBuffer::ClearChannelData() const {
    memset(mData, 0, mFormat.mDataEnds[CharBoneChannel::kNumTypes - 1] + mFormat.mBoneNames.size() * sizeof(float));
}

namespace {

// Normalizes the scale-like channels from `first` to `last` stored as three
// floats at `data`. Inlined for the scales and the inverse parent scales.
// Name not in the reference map.
void NormalizeScales(Vector3* data, float* weights, unsigned int first, unsigned int last, bool additive) {
    for (unsigned int channel = first; channel < last; ++channel, ++data) {
        float& weight = weights[channel];
        if (weight == 0.0f) {
            continue;
        }
        if (additive && weight < 1.0f) {
            const float missing = 1.0f - weight;
            data->x += missing;
            data->y += missing;
            data->z += missing;
        } else {
            const float scale = 1.0f / weight;
            data->x *= scale;
            data->y *= scale;
            data->z *= scale;
        }
        weight = 1.0f;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x3364B0. The single-axis rotations are
// left as they are.
void CharBoneStateBuffer::NormalizeChannelData(bool additive) const {
    const unsigned int* const ends = mFormat.mBoneEnds;
    const unsigned int* const offsets = mFormat.mDataEnds;
    float* const weights = mWeights;

    Vector3* positions = reinterpret_cast<Vector3*>(mData);
    for (unsigned int channel = 0; channel < ends[CharBoneChannel::kPosition]; ++channel, ++positions) {
        float& weight = weights[channel];
        if (weight == 0.0f) {
            continue;
        }
        if (!additive) {
            const float scale = 1.0f / weight;
            positions->x *= scale;
            positions->y *= scale;
            positions->z *= scale;
        }
        weight = 1.0f;
    }
    NormalizeScales(
        reinterpret_cast<Vector3*>(mData + offsets[CharBoneChannel::kPosition]),
        weights,
        ends[CharBoneChannel::kPosition],
        ends[CharBoneChannel::kScale],
        additive);
    NormalizeScales(
        reinterpret_cast<Vector3*>(mData + offsets[CharBoneChannel::kScale]),
        weights,
        ends[CharBoneChannel::kScale],
        ends[CharBoneChannel::kInverseParentScale],
        additive);

    Hmx::Quat* rotation = reinterpret_cast<Hmx::Quat*>(mData + offsets[CharBoneChannel::kInverseParentScale]);
    for (unsigned int channel = ends[CharBoneChannel::kInverseParentScale]; channel < ends[CharBoneChannel::kRotation];
         ++channel, ++rotation) {
        float& weight = weights[channel];
        if (weight == 0.0f) {
            continue;
        }
        if (additive) {
            // The missing weight goes to the identity, on the quaternion's
            // own hemisphere.
            rotation->w += rotation->w < 0.0f ? weight - 1.0f : 1.0f - weight;
        }
        Normalize(*rotation, *rotation);
        weight = 1.0f;
    }
}
