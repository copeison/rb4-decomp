// anim/CharAnim.o (0x32CFA0 to 0x331A4F). The builder Create (0x32D4B0) and
// the helpers only it calls (the facing computation at 0x32FA50 and
// 0x32FC10, the raw sampling at 0x330B80, the sorts and list formatting at
// 0x3302C0 and 0x331000-0x331482, 0x331590) are not reconstructed; see
// docs/anim-characters.md.
#include "anim/char/CharAnim.h"

#include "anim/char/CharAnimBuffer.h"
#include "anim/char/CharAnimRaw.h"
#include "anim/char/CharBoneMaskCom.h"
#include "utl/text/Str.h"

const unsigned long CharAnim::kRev = 10;

// Reconstructed from eboot.elf at 0x32CFA0. The constructor is inlined.
CharAnim* CharAnim::New() {
    return new CharAnim();
}

// Reconstructed from eboot.elf at 0x32D030.
CharAnim::CharAnim()
    : mFramesPerSecond(30.0f), mBeatTrack(), mKeyframes(), mBlendMode(AnimResource::kBlendAdditive), mSubTreeIndex(0) {}

// Reconstructed from eboot.elf at 0x32D0B0.
CharAnim::~CharAnim() {}

// Reconstructed from eboot.elf at 0x32D120.
bool CharAnim::Load(BinStream& stream, bool cached) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 10) {
        return false;
    }
    stream.ReadEndian(&mFramesPerSecond, 4);
    ReadKeyTrack(stream, mBeatTrack);
    int value = 0;
    stream.ReadEndian(&value, 4);
    mBlendMode = static_cast<AnimResource::BlendMode>(value);
    stream.ReadEndian(&value, 4);
    mSubTreeIndex = static_cast<unsigned int>(value);
    return mKeyframes.Load(stream, cached);
}

// Reconstructed from eboot.elf at 0x32D230.
void CharAnim::Save(BinStream& stream, bool cached) const {
    int value = 10;
    stream.WriteEndian(&value, 4);
    stream.WriteEndian(&mFramesPerSecond, 4);
    stream << mBeatTrack;
    value = mBlendMode;
    stream.WriteEndian(&value, 4);
    value = static_cast<int>(mSubTreeIndex);
    stream.WriteEndian(&value, 4);
    mKeyframes.Save(stream, cached);
}

// Reconstructed from eboot.elf at 0x32D300.
BinStream& operator<<(BinStream& stream, const TKeyTrack<float>& track) {
    int value = 1;
    stream.WriteEndian(&value, 4);
    value = track.mInterpolation;
    stream.WriteEndian(&value, 4);
    value = 1;
    stream.WriteEndian(&value, 4);
    const unsigned int count = track.mKeys.size();
    stream.WriteEndian(&count, 4);
    for (unsigned long i = 0; i < count; ++i) {
        const TKey<float>& key = track.mKeys[i];
        stream.WriteEndian(&key.mFrame, 4);
        stream.WriteEndian(&key.mValue, 4);
    }
    return stream;
}

// Reconstructed from eboot.elf at 0x32D450.
bool CharAnim::Create(const CharBoneMaskCom* mask, CharAnimRaw* raw, CharAnimRaw* relative, ResourcePath path) {
    return Create(
        mask->GetSkeletonResource(),
        mask->mBoneChannelMasks,
        mask->mConstraints,
        static_cast<AnimResource::BlendMode>(mask->mBlendMode),
        mask->mSubtreeIndex,
        mask->mBlendFromRoot,
        raw,
        relative,
        path);
}

// Reconstructed from eboot.elf at 0x32F980.
bool CharAnim::Create(
    CharSkeletonResource* skeleton,
    const PropArray<CharBoneChannelMask>& masks,
    AnimResource::BlendMode mode,
    unsigned long subTree,
    bool blendFromRoot,
    CharAnimRaw* raw,
    CharAnimRaw* relative,
    ResourcePath path) {
    const PropArray<CharBoneMaskCom::Constraint> constraints;
    return Create(skeleton, masks, constraints, mode, subTree, blendFromRoot, raw, relative, path);
}

// Reconstructed from eboot.elf at 0x330450.
void CharAnim::Apply(
    float weight,
    float frame,
    float delta,
    bool wrapped,
    AnimResource::BlendMode mode,
    CharAnimBuffer& buffer) const {
    mKeyframes.Apply(weight, frame, delta, wrapped, mode, buffer);
}

// Reconstructed from eboot.elf at 0x330460.
void CharAnim::ScaleDown(float weight, CharAnimBuffer& buffer) const {
    mKeyframes.ScaleDown(weight, buffer);
}

// Reconstructed from eboot.elf at 0x330470. Keys at the same beat resolve
// to the last of them; outside the keys the end segments extrapolate.
float CharAnim::GetFrameFromBeat(float beat) const {
    const unsigned long count = mBeatTrack.mKeys.size();
    if (count <= 1) {
        return beat * 16.0f;
    }
    const TKey<float>* const keys = mBeatTrack.mKeys.data();
    long index = -1;
    if (beat >= keys[0].mValue) {
        unsigned long low = 0;
        unsigned long high = count;
        while (high > low + 1) {
            const unsigned long middle = (low + high) >> 1;
            if (beat >= keys[middle].mValue) {
                low = middle;
            } else {
                high = middle;
            }
        }
        index = static_cast<long>(low);
        while (static_cast<unsigned long>(index) + 1 < count && keys[index + 1].mValue == keys[index].mValue) {
            ++index;
        }
    }
    const long last = static_cast<long>(count) - 2;
    unsigned long segment = index < 0 ? 0 : index;
    if (index > last) {
        segment = last;
    }
    const TKey<float>& from = keys[segment];
    const TKey<float>& to = keys[segment + 1];
    const float beats = to.mValue - from.mValue;
    if (beats == 0.0f) {
        return from.mFrame;
    }
    return (to.mFrame - from.mFrame) * ((beat - from.mValue) / beats) + from.mFrame;
}

// Reconstructed from eboot.elf at 0x330540. A frame before the first key
// takes the last segment, as the binary's unsigned clamp does.
float CharAnim::GetBeatFromFrame(float frame) const {
    const unsigned long count = mBeatTrack.mKeys.size();
    if (count <= 1) {
        return frame * 0.0625f;
    }
    const TKey<float>* const keys = mBeatTrack.mKeys.data();
    unsigned long first = 0;
    unsigned long remaining = count;
    while (remaining != 0) {
        const unsigned long half = remaining >> 1;
        if (frame < keys[first + half].mFrame) {
            remaining = half;
        } else {
            first += half + 1;
            remaining -= half + 1;
        }
    }
    unsigned long segment = first - 1;
    if (segment >= count) {
        segment = static_cast<unsigned long>(-1);
    }
    if (segment > count - 2) {
        segment = count - 2;
    }
    const TKey<float>& from = keys[segment];
    const TKey<float>& to = keys[segment + 1];
    return (frame - from.mFrame) / (to.mFrame - from.mFrame) * (to.mValue - from.mValue) + from.mValue;
}

namespace {

// UnitsToString's table (0x18FA3D0).
const char* const kUnitNames[CharAnim::kNumUnits + 1] = {
    "realtime", "beat", "beatAlign1", "beatAlign4", "beatAlign2", "numUnits"};

}  // namespace

// Reconstructed from eboot.elf at 0x3305F0.
const char* CharAnim::UnitsToString(Units units) {
    return kUnitNames[units];
}

// Reconstructed from eboot.elf at 0x330600.
eastl::vector<AllowedValue<int>> CharAnim::GetRelativeAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(3);
    values.push_back(AllowedValue<int>{0, String("Not Relative"), String("Played back normally, not additively or relative")});
    values.push_back(AllowedValue<int>{1, String("Bind Relative"), String("Played back additively relative to the bind pose")});
    values.push_back(AllowedValue<int>{
        2,
        String("Other Relative"),
        String("Played back additively relative to the first frame of a different (or the same) animation")});
    return values;
}

// Reconstructed from eboot.elf at 0x330830.
eastl::vector<AllowedValue<int>> CharAnim::GetUnitsAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(5);
    values.push_back(AllowedValue<int>{kRealTime, String("Real-time"), String("Play back in seconds, not beats, the default")});
    values.push_back(AllowedValue<int>{kBeat, String("Beat"), String("Play with the beat, but not aligned to the beat")});
    values.push_back(AllowedValue<int>{kBeatAlign1, String("Beat Aligned"), String("Play aligned to the beat")});
    values.push_back(AllowedValue<int>{kBeatAlign2, String("Two Beat Aligned"), String("Play aligned to every other beat")});
    values.push_back(AllowedValue<int>{kBeatAlign4, String("Downbeat Aligned"), String("Play aligned to the downbeat")});
    return values;
}

// Reconstructed from eboot.elf at 0x331490.
BinStream& operator>>(BinStream& stream, PropArray<TKey<float>>& keys) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 0) {
        unsigned int count = 0;
        stream.ReadEndian(&count, 4);
        keys.Resize(count);
        for (unsigned int i = 0; i < count; ++i) {
            TKey<float>& key = keys[i];
            stream.ReadEndian(&key.mFrame, 4);
            stream.ReadEndian(&key.mValue, 4);
        }
    } else {
        unsigned int count = 0;
        stream.ReadEndian(&count, 4);
        int unused = 0;
        stream.ReadEndian(&unused, 4);
        keys.Resize(count);
    }
    return stream;
}
