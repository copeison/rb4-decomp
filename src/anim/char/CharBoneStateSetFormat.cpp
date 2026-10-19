// anim/CharBoneStateSetFormat.o (0x2E4ED0 to 0x2E5C8F). The binary emits the
// object between anim/CharBoneMaskResource.o and the clip objects; the
// eastl::vector growth it uses is the container template's.
#include "anim/char/CharBoneStateSetFormat.h"

#include <string.h>

#include "anim/char/CharBoneChannelMask.h"
#include "os/memory/MemMgr.h"
#include "utl/streams/BinStream.h"

// Reconstructed from eboot.elf at 0x2E4ED0.
CharBoneStateSetFormat::CharBoneStateSetFormat() : mBoneEnds(), mDataEnds(), mBoneNames() {}

// Reconstructed from eboot.elf at 0x2E4F10. Positions, scales and inverse
// parent scales take three floats per bone, full rotations a quaternion and
// single-axis rotations 8 bytes.
CharBoneStateSetFormat::CharBoneStateSetFormat(const PropArray<CharBoneChannelMask>& masks) : mBoneNames() {
    eastl::vector<Symbol> names[CharBoneChannel::kNumTypes];
    for (unsigned long i = 0; i < masks.size(); ++i) {
        const CharBoneChannelMask& mask = masks[i];
        if (mask.mUsePosition) {
            InsertBoneName(names[CharBoneChannel::kPosition], mask.mName);
        }
        if (mask.mUseScale) {
            InsertBoneName(names[CharBoneChannel::kScale], mask.mName);
        }
        if (mask.mUseInverseParentScale) {
            InsertBoneName(names[CharBoneChannel::kInverseParentScale], mask.mName);
        }
        switch (mask.mRotation) {
        case CharBoneChannelMask::kRotationFull:
            InsertBoneName(names[CharBoneChannel::kRotation], mask.mName);
            break;
        case CharBoneChannelMask::kRotationX:
            InsertBoneName(names[CharBoneChannel::kRotationX], mask.mName);
            break;
        case CharBoneChannelMask::kRotationY:
            InsertBoneName(names[CharBoneChannel::kRotationY], mask.mName);
            break;
        case CharBoneChannelMask::kRotationZ:
            InsertBoneName(names[CharBoneChannel::kRotationZ], mask.mName);
            break;
        default:
            break;
        }
    }
    for (int type = 0; type < CharBoneChannel::kNumTypes; ++type) {
        for (const Symbol& name : names[type]) {
            mBoneNames.push_back(name);
        }
    }
    static const unsigned int kChannelSizes[CharBoneChannel::kNumTypes] = {12, 12, 12, 16, 8, 8, 8};
    unsigned int bones = 0;
    unsigned int bytes = 0;
    for (int type = 0; type < CharBoneChannel::kNumTypes; ++type) {
        const unsigned int count = static_cast<unsigned int>(names[type].size());
        bones += count;
        bytes += kChannelSizes[type] * count;
        mBoneEnds[type] = bones;
        mDataEnds[type] = bytes;
    }
}

// Reconstructed from eboot.elf at 0x2E5470.
void CharBoneStateSetFormat::InsertBoneName(eastl::vector<Symbol>& names, const Symbol& name) const {
    for (const Symbol& existing : names) {
        if (existing == name) {
            return;
        }
    }
    names.push_back(name);
}

// Reconstructed from eboot.elf at 0x2E5580.
CharBoneStateSetFormat::~CharBoneStateSetFormat() {}

// Reconstructed from eboot.elf at 0x2E55F0. The string table is read into
// temporary memory.
bool CharBoneStateSetFormat::Load(BinStream& stream, bool cached) {
    static_cast<void>(cached);
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 0) {
        return false;
    }
    stream.Read(mBoneEnds, sizeof(mBoneEnds));
    stream.Read(mDataEnds, sizeof(mDataEnds));
    // The channel count, the string count and the string table's size.
    unsigned int header[3];
    stream.Read(header, sizeof(header));
    mBoneNames.clear();
    mBoneNames.reserve(header[0]);
    unsigned int saved = 0;
    MemPushTemp(saved, true, true);
    const char** const strings = new const char*[header[1]];
    char* const table = new char[header[2]];
    unsigned int* const indices = new unsigned int[header[0]];
    stream.Read(indices, 4UL * header[0]);
    stream.Read(table, header[2]);
    const char* string = table;
    for (unsigned int i = 0; i < header[1]; ++i) {
        strings[i] = string;
        string += strlen(string) + 1;
    }
    MemPopTemp(saved);
    for (unsigned int i = 0; i < header[0]; ++i) {
        mBoneNames.push_back(Symbol(strings[indices[i]]));
    }
    delete[] indices;
    delete[] table;
    delete[] strings;
    return true;
}

// Reconstructed from eboot.elf at 0x2E58D0.
void CharBoneStateSetFormat::Save(BinStream& stream, bool cached) const {
    static_cast<void>(cached);
    const int rev = 0;
    stream.WriteEndian(&rev, 4);
    stream.Write(mBoneEnds, sizeof(mBoneEnds));
    stream.Write(mDataEnds, sizeof(mDataEnds));
    eastl::vector<Symbol> strings;
    eastl::vector<unsigned int> indices;
    for (const Symbol& name : mBoneNames) {
        unsigned int index = 0;
        Symbol* found = strings.begin();
        while (found != strings.end() && *found != name) {
            ++found;
        }
        if (found == strings.end()) {
            strings.push_back(name);
            index = static_cast<unsigned int>(strings.size()) - 1;
        } else {
            index = static_cast<unsigned int>(found - strings.begin());
        }
        indices.push_back(index);
    }
    unsigned int header[3] = {static_cast<unsigned int>(indices.size()), static_cast<unsigned int>(strings.size()), 0};
    for (const Symbol& string : strings) {
        header[2] += static_cast<unsigned int>(strlen(string.Str())) + 1;
    }
    stream.Write(header, sizeof(header));
    stream.Write(indices.begin(), 4UL * header[0]);
    for (const Symbol& string : strings) {
        stream.Write(string.Str(), strlen(string.Str()) + 1);
    }
}
