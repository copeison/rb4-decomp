// The lip-sync data (0x336900 to 0x33A9AB; no object in the reference map).
// The XML loader and its node graph (0x3370C0-0x33A9AB) are not
// reconstructed; see docs/anim-characters.md.
#include "anim/char/CharLipsync.h"

#include "utl/streams/BinStream.h"

// Reconstructed from eboot.elf at 0x336900.
CharLipsync::CharLipsync() : mVisemes(), mTracks(), mFrameOffsets(), mData(nullptr) {}

// Reconstructed from eboot.elf at 0x336980.
CharLipsync::~CharLipsync() {
    operator delete(mData);
    mData = nullptr;
}

// Reconstructed from eboot.elf at 0x336A00. The frame offsets are read as
// raw bytes.
bool CharLipsync::Load(BinStream& stream) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 0) {
        return false;
    }
    stream.ReadEndian(&mFramesPerSecond, 4);
    unsigned int count = 0;
    stream.ReadEndian(&count, 4);
    mVisemes.resize(count);
    for (Symbol& viseme : mVisemes) {
        stream >> viseme;
    }
    stream.ReadEndian(&count, 4);
    mTracks.resize(count);
    for (Symbol& track : mTracks) {
        stream >> track;
    }
    stream.ReadEndian(&count, 4);
    mFrameOffsets.resize(static_cast<int>(count));
    stream.Read(mFrameOffsets.begin(), 4L * static_cast<int>(count));
    const unsigned int size = mFrameOffsets.end()[-1];
    mData = new unsigned char[size];
    stream.Read(mData, size);
    return true;
}

// Reconstructed from eboot.elf at 0x336BE0.
void CharLipsync::Save(BinStream& stream) const {
    int value = 0;
    stream.WriteEndian(&value, 4);
    stream.WriteEndian(&mFramesPerSecond, 4);
    value = static_cast<int>(mVisemes.size());
    stream.WriteEndian(&value, 4);
    for (const Symbol& viseme : mVisemes) {
        stream << viseme;
    }
    value = static_cast<int>(mTracks.size());
    stream.WriteEndian(&value, 4);
    for (const Symbol& track : mTracks) {
        stream << track;
    }
    value = static_cast<int>(mFrameOffsets.size());
    stream.WriteEndian(&value, 4);
    for (const unsigned int offset : mFrameOffsets) {
        stream.WriteEndian(&offset, 4);
    }
    stream.Write(mData, mFrameOffsets.end()[-1]);
}

// Reconstructed from eboot.elf at 0x336DA0.
CharLipsync::Playback::Playback() : mWeights(), mLipsync(nullptr), mTrack(0) {}

// Reconstructed from eboot.elf at 0x336DE0.
void CharLipsync::Playback::Set(const CharLipsync* lipsync, unsigned long track) {
    mLipsync = lipsync;
    mTrack = track;
    mWeights.resize(lipsync != nullptr ? lipsync->mVisemes.size() : 0);
    for (unsigned long i = 0; i < mWeights.size(); ++i) {
        mWeights[i] = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x337050.
unsigned long CharLipsync::Playback::FindTrackData(unsigned long frame, unsigned long& end) const {
    const unsigned int* const offsets = mLipsync->mFrameOffsets.begin();
    unsigned long position = offsets[frame];
    end = offsets[frame + 1];
    const unsigned char* const data = mLipsync->mData;
    for (unsigned long track = 0; track < mTrack; ++track) {
        while (position < end && data[position] != 0xFF) {
            position += 2;
        }
        if (position >= end) {
            return position;
        }
        ++position;
    }
    return position;
}

// Reconstructed from eboot.elf at 0x336E80. The frame after the time is
// the one blended towards; past the last frame the blend stops just short
// of it.
void CharLipsync::Playback::Poll(float time) {
    const CharLipsync* const lipsync = mLipsync;
    if (lipsync == nullptr) {
        return;
    }
    const unsigned long count = lipsync->mFrameOffsets.size();
    if (count - 1 < 2) {
        return;
    }
    const float position = time * lipsync->mFramesPerSecond;
    const long next = static_cast<long>(__builtin_ceilf(position));
    float t = 0.0f;
    unsigned long frame = 1;
    if (next > 0) {
        frame = count - 2;
        if (next < static_cast<long>(count - 1)) {
            t = position - static_cast<float>(next - 1);
            frame = next;
        } else {
            t = 0.9999999f;
        }
    }
    for (float* const weight : mWeights) {
        if (weight != nullptr) {
            *weight = 0.0f;
        }
    }
    const unsigned char* const data = lipsync->mData;
    unsigned long end = 0;
    for (unsigned long i = FindTrackData(frame - 1, end); i < end; i += 2) {
        const unsigned char viseme = data[i];
        if (viseme == 0xFF) {
            break;
        }
        float* const weight = mWeights[viseme];
        if (weight != nullptr) {
            *weight = data[i + 1] * (1.0f / 255.0f);
        }
    }
    for (unsigned long i = FindTrackData(frame, end); i < end; i += 2) {
        const unsigned char viseme = data[i];
        if (viseme == 0xFF) {
            break;
        }
        float* const weight = mWeights[viseme];
        if (weight != nullptr) {
            *weight = (data[i + 1] * (1.0f / 255.0f) - *weight) * t + *weight;
        }
    }
}
