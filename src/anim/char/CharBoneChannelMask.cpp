// anim/CharBoneChannelMask.o (0x2E14E0 to 0x2E1AEF).
#include "anim/char/CharBoneChannelMask.h"

#include "utl/streams/BinStream.h"
#include "utl/text/Str.h"

// Reconstructed from eboot.elf at 0x2E14E0.
CharBoneChannelMask::CharBoneChannelMask()
    : mUsePosition(false), mUseScale(false), mUseInverseParentScale(false), mRotation(kRotationNone), mName() {}

// Reconstructed from eboot.elf at 0x2E1510.
CharBoneChannelMask::~CharBoneChannelMask() {}

// Reconstructed from eboot.elf at 0x2E1520.
void CharBoneChannelMask::SetSRTIPS() {
    mUsePosition = true;
    mUseScale = true;
    mUseInverseParentScale = true;
    mRotation = kRotationFull;
}

// Reconstructed from eboot.elf at 0x2E1540.
void CharBoneChannelMask::SetDisable() {
    mUsePosition = false;
    mUseScale = false;
    mUseInverseParentScale = false;
    mRotation = kRotationNone;
}

// Reconstructed from eboot.elf at 0x2E1560.
unsigned int CharBoneChannelMask::GetBitfield() const {
    unsigned int bits = static_cast<unsigned int>(mUsePosition) | static_cast<unsigned int>(mUseScale) << 1
        | static_cast<unsigned int>(mUseInverseParentScale) << 2;
    bits |= static_cast<unsigned int>(mRotation == kRotationFull) << 3;
    bits |= static_cast<unsigned int>(mRotation == kRotationX) << 4;
    bits |= static_cast<unsigned int>(mRotation == kRotationY) << 5;
    bits |= static_cast<unsigned int>(mRotation == kRotationZ) << 6;
    return bits;
}

// Reconstructed from eboot.elf at 0x2E15F0.
bool CharBoneChannelMask::operator!=(const CharBoneChannelMask& other) const {
    return mName != other.mName || mUsePosition != other.mUsePosition || mUseScale != other.mUseScale
        || mUseInverseParentScale != other.mUseInverseParentScale || mRotation != other.mRotation;
}

// Reconstructed from eboot.elf at 0x2E1620.
eastl::vector<AllowedValue<int>> CharBoneChannelMask::GetRotationAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(5);
    values.push_back(AllowedValue<int>{
        kRotationNone, String("None"), String("This bone contributes no rotation information.")});
    values.push_back(AllowedValue<int>{kRotationX, String("X"), String("This bone's X rotation is used.")});
    values.push_back(AllowedValue<int>{kRotationY, String("Y"), String("This bone's Y rotation is used.")});
    values.push_back(AllowedValue<int>{kRotationZ, String("Z"), String("This bone's Z rotation is used.")});
    values.push_back(AllowedValue<int>{
        kRotationFull, String("Full"), String("All of this bone's rotation information is used (i.e. XYZ).")});
    return values;
}

// Reconstructed from eboot.elf at 0x2E1970.
BinStream& operator<<(BinStream& stream, const CharBoneChannelMask& mask) {
    const int rev = 0;
    stream.WriteEndian(&rev, sizeof(rev));
    stream << mask.mName;
    bool flag = mask.mUsePosition;
    stream.Write(&flag, 1);
    flag = mask.mUseScale;
    stream.Write(&flag, 1);
    flag = mask.mUseInverseParentScale;
    stream.Write(&flag, 1);
    const int rotation = mask.mRotation;
    stream.WriteEndian(&rotation, sizeof(rotation));
    return stream;
}

// Reconstructed from eboot.elf at 0x2E1A30. The revision is read and not
// checked.
BinStream& operator>>(BinStream& stream, CharBoneChannelMask& mask) {
    int rev = 0;
    stream.ReadEndian(&rev, sizeof(rev));
    stream >> mask.mName;
    unsigned char flag = 0;
    stream.Read(&flag, 1);
    mask.mUsePosition = flag != 0;
    stream.Read(&flag, 1);
    mask.mUseScale = flag != 0;
    stream.Read(&flag, 1);
    mask.mUseInverseParentScale = flag != 0;
    int rotation = 0;
    stream.ReadEndian(&rotation, sizeof(rotation));
    mask.mRotation = static_cast<CharBoneChannelMask::Rotation>(rotation);
    return stream;
}
