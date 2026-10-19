// anim/CharBoneKeyframeSet.o (0x334DB0 to 0x3361CF).
#include "anim/char/CharBoneKeyframeSet.h"

#include "anim/char/CharAnimBuffer.h"
#include "anim/skeleton/CharSkeleton.h"
#include "math/rotation/Quat.h"
#include "utl/streams/BinStream.h"

// Reconstructed from eboot.elf at 0x334DB0.
CharBoneKeyframeSet::CharBoneKeyframeSet()
    : mFormat(), mNumKeyframes(0), mRelative(false), mKeyframes(nullptr), mRootXfms(nullptr) {}

// Reconstructed from eboot.elf at 0x334DE0.
CharBoneKeyframeSet::~CharBoneKeyframeSet() {
    delete[] mKeyframes;
    mKeyframes = nullptr;
    delete[] mRootXfms;
    mRootXfms = nullptr;
}

// Reconstructed from eboot.elf at 0x334E30. The old data is not freed.
void CharBoneKeyframeSet::Init(
    const CharBoneStateSetFormat& format,
    const unsigned char* keyframes,
    const QuatXfm* rootXfms,
    unsigned long numKeyframes,
    bool relative) {
    mFormat = format;
    mKeyframes = keyframes;
    mRootXfms = rootXfms;
    mRelative = relative;
    mNumKeyframes = static_cast<unsigned int>(numKeyframes);
}

// Reconstructed from eboot.elf at 0x334F70. The old data is not freed.
bool CharBoneKeyframeSet::Load(BinStream& stream, bool cached) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 2 || !mFormat.Load(stream, cached)) {
        return false;
    }
    stream.ReadEndian(&mNumKeyframes, 4);
    bool relative = false;
    stream.Read(&relative, 1);
    mRelative = relative;
    const unsigned int size = mNumKeyframes * mFormat.mDataEnds[CharBoneChannel::kNumTypes - 1];
    unsigned char* const keyframes = new unsigned char[size];
    stream.Read(keyframes, size);
    mKeyframes = keyframes;
    bool hasRootXfms = false;
    stream.Read(&hasRootXfms, 1);
    if (hasRootXfms) {
        QuatXfm* const rootXfms = new QuatXfm[mNumKeyframes];
        mRootXfms = rootXfms;
        stream.Read(rootXfms, sizeof(QuatXfm) * mNumKeyframes);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x3350E0.
void CharBoneKeyframeSet::Save(BinStream& stream, bool cached) const {
    const int rev = 2;
    stream.WriteEndian(&rev, 4);
    mFormat.Save(stream, cached);
    stream.WriteEndian(&mNumKeyframes, 4);
    stream.Write(&mRelative, 1);
    stream.Write(mKeyframes, mNumKeyframes * mFormat.mDataEnds[CharBoneChannel::kNumTypes - 1]);
    const bool hasRootXfms = mRootXfms != nullptr;
    stream.Write(&hasRootXfms, 1);
    if (mRootXfms != nullptr) {
        stream.Write(mRootXfms, sizeof(QuatXfm) * mNumKeyframes);
    }
}

namespace {

// The identity transform. Name not in the reference map.
const Transform kIdentityXfm = {{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, {0.0f, 0.0f, 0.0f}};

// The position of a frame clamped to the keyframes. Inlined into Apply.
// Name not in the reference map.
CharBoneKeyframeSet::FramePos FramePosAt(float frame, unsigned int numKeyframes) {
    const float last = static_cast<float>(numKeyframes) + -1.0f;
    const float clamped = last < frame ? last : (frame > 0.0f ? frame : 0.0f);
    const unsigned int whole = static_cast<unsigned int>(static_cast<long>(clamped));
    return {whole, clamped - static_cast<float>(whole)};
}

// The transform of a root transform. Inlined into Apply. Name not in the
// reference map.
Transform ToTransform(const QuatXfm& xfm) {
    Transform result = kIdentityXfm;
    result.v = xfm.v;
    MakeRotMatrix(xfm.q, result.m);
    return result;
}

// `a` relative to `b`: a composed with the inverse of b, the translation
// taken as a difference first. Inlined into Apply. Name not in the
// reference map.
void RelativeXfm(const Transform& a, const Transform& b, Transform& result) {
    Hmx::Matrix3 inverse = Hmx::Matrix3::sID;
    Invert(b.m, inverse, nullptr);
    const Hmx::Matrix3 m = a.m;
    const Vector3 d = {a.v.x - b.v.x, a.v.y - b.v.y, a.v.z - b.v.z};
    const Vector3* const rows[3] = {&m.x, &m.y, &m.z};
    Vector3* const out[3] = {&result.m.x, &result.m.y, &result.m.z};
    for (int i = 0; i < 3; ++i) {
        const Vector3& r = *rows[i];
        *out[i] = {
            r.x * inverse.x.x + r.y * inverse.y.x + r.z * inverse.z.x,
            r.x * inverse.x.y + r.y * inverse.y.y + r.z * inverse.z.y,
            r.x * inverse.x.z + r.y * inverse.y.z + r.z * inverse.z.z};
    }
    result.v = {
        d.x * inverse.x.x + d.y * inverse.y.x + d.z * inverse.z.x,
        d.x * inverse.x.y + d.y * inverse.y.y + d.z * inverse.z.y,
        d.x * inverse.x.z + d.y * inverse.y.z + d.z * inverse.z.z};
}

}  // namespace

// Reconstructed from eboot.elf at 0x335220. The frame is clamped to the
// keyframes and its two keyframes added with the weight split between them.
// With root transforms the root bones are kept relative to the animation's
// root motion: the buffer's earlier root bones are set aside, this
// animation's are expressed in its root transform at the frame and added
// back, and the root motion from the frame `delta` ago (through the loop's
// end when `wrapped`) goes to the buffer's frame root motion. A layered
// animation first fades what the buffer holds.
void CharBoneKeyframeSet::Apply(
    float weight,
    float frame,
    float delta,
    bool wrapped,
    AnimResource::BlendMode mode,
    CharAnimBuffer& buffer) const {
    if (mNumKeyframes == 0) {
        return;
    }
    FramePos pos = FramePosAt(frame, mNumKeyframes);
    const float fraction = pos.mFraction;
    if (mode == AnimResource::kBlendLayered) {
        const CharBoneStateSet set = {&mFormat, mKeyframes};
        buffer.ScaleDown(set, 1.0f - weight, mRelative);
    }
    unsigned long numRoots = 0;
    const unsigned int* roots = nullptr;
    QuatXfm* rootXfms = nullptr;
    if (mRootXfms != nullptr) {
        roots = buffer.mSkeleton->GetRootIndices(numRoots);
        if (numRoots != 0) {
            rootXfms = static_cast<QuatXfm*>(__builtin_alloca(numRoots * sizeof(QuatXfm)));
        }
        buffer.GrabAndClearRootBones(rootXfms);
    }
    const unsigned int size = mFormat.mDataEnds[CharBoneChannel::kNumTypes - 1];
    const CharBoneStateSet from = {&mFormat, mKeyframes + pos.mFrame * size};
    buffer.Apply(from, (1.0f - fraction) * weight, mRelative);
    if (fraction > 0.0f) {
        const CharBoneStateSet to = {&mFormat, mKeyframes + (pos.mFrame + 1) * size};
        buffer.Apply(to, fraction * weight, mRelative);
    }
    if (mRootXfms == nullptr) {
        return;
    }
    if (mode == AnimResource::kBlendLayered) {
        buffer.ScaleDownDeltaRootXfm(1.0f - weight);
    }

    QuatXfm root;
    root.v = {0.0f, 0.0f, 0.0f};
    GetRootXfm(pos, root);
    Transform rootXfm = ToTransform(root);
    if (numRoots != 0) {
        const float scale = 1.0f / weight;
        for (unsigned long i = 0; i < numRoots; ++i) {
            QuatXfm bone;
            buffer.GrabBone(roots[i], bone);
            bone.v = {bone.v.x * scale, bone.v.y * scale, bone.v.z * scale};
            bone.q = {bone.q.x * scale, bone.q.y * scale, bone.q.z * scale, bone.q.w * scale};
            Transform relative;
            RelativeXfm(ToTransform(bone), rootXfm, relative);
            bone.v = relative.v;
            bone.q.Set(relative.m);
            QuatXfm& sum = rootXfms[i];
            sum.v.x += weight * bone.v.x;
            sum.v.y += weight * bone.v.y;
            sum.v.z += weight * bone.v.z;
            const float magnitude = __builtin_fabsf(weight);
            const Hmx::Quat q = {bone.q.x * magnitude, bone.q.y * magnitude, bone.q.z * magnitude, bone.q.w * weight};
            if (sum.q.y * q.y + sum.q.x * q.x + sum.q.z * q.z + sum.q.w * q.w < 0.0f) {
                sum.q = {sum.q.x - q.x, sum.q.y - q.y, sum.q.z - q.z, sum.q.w - q.w};
            } else {
                sum.q = {sum.q.x + q.x, sum.q.y + q.y, sum.q.z + q.z, sum.q.w + q.w};
            }
        }
        buffer.SetRootBones(rootXfms);
    }
    if (wrapped) {
        // Past the loop's end the root motion continues from the end's root
        // transform.
        pos = FramePosAt(static_cast<float>(mNumKeyframes) + -1.0f, mNumKeyframes);
        QuatXfm end;
        end.v = {0.0f, 0.0f, 0.0f};
        GetRootXfm(pos, end);
        const Transform endXfm = ToTransform(end);
        const float first = static_cast<float>(mNumKeyframes) + -1.0f;
        pos = FramePosAt(first < 0.0f ? first : 0.0f, mNumKeyframes);
        QuatXfm start;
        start.v = {0.0f, 0.0f, 0.0f};
        GetRootXfm(pos, start);
        Transform loop;
        RelativeXfm(endXfm, ToTransform(start), loop);
        Multiply(loop, rootXfm, rootXfm);
    }
    QuatXfm previous;
    pos = FramePosAt(frame - delta, mNumKeyframes);
    GetRootXfm(pos, previous);
    Transform motion;
    RelativeXfm(rootXfm, ToTransform(previous), motion);
    QuatXfm deltaRoot;
    deltaRoot.v = motion.v;
    deltaRoot.q.Set(motion.m);
    buffer.ApplyDeltaRootXfm(deltaRoot, weight);
}

// Reconstructed from eboot.elf at 0x336180.
void CharBoneKeyframeSet::ScaleDown(float weight, CharAnimBuffer& buffer) const {
    const CharBoneStateSet set = {&mFormat, mKeyframes};
    buffer.ScaleDown(set, weight, mRelative);
}

// Reconstructed from eboot.elf at 0x3361D0. The translation is blended
// through the inline vector interpolation, which keeps the ends exactly.
void CharBoneKeyframeSet::GetRootXfm(const FramePos& pos, QuatXfm& xfm) const {
    const QuatXfm& from = mRootXfms[pos.mFrame];
    if (pos.mFraction <= 0.0f) {
        xfm = from;
        return;
    }
    const QuatXfm& to = mRootXfms[pos.mFrame + 1];
    const float t = pos.mFraction;
    if (t == 0.0f) {
        xfm.v = from.v;
    } else if (t == 1.0f) {
        xfm.v = to.v;
    } else {
        xfm.v.x = (to.v.x - from.v.x) * t + from.v.x;
        xfm.v.y = (to.v.y - from.v.y) * t + from.v.y;
        xfm.v.z = (to.v.z - from.v.z) * t + from.v.z;
    }
    FastInterp(from.q, to.q, t, xfm.q);
}
