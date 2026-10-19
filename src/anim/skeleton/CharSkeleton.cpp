// anim/CharSkeleton.o (0x2F7260 to 0x2FA3DB). The binary also emits the
// introsort of the allowed values (0x2F8920-0x2F9760) and the vector
// growth helpers here; the containers model them.
#include "anim/skeleton/CharSkeleton.h"

#include <strings.h>

#include "utl/streams/BinStream.h"
#include "utl/text/Str.h"

namespace {

// Allowed bone values sort by name without regard to case; equal names by
// their labels. The map's AllowedValue<Symbol> comparison; inlined into
// the sort. Name not in the reference map.
bool AllowedBoneLess(const AllowedValue<Symbol>& a, const AllowedValue<Symbol>& b) {
    if (a.mValue != b.mValue) {
        return strcasecmp(a.mValue.Str(), b.mValue.Str()) < 0;
    }
    return a.mName < b.mName;
}

// The map has these stream operators in the math and skeleton headers;
// the binary emits them in this object.
// Reconstructed from eboot.elf at 0x2F7DA0.
BinStream& operator>>(BinStream& stream, Transform& xfm) {
    stream.ReadEndian(&xfm.m.x.x, 4);
    stream.ReadEndian(&xfm.m.x.y, 4);
    stream.ReadEndian(&xfm.m.x.z, 4);
    stream.ReadEndian(&xfm.m.y.x, 4);
    stream.ReadEndian(&xfm.m.y.y, 4);
    stream.ReadEndian(&xfm.m.y.z, 4);
    stream.ReadEndian(&xfm.m.z.x, 4);
    stream.ReadEndian(&xfm.m.z.y, 4);
    stream.ReadEndian(&xfm.m.z.z, 4);
    stream.ReadEndian(&xfm.v.x, 4);
    stream.ReadEndian(&xfm.v.y, 4);
    stream.ReadEndian(&xfm.v.z, 4);
    return stream;
}

// Writes one float in the stream's byte order. Inlined into the writers.
// Name not in the reference map.
void WriteFloat(BinStream& stream, float value) {
    stream.WriteEndian(&value, 4);
}

// Reconstructed from eboot.elf at 0x2F81D0.
BinStream& operator<<(BinStream& stream, const Hmx::Matrix3& matrix) {
    WriteFloat(stream, matrix.x.x);
    WriteFloat(stream, matrix.x.y);
    WriteFloat(stream, matrix.x.z);
    WriteFloat(stream, matrix.y.x);
    WriteFloat(stream, matrix.y.y);
    WriteFloat(stream, matrix.y.z);
    WriteFloat(stream, matrix.z.x);
    WriteFloat(stream, matrix.z.y);
    WriteFloat(stream, matrix.z.z);
    return stream;
}

// Reconstructed from eboot.elf at 0x2F80E0.
BinStream& operator<<(BinStream& stream, const eastl::vector<Transform>& transforms) {
    int count = static_cast<int>(transforms.size());
    stream.WriteEndian(&count, 4);
    for (const Transform& xfm : transforms) {
        stream << xfm.m;
        WriteFloat(stream, xfm.v.x);
        WriteFloat(stream, xfm.v.y);
        WriteFloat(stream, xfm.v.z);
    }
    return stream;
}

// Reconstructed from eboot.elf at 0x2F98C0.
BinStream& operator>>(BinStream& stream, CharBoneTransform& bone) {
    stream.ReadEndian(&bone.mRotation.x, 4);
    stream.ReadEndian(&bone.mRotation.y, 4);
    stream.ReadEndian(&bone.mRotation.z, 4);
    stream.ReadEndian(&bone.mRotation.w, 4);
    stream.ReadEndian(&bone.mScale.x, 4);
    stream.ReadEndian(&bone.mScale.y, 4);
    stream.ReadEndian(&bone.mScale.z, 4);
    stream.ReadEndian(&bone.mTranslation.x, 4);
    stream.ReadEndian(&bone.mTranslation.y, 4);
    stream.ReadEndian(&bone.mTranslation.z, 4);
    stream.ReadEndian(&bone.mParentScale.x, 4);
    stream.ReadEndian(&bone.mParentScale.y, 4);
    stream.ReadEndian(&bone.mParentScale.z, 4);
    return stream;
}

// Reconstructed from eboot.elf at 0x2F9D50.
BinStream& operator<<(BinStream& stream, const CharBoneTransform& bone) {
    WriteFloat(stream, bone.mRotation.x);
    WriteFloat(stream, bone.mRotation.y);
    WriteFloat(stream, bone.mRotation.z);
    WriteFloat(stream, bone.mRotation.w);
    WriteFloat(stream, bone.mScale.x);
    WriteFloat(stream, bone.mScale.y);
    WriteFloat(stream, bone.mScale.z);
    WriteFloat(stream, bone.mTranslation.x);
    WriteFloat(stream, bone.mTranslation.y);
    WriteFloat(stream, bone.mTranslation.z);
    WriteFloat(stream, bone.mParentScale.x);
    WriteFloat(stream, bone.mParentScale.y);
    WriteFloat(stream, bone.mParentScale.z);
    return stream;
}

// Reads a count and resizes the vector to it. Inlined into Load. Name not
// in the reference map.
template <class T>
void ReadCount(BinStream& stream, eastl::vector<T>& values) {
    unsigned int count = 0;
    stream.ReadEndian(&count, 4);
    values.resize(count);
}

// Writes the vector's count. Inlined into Save. Name not in the reference
// map.
template <class T>
void WriteCount(BinStream& stream, const eastl::vector<T>& values) {
    int count = static_cast<int>(values.size());
    stream.WriteEndian(&count, 4);
}

}  // namespace

// Reconstructed from eboot.elf at 0x2F7260.
CharSkeleton::~CharSkeleton() {}

// Reconstructed from eboot.elf at 0x2F7300.
CharSkeleton* CharSkeleton::New() {
    return new CharSkeleton();
}

// Reconstructed from eboot.elf at 0x2F7400.
long CharSkeleton::FindBoneIndex(Symbol name) const {
    for (unsigned long bone = 0; bone < mNames.size(); ++bone) {
        if (mNames[bone] == name) {
            return static_cast<long>(bone);
        }
    }
    return -1;
}

// Reconstructed from eboot.elf at 0x2F7440.
void CharSkeleton::ComputeProjectSpaceXfm(unsigned long bone, Transform& xfm) const {
    Transform result;
    mLocalTransforms[bone].ToTransform(result);
    unsigned long parent = mParentIndices[bone];
    if (parent != bone) {
        for (;;) {
            Transform parentXfm;
            mLocalTransforms[parent].ToTransform(parentXfm);
            Multiply(result, parentXfm, result);
            const unsigned long next = mParentIndices[parent];
            if (parent == next) {
                break;
            }
            parent = next;
        }
    }
    xfm = result;
}

// Reconstructed from eboot.elf at 0x2F7750.
eastl::vector<AllowedValue<Symbol>> CharSkeleton::GetAllowedValueBones(bool includeNone) const {
    eastl::vector<AllowedValue<Symbol>> values;
    values.reserve((includeNone ? 1 : 0) + mNames.size());
    if (includeNone) {
        values.push_back(AllowedValue<Symbol>{Symbol(""), String("none"), String("none")});
    }
    for (unsigned long bone = 0; bone < mNames.size(); ++bone) {
        const Symbol name = mNames[bone];
        values.push_back(AllowedValue<Symbol>{name, String(name.Str()), String(name.Str())});
    }
    eastl::sort(values.begin(), values.end(), AllowedBoneLess);
    return values;
}

// Reconstructed from eboot.elf at 0x2F7A10. The mesh-to-bone transforms
// come before the names in the file.
bool CharSkeleton::Load(BinStream& stream) {
    unsigned int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev < 4) {
        return false;
    }
    ReadCount(stream, mParentIndices);
    for (unsigned int& parent : mParentIndices) {
        stream.ReadEndian(&parent, 4);
    }
    ReadCount(stream, mChildIndices);
    for (unsigned int& child : mChildIndices) {
        stream.ReadEndian(&child, 4);
    }
    ReadCount(stream, mLocalTransforms);
    for (CharBoneTransform& bone : mLocalTransforms) {
        stream >> bone;
    }
    ReadCount(stream, mMeshToBoneTransforms);
    for (Transform& xfm : mMeshToBoneTransforms) {
        stream >> xfm;
    }
    ReadCount(stream, mNames);
    for (Symbol& name : mNames) {
        stream >> name;
    }
    stream >> mRootTransform;
    stream.ReadEndian(&mRootRotation.x.x, 4);
    stream.ReadEndian(&mRootRotation.x.y, 4);
    stream.ReadEndian(&mRootRotation.x.z, 4);
    stream.ReadEndian(&mRootRotation.y.x, 4);
    stream.ReadEndian(&mRootRotation.y.y, 4);
    stream.ReadEndian(&mRootRotation.y.z, 4);
    stream.ReadEndian(&mRootRotation.z.x, 4);
    stream.ReadEndian(&mRootRotation.z.y, 4);
    stream.ReadEndian(&mRootRotation.z.z, 4);
    stream.ReadEndian(&mUnitScale, 4);
    return true;
}

// Reconstructed from eboot.elf at 0x2F7E80.
void CharSkeleton::Save(BinStream& stream) const {
    int rev = 4;
    stream.WriteEndian(&rev, 4);
    WriteCount(stream, mParentIndices);
    for (unsigned int parent : mParentIndices) {
        stream.WriteEndian(&parent, 4);
    }
    WriteCount(stream, mChildIndices);
    for (unsigned int child : mChildIndices) {
        stream.WriteEndian(&child, 4);
    }
    WriteCount(stream, mLocalTransforms);
    for (const CharBoneTransform& bone : mLocalTransforms) {
        stream << bone;
    }
    stream << mMeshToBoneTransforms;
    WriteCount(stream, mNames);
    for (const Symbol& name : mNames) {
        stream << name;
    }
    stream << mRootTransform.m;
    WriteFloat(stream, mRootTransform.v.x);
    WriteFloat(stream, mRootTransform.v.y);
    WriteFloat(stream, mRootTransform.v.z);
    stream << mRootRotation;
    WriteFloat(stream, mUnitScale);
}

// Reconstructed from eboot.elf at 0x2F82E0.
void CharSkeleton::SetParentIndices(const eastl::vector<unsigned int>& parents) {
    mParentIndices = parents;
    _ComputeChildIndices();
}

// Reconstructed from eboot.elf at 0x2F83E0. A bone's children follow it
// until a bone whose parent comes before it.
void CharSkeleton::_ComputeChildIndices() {
    const unsigned long count = mParentIndices.size();
    mChildIndices.reserve(count * 3);
    mChildIndices.resize(count + 1);
    for (unsigned long bone = 0; bone < count; ++bone) {
        mChildIndices[bone] = static_cast<unsigned int>(mChildIndices.size());
        for (unsigned long child = bone + 1; child < count; ++child) {
            const unsigned int parent = mParentIndices[child];
            if (parent < bone) {
                break;
            }
            if (parent == bone) {
                mChildIndices.push_back(static_cast<unsigned int>(child));
            }
        }
    }
    mChildIndices[count] = static_cast<unsigned int>(mChildIndices.size());
    for (unsigned long bone = 0; bone < count; ++bone) {
        if (mParentIndices[bone] == bone) {
            mChildIndices.push_back(static_cast<unsigned int>(bone));
        }
    }
}

// Reconstructed from eboot.elf at 0x2F8770.
const unsigned int* CharSkeleton::GetChildIndices(unsigned long bone, unsigned long& count) const {
    const unsigned int start = mChildIndices[bone];
    count = mChildIndices[bone + 1] - start;
    return mChildIndices.mpBegin + start;
}

// Reconstructed from eboot.elf at 0x2F8790.
const unsigned int* CharSkeleton::GetRootIndices(unsigned long& count) const {
    const unsigned int start = mChildIndices[mLocalTransforms.size()];
    count = mChildIndices.size() - start;
    return mChildIndices.mpBegin + start;
}

// Reconstructed from eboot.elf at 0x2F87D0.
void CharSkeleton::SetLocalTransforms(const eastl::vector<CharBoneTransform>& transforms) {
    mLocalTransforms = transforms;
}

// Reconstructed from eboot.elf at 0x2F87F0.
void CharSkeleton::SetMeshToBoneTransforms(const eastl::vector<Transform>& transforms) {
    mMeshToBoneTransforms = transforms;
}

// Reconstructed from eboot.elf at 0x2F8810.
void CharSkeleton::SetNames(const eastl::vector<Symbol>& names) {
    mNames = names;
}
