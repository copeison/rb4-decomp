#include "render/shaders/RndShaderDefines.h"

#include <vector>

#include "render/shaders/RndShaderUtl.h"

namespace {

void Print(unsigned int& hash, const char* text) {
    CrcPrint(hash, text);
}

struct Enumeration {
    std::vector<const RndShaderDefines::Entry*> mEntries;
    std::vector<RndShaderMacro> mMacros;
    RndShaderPermutationVisitor mVisitor;
    void* mContext;
};

void AppendEntries(Enumeration& enumeration, const RndShaderDefines& defines) {
    for (const auto& entry : defines.mEntries) {
        enumeration.mEntries.push_back(&entry);
    }
}

// Reconstructed from eboot.elf at 0x63C8F0, RndShaderDefines::VisitRecur.
// Each define contributes every value in its range. The shifted 32-bit field
// is sign-extended before placement.
void VisitRecur(Enumeration& enumeration, unsigned long depth, RndShaderKey key) {
    if (depth >= enumeration.mEntries.size()) {
        enumeration.mVisitor(
            enumeration.mContext,
            enumeration.mMacros.data(),
            enumeration.mMacros.size(),
            key);
        return;
    }

    const auto& entry = *enumeration.mEntries[depth];
    const auto shift = entry.mGlobal ? 32U : 0U;
    const auto mask = static_cast<RndShaderKey>(entry.mMask) << shift;
    for (int value = entry.mFirst; value < entry.mEnd; ++value) {
        enumeration.mMacros.push_back(
            {entry.mName.c_str(), IntToStaticString(value)});
        const auto shifted = static_cast<int>(
            static_cast<unsigned int>(value - entry.mFirst) << entry.mShift);
        const auto field = static_cast<RndShaderKey>(static_cast<long>(shifted))
            << shift;
        VisitRecur(enumeration, depth + 1, (key & ~mask) | field);
        enumeration.mMacros.pop_back();
    }
}

}  // namespace

constexpr unsigned long RndShaderDefinesGroup::kProgramDefines[];

// Reconstructed from eboot.elf at 0x63D520.
void RndShaderFixedDefines::Add(Symbol name, int value) {
    mDefines.emplace_back(Define{name.Str(), value, String("")});
}

// Reconstructed from eboot.elf at 0x63D5D0.
void RndShaderFixedDefines::AddComment(const char* comment) {
    const Symbol empty("");
    mDefines.emplace_back(Define{empty.Str(), 0, String(comment)});
}

// Reconstructed from eboot.elf at 0x63D6E0.
void RndShaderFixedDefines::PrintCode(unsigned int& hash) const {
    for (const auto& define : mDefines) {
        if (define.mComment.c_str()[0] != '\0') {
            Print(hash, "\n// ");
            Print(hash, define.mComment.c_str());
            Print(hash, "\n");
        }
        if (define.mName[0] != '\0') {
            Print(hash, "#define ");
            Print(hash, define.mName);
            Print(hash, " ");
            CrcPrintSigned(hash, define.mValue);
            Print(hash, "\n");
        }
    }
}

// Reconstructed from eboot.elf at 0x63C380.
unsigned int RndShaderDefInfo::GetValue(RndShaderKey key) const {
    RndShaderKey mask = mMask;
    if (mGlobal) {
        mask <<= 32;
    }
    const auto field = (key & mask) >> mShift;
    const auto value = mGlobal ? static_cast<unsigned int>(field >> 32)
                               : static_cast<unsigned int>(field);
    return static_cast<unsigned int>(mFirst) + value;
}

RndShaderKey RndShaderDefInfo::SetValue(RndShaderKey key, unsigned int value) const {
    const auto field = static_cast<RndShaderKey>(static_cast<long>(
        static_cast<int>((value - static_cast<unsigned int>(mFirst)) << mShift)));
    return (key & ~static_cast<RndShaderKey>(mMask)) | field;
}

// Reconstructed from eboot.elf at 0x63C3F0. The define takes the next free
// bits, as many as its value range needs.
RndShaderDefInfo RndShaderDefines::Add(Symbol name, int first, int end) {
    auto remaining = static_cast<unsigned int>(end + ~first);
    unsigned int bits = 0;
    unsigned int mask = 0;
    while (remaining != 0) {
        remaining >>= 1;
        ++bits;
        mask = mask * 2 + 1;
    }

    mEntries.emplace_back(Entry{
        String(name.Str()), first, end, mask << mNumBits, mNumBits, mGlobal});
    mNumBits += bits;

    const auto& entry = mEntries.back();
    return {entry.mFirst, entry.mEnd, entry.mMask, entry.mShift, entry.mGlobal};
}

// Reconstructed from eboot.elf at 0x63C550.
RndShaderDefInfo RndShaderDefines::AddBool(Symbol name) {
    return Add(name, 0, 2);
}

// Reconstructed from eboot.elf at 0x63D100 and 0x63C740. The global defines
// are enumerated before the program type's own.
void RndShaderDefinesGroup::Visit(
    RndShaderProgramType type,
    RndShaderPermutationVisitor visitor,
    void* context) const {
    Enumeration enumeration{{}, {}, visitor, context};
    AppendEntries(enumeration, mDefines[0]);
    if (type < kNumShaderProgramTypes) {
        AppendEntries(enumeration, mDefines[kProgramDefines[type]]);
    }
    VisitRecur(enumeration, 0, 0);
}

// Reconstructed from eboot.elf at 0x63E5E0.
bool ShaderStagesInclude(int stages, RndShaderProgramType type) {
    switch (type) {
    case kShaderProgramVertex:
        return (stages & 0x01) != 0;
    case kShaderProgramHull:
    case kShaderProgramDomain:
        return (stages & 0x02) != 0;
    case kShaderProgramGeometry:
        return (stages & 0x04) != 0;
    case kShaderProgramPixel:
        return (stages & 0x08) != 0;
    case kShaderProgramCompute:
        return (stages & 0x10) != 0;
    }
    return false;
}
