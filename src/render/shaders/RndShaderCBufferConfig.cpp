#include "render/shaders/RndShaderCBufferConfig.h"

#include "render/shaders/RndShaderUtl.h"

namespace {

// Registers per constant of each numeric type.
constexpr unsigned long kRegisterCounts[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 3, 3, 4, 4,
};

constexpr unsigned long kRTSlices = 6;

void Print(unsigned int& hash, const char* text) {
    CrcPrint(hash, text);
}

void Print(unsigned int& hash, unsigned long value) {
    CrcPrintUnsigned(hash, value);
}

const char* TypeName(RndShaderNumericType type) {
    return RndShaderUtl::NumericTypeToHlsl(type);
}

void PrintArraySuffix(
    unsigned int& hash,
    const RndShaderCBufferConfig::Constant& constant) {
    if (constant.mCount < 0 && !constant.mRTSliced) {
        return;
    }
    const auto count =
        constant.mCount < 0 ? 1UL : static_cast<unsigned long>(constant.mCount);
    Print(hash, "[");
    Print(hash, count * (constant.mRTSliced ? kRTSlices : 1UL));
    Print(hash, "]");
}

// Metal structs pad each scalar or two-component constant to 16 bytes.
void PrintMetalMembers(unsigned int& hash, const RndShaderCBufferConfig& config) {
    for (const auto& constant : config.mConstants) {
        const auto* localName = constant.mName + 1;
        Print(hash, "   ");
        Print(hash, TypeName(constant.mType));
        Print(hash, " ");
        Print(hash, localName);
        PrintArraySuffix(hash, constant);
        Print(hash, ";\n");

        if (constant.mCount >= 0 || constant.mRTSliced) {
            continue;
        }
        const auto type = constant.mType;
        const unsigned long padding =
            type == kShaderNumericBool || type == kShaderNumericInt ||
                type == kShaderNumericUInt || type == kShaderNumericFloat
            ? 3
            : type == kShaderNumericInt2 || type == kShaderNumericUInt2 ||
                type == kShaderNumericFloat2
            ? 2
            : 0;
        for (unsigned long i = 0; i < padding; ++i) {
            Print(hash, "   float _pad");
            Print(hash, i);
            Print(hash, "_");
            Print(hash, localName);
            Print(hash, ";\n");
        }
    }
}

void PrintHlslMembers(unsigned int& hash, const RndShaderCBufferConfig& config) {
    for (const auto& constant : config.mConstants) {
        Print(hash, "   ");
        Print(hash, TypeName(constant.mType));
        Print(hash, " ");
        Print(hash, constant.mName);
        PrintArraySuffix(hash, constant);
        Print(hash, " : packoffset(c");
        Print(hash, constant.mOffset);
        Print(hash, ");\n");
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x63A0C0.
RndShaderCBufferConfig::RndShaderCBufferConfig(
    const char* name,
    unsigned int slot,
    unsigned int stages,
    unsigned long numBuffers)
    : mName(name),
      mSlot(slot),
      mStages(stages),
      mNumBuffers(numBuffers),
      mSize(0),
      mLocked(false) {}

unsigned long RndShaderCBufferConfig::_AddConstant(
    RndShaderNumericType type,
    long count,
    bool rtSliced,
    const char* name) {
    const auto offset = mSize;
    if (!mLocked) {
        mConstants.emplace_back(Constant{offset, type, count, rtSliced, name});
    }
    const auto registers =
        type < sizeof(kRegisterCounts) / sizeof(*kRegisterCounts)
        ? kRegisterCounts[type]
        : 0;
    mSize += registers * (count < 0 ? 1UL : static_cast<unsigned long>(count)) *
        (rtSliced ? kRTSlices : 1UL);
    return offset;
}

// Reconstructed from eboot.elf at 0x63A1B0.
unsigned long RndShaderCBufferConfig::AddConstant(
    RndShaderNumericType type,
    const char* name) {
    return _AddConstant(type, -1, false, name);
}

// Reconstructed from eboot.elf at 0x63A370.
unsigned long RndShaderCBufferConfig::AddConstantArray(
    RndShaderNumericType type,
    unsigned long count,
    const char* name) {
    return _AddConstant(type, static_cast<long>(count), false, name);
}

// Reconstructed from eboot.elf at 0x63A700.
unsigned long RndShaderCBufferConfig::AddRTSlicedConstantArray(
    RndShaderNumericType type,
    unsigned long count,
    const char* name) {
    return _AddConstant(type, static_cast<long>(count), true, name);
}

// Reconstructed from eboot.elf at 0x63A8C0, with the member emitters at
// 0x63AB40 and 0x63AE10.
void RndShaderCBufferConfig::PrintCode(unsigned int& hash) const {
    Print(hash, "// ");
    Print(hash, mName);
    Print(hash, " constants\n");
    Print(hash, "#if (HX_METAL == 1)\n");
    Print(hash, "HxCBuffer ");
    Print(hash, mName);
    Print(hash, "\n{\n");
    PrintMetalMembers(hash, *this);
    Print(hash, "};\n");
    Print(hash, "# define HX_USE_CBUFFER_");
    Print(hash, mName);
    Print(hash, " , constant ");
    Print(hash, mName);
    Print(hash, "& _g");
    Print(hash, mName);
    Print(hash, " [[buffer(");
    Print(hash, static_cast<unsigned long>(mSlot + 3U));
    Print(hash, ")]]\n");
    for (const auto& constant : mConstants) {
        Print(hash, "# define ");
        Print(hash, constant.mName);
        Print(hash, " _g");
        Print(hash, mName);
        Print(hash, ".");
        Print(hash, constant.mName + 1);
        Print(hash, "\n");
    }
    Print(hash, "#else\n");
    Print(hash, "HxCBuffer ");
    Print(hash, mName);
    Print(hash, " : register(b");
    Print(hash, static_cast<unsigned long>(mSlot));
    Print(hash, ")\n{\n");
    PrintHlslMembers(hash, *this);
    Print(hash, "};\n");
    Print(hash, "# define HX_USE_CBUFFER_");
    Print(hash, mName);
    Print(hash, "\n");
    Print(hash, "#endif // ...if/else HX_METAL\n");
}
