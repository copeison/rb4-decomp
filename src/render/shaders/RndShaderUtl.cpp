#include "render/shaders/RndShaderUtl.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "utl/streams/FileStream.h"

namespace {

constexpr unsigned int kFnvPrime = 0x01000193U;

// HLSL names of the numeric types. Name not in the reference map.
constexpr const char* kHlslNames[] = {
    "bool", "int", "int2", "int3", "int4",
    "uint", "uint2", "uint3", "uint4",
    "float", "float2", "float3", "float4",
    "float3x3", "float3x4", "float4x3", "float4x4",
};

// Table at 0x12AC200: the scalar type underlying each numeric type. Name not
// in the reference map.
constexpr unsigned int kBaseTypes[] = {
    0, 1, 1, 1, 1, 5, 5, 5, 5, 9, 9, 9, 9, 9, 9, 9, 9,
};

constexpr unsigned int kNumNumericTypes =
    sizeof(kHlslNames) / sizeof(*kHlslNames);
static_assert(sizeof(kBaseTypes) / sizeof(*kBaseTypes) == kNumNumericTypes);

}  // namespace

// Reconstructed from eboot.elf at 0x645680.
const char* RndShaderUtl::NumericTypeToHlsl(RndShaderNumericType type) {
    return static_cast<unsigned int>(type) < kNumNumericTypes
        ? kHlslNames[type]
        : nullptr;
}

// Reconstructed from eboot.elf at 0x645630.
RndShaderNumericType RndShaderUtl::GetNumericTypeBaseType(
    RndShaderNumericType type) {
    return static_cast<RndShaderNumericType>(
        static_cast<unsigned int>(type) < kNumNumericTypes ? kBaseTypes[type]
                                                           : 0xFFFFFFFFU);
}

// Reconstructed from eboot.elf at 0x6456C0.
unsigned int RndShaderUtl::ChecksumSourceCodeFile(const char* path) {
    FileStream stream(path, kRead, false);
    auto remaining = stream.Size();
    auto crc = kCrcTextStreamBasis;
    for (; remaining != 0; --remaining) {
        char character = 0;
        stream.Read(&character, 1);
        if (character != '\r') {
            CrcPrintByte(crc, static_cast<unsigned char>(character));
        }
    }
    return crc;
}

// Reconstructed from eboot.elf at 0x50CA20.
void CrcPrint(unsigned int& crc, const char* text) {
    while (*text != '\0') {
        CrcPrintByte(crc, static_cast<unsigned char>(*text));
        ++text;
    }
}

void CrcPrintByte(unsigned int& crc, unsigned char value) {
    const auto extended = static_cast<unsigned int>(
        static_cast<int>(static_cast<signed char>(value)));
    crc = (crc ^ extended) * kFnvPrime;
}

// Reconstructed from eboot.elf at 0x2589D0.
void CrcPrintUnsigned(unsigned int& crc, unsigned long value) {
    char digits[21];
    auto* end = digits + sizeof(digits) - 1;
    auto* cursor = end;
    *end = '\0';
    do {
        *--cursor = static_cast<char>('0' + value % 10);
        value /= 10;
    } while (value != 0);
    CrcPrint(crc, cursor);
}

void CrcPrintSigned(unsigned int& crc, int value) {
    if (value < 0) {
        CrcPrint(crc, "-");
        CrcPrintUnsigned(
            crc, static_cast<unsigned long>(-static_cast<long>(value)));
        return;
    }
    CrcPrintUnsigned(crc, static_cast<unsigned int>(value));
}
