#pragma once

enum RndShaderNumericType : unsigned int;

// Shader helpers: numeric type names and source file checksums.
class RndShaderUtl {
public:
    // HLSL spelling of a numeric type, or null when the type is out of range.
    static const char* NumericTypeToHlsl(RndShaderNumericType type);  // 0x645680

    // Scalar type underlying a numeric type (table at 0x12AC200), or -1 when
    // the type is out of range. Name not in the reference map.
    static RndShaderNumericType GetNumericTypeBaseType(
        RndShaderNumericType type);  // 0x645630

    // Hashes a shader source file byte by byte, ignoring carriage returns so
    // line-ending conversions do not invalidate compiled caches. The map also
    // has ChecksumSourceCodeFile(char const*, Hmx::CRC&); this build has the
    // single function.
    static unsigned int ChecksumSourceCodeFile(const char* path);  // 0x6456C0
};

// Generated shader source is never materialized at runtime; the binary
// streams it into a CrcTextStream (vtable 0x1911CD8), whose Print folds each
// character into an FNV-1a hash. Until TextStream is reconstructed the
// PrintCode methods take the hash itself, and these functions reproduce the
// stream's text and number output. Names not in the reference map.
constexpr unsigned int kCrcTextStreamBasis = 0x811C9DC5U;

// CrcTextStream::Print (0x50CA20). Characters are sign-extended before being
// mixed into the hash.
void CrcPrint(unsigned int& crc, const char* text);
// TextStream::operator<<(unsigned long) (0x2589D0), which formats with "%lu".
void CrcPrintUnsigned(unsigned int& crc, unsigned long value);
// TextStream::operator<<(int).
void CrcPrintSigned(unsigned int& crc, int value);
// Mixes one character into the hash.
void CrcPrintByte(unsigned int& crc, unsigned char value);
