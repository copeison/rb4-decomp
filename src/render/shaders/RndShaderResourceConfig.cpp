#include "render/shaders/RndShaderResourceConfig.h"

#include "render/shaders/RndShaderUtl.h"
#include "utl/text/Symbol.h"

namespace {

constexpr unsigned int kTextureResource = 0;
constexpr unsigned int kBufferResource = 1;
constexpr unsigned int kNone = 0xFFFFFFFFU;
// Buffers are declared after the texture registers.
constexpr unsigned long kBufferRegisterBase = 12;
// Pixel outputs are numbered down from render-target slot 7.
constexpr unsigned long kLastPixelOutput = 7;

// Table at 0x192F260, read through 0x63E5D0.
constexpr const char* kProgramTypeNames[kNumShaderProgramTypes] = {
    "HX_PROGRAM_TYPE_VERTEX",
    "HX_PROGRAM_TYPE_HULL",
    "HX_PROGRAM_TYPE_DOMAIN",
    "HX_PROGRAM_TYPE_GEOMETRY",
    "HX_PROGRAM_TYPE_PIXEL",
    "HX_PROGRAM_TYPE_COMPUTE",
};

const Symbol& EmptySymbol() {
    static const Symbol empty("");
    return empty;
}

void Print(unsigned int& hash, const char* text) {
    CrcPrint(hash, text);
}

void Print(unsigned int& hash, unsigned long value) {
    CrcPrintUnsigned(hash, value);
}

// The original passes possibly-null text straight to the stream; no
// declared resource reaches that case.
void PrintOptional(unsigned int& hash, const char* text) {
    if (text != nullptr) {
        Print(hash, text);
    }
}

// Metal declarations keep the define and its declaration on one line.
const char* Separator(bool metal) {
    return metal ? " " : "\n";
}

const char* TextureMacro(
    const RndShaderResourceConfig::ResourceInfo& info,
    bool write) {
    const bool sliced = static_cast<unsigned char>(info.mRTSliced) != 0;
    switch (info.mTextureType) {
    case 0:
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTexture1D"
                     : "HX_TEXTURE_DEF(HxTexture1D";
    case 1:
        if (sliced) {
            return "HX_TEXTURE_DEF(HxTexture2DRTSliced";
        }
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTexture2D"
                     : "HX_TEXTURE_DEF(HxTexture2D";
    case 2:
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTexture3D"
                     : "HX_TEXTURE_DEF(HxTexture3D";
    case 3:
        return "HX_TEXTURE_DEF(HxTextureCube";
    case 4:
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTextureArray1D"
                     : "HX_TEXTURE_DEF(HxTextureArray1D";
    case 5:
        if (sliced) {
            return "HX_TEXTURE_DEF(HxTextureArray2DRTSliced";
        }
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTextureArray2D"
                     : "HX_TEXTURE_DEF(HxTextureArray2D";
    case 7:
        return "HX_TEXTURE_DEF(HxTextureArrayCube";
    default:
        return nullptr;
    }
}

void PrintResourceTail(
    unsigned int& hash,
    const RndShaderResourceConfig::ResourceInfo& info) {
    Print(hash, info.mName);
    Print(hash, ", ");
    Print(hash, info.mIndex);
    Print(hash, ", ");
    Print(hash, info.mRegister);
    Print(hash, ")\n");
}

// Reconstructed from eboot.elf at 0x644AB0. Each distinct sampler is
// declared once, numbered in first-use order.
void PrintSamplers(
    unsigned int& hash,
    const eastl::vector<RndShaderResourceConfig::ResourceInfo>& textures,
    bool metal) {
    unsigned long sampler = 0;
    for (unsigned long i = 0; i < textures.size(); ++i) {
        const auto& info = textures[i];
        bool seen = false;
        for (unsigned long j = 0; j < i; ++j) {
            if (textures[j].mSamplerName == info.mSamplerName) {
                seen = true;
                break;
            }
        }
        if (seen) {
            continue;
        }
        Print(hash, "#  define HX_USE_SAMPLER_");
        Print(hash, info.mSamplerName);
        Print(hash, Separator(metal));
        Print(hash, "HX_SAMPLER_DEF(");
        Print(hash, info.mSamplerName);
        Print(hash, ", ");
        Print(hash, sampler++);
        Print(hash, ", ");
        Print(hash, info.mRegister);
        Print(hash, ")\n");
    }
}

}  // namespace

RndShaderResourceConfig::RndShaderResourceConfig() : mNumRegisters{} {}

unsigned long RndShaderResourceConfig::_NumReadResources(
    RndShaderProgramType type) const {
    return mResources[kSampledTextures + type].size() +
        mResources[kTextures + type].size() +
        mResources[kInputBuffers + type].size();
}

// Reconstructed from eboot.elf at 0x643260. Textures without a sampler go
// to the unsampled list.
unsigned long RndShaderResourceConfig::AddTexture(
    const char* name,
    const char* sampler,
    unsigned int textureType,
    RndShaderProgramType type,
    RndShaderNumericType numericType) {
    if (type >= kNumShaderProgramTypes) {
        return 0;
    }
    const Symbol nameSymbol(name);
    const Symbol samplerSymbol(sampler);
    const auto index = _NumReadResources(type);
    auto& list = mResources[
        (samplerSymbol == EmptySymbol() ? kTextures : kSampledTextures) + type];
    list.emplace_back(ResourceInfo{
        kTextureResource,
        textureType,
        numericType,
        -1,
        nameSymbol.Str(),
        samplerSymbol.Str(),
        EmptySymbol().Str(),
        0,
        index,
        mNumRegisters[type]++,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x6438D0. A pixel texture with one slice
// per render-target slice.
unsigned long RndShaderResourceConfig::AddTexture2DRTSliced(
    const char* name,
    const char* sampler,
    unsigned int textureType,
    RndShaderNumericType numericType) {
    const Symbol nameSymbol(name);
    const Symbol samplerSymbol(sampler);
    const auto index = _NumReadResources(kShaderProgramPixel);
    auto& list = mResources[
        (samplerSymbol == EmptySymbol() ? kTextures : kSampledTextures) +
        kShaderProgramPixel];
    list.emplace_back(ResourceInfo{
        kTextureResource,
        textureType,
        numericType,
        -1,
        nameSymbol.Str(),
        samplerSymbol.Str(),
        EmptySymbol().Str(),
        1,
        index,
        mNumRegisters[kShaderProgramPixel]++,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x643670.
unsigned long RndShaderResourceConfig::AddTextureWritable(
    const char* name,
    unsigned int textureType,
    RndShaderProgramType type,
    RndShaderNumericType numericType) {
    if (type >= kNumShaderProgramTypes) {
        return 0;
    }
    auto& list = mResources[kWritableResources + type];
    const auto count = list.size();
    const auto index =
        type == kShaderProgramPixel ? kLastPixelOutput - count : count;
    const Symbol nameSymbol(name);
    list.emplace_back(ResourceInfo{
        kTextureResource,
        textureType,
        numericType,
        -1,
        nameSymbol.Str(),
        EmptySymbol().Str(),
        EmptySymbol().Str(),
        0,
        index,
        mNumRegisters[type]++,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x643C70.
unsigned long RndShaderResourceConfig::AddComputeBuffer(
    const char* name,
    unsigned int usage,
    RndShaderNumericType numericType,
    RndShaderProgramType type) {
    if (type >= kNumShaderProgramTypes) {
        return 0;
    }
    const auto index = _NumReadResources(type);
    const Symbol nameSymbol(name);
    mResources[kInputBuffers + type].emplace_back(ResourceInfo{
        kBufferResource,
        kNone,
        numericType,
        static_cast<int>(usage),
        nameSymbol.Str(),
        EmptySymbol().Str(),
        EmptySymbol().Str(),
        0,
        index,
        mNumRegisters[kNumShaderProgramTypes + type]++ + kBufferRegisterBase,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x643EF0.
unsigned long RndShaderResourceConfig::AddComputeBufferWritable(
    const char* name,
    unsigned int usage,
    RndShaderNumericType numericType,
    RndShaderProgramType type) {
    if (type >= kNumShaderProgramTypes) {
        return 0;
    }
    auto& list = mResources[kWritableResources + type];
    const auto count = list.size();
    const auto index =
        type == kShaderProgramPixel ? kLastPixelOutput - count : count;
    const Symbol nameSymbol(name);
    list.emplace_back(ResourceInfo{
        kBufferResource,
        kNone,
        numericType,
        static_cast<int>(usage),
        nameSymbol.Str(),
        EmptySymbol().Str(),
        EmptySymbol().Str(),
        0,
        index,
        mNumRegisters[kNumShaderProgramTypes + type]++ + kBufferRegisterBase,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x644150.
unsigned long RndShaderResourceConfig::AddComputeBufferCustomTyped(
    const char* name,
    const char* structName,
    unsigned int usage,
    RndShaderProgramType type) {
    if (type >= kNumShaderProgramTypes) {
        return 0;
    }
    const auto index = _NumReadResources(type);
    const Symbol nameSymbol(name);
    const Symbol structSymbol(structName);
    mResources[kInputBuffers + type].emplace_back(ResourceInfo{
        kBufferResource,
        kNone,
        kNone,
        static_cast<int>(usage),
        nameSymbol.Str(),
        EmptySymbol().Str(),
        structSymbol.Str(),
        0,
        index,
        mNumRegisters[kNumShaderProgramTypes + type]++ + kBufferRegisterBase,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x644400.
unsigned long RndShaderResourceConfig::AddComputeBufferCustomTypedWritable(
    const char* name,
    const char* structName,
    unsigned int usage,
    RndShaderProgramType type) {
    if (type >= kNumShaderProgramTypes) {
        return 0;
    }
    auto& list = mResources[kWritableResources + type];
    const auto count = list.size();
    const auto index =
        type == kShaderProgramPixel ? kLastPixelOutput - count : count;
    const Symbol nameSymbol(name);
    const Symbol structSymbol(structName);
    list.emplace_back(ResourceInfo{
        kBufferResource,
        kNone,
        kNone,
        static_cast<int>(usage),
        nameSymbol.Str(),
        EmptySymbol().Str(),
        structSymbol.Str(),
        0,
        index,
        mNumRegisters[kNumShaderProgramTypes + type]++ + kBufferRegisterBase,
    });
    return index;
}

// Reconstructed from eboot.elf at 0x644C80.
void RndShaderResourceConfig::_PrintTexture(
    const ResourceInfo& info,
    bool write,
    bool metal,
    unsigned int& hash) const {
    if (info.mIndex == static_cast<unsigned long>(-1)) {
        return;
    }
    Print(hash, "#  define HX_USE_TEXTURE_");
    Print(hash, info.mName);
    Print(hash, Separator(metal));
    PrintOptional(hash, TextureMacro(info, write));
    Print(hash, ", ");
    const auto numericType =
        static_cast<RndShaderNumericType>(info.mNumericType);
    PrintOptional(hash, RndShaderUtl::NumericTypeToHlsl(numericType));
    Print(hash, ", ");
    PrintOptional(
        hash,
        RndShaderUtl::NumericTypeToHlsl(
            RndShaderUtl::GetNumericTypeBaseType(numericType)));
    Print(hash, ", ");
    PrintResourceTail(hash, info);
}

// Reconstructed from eboot.elf at 0x644E40. Custom-typed buffers name their
// structure in place of a numeric type.
void RndShaderResourceConfig::_PrintComputeBuffer(
    const ResourceInfo& info,
    bool write,
    bool metal,
    unsigned int& hash) const {
    if (info.mIndex == static_cast<unsigned long>(-1)) {
        return;
    }
    const char* macro = nullptr;
    if (info.mUsage == 0) {
        macro = write ? "HX_BUFFER_WRITE_DEF" : "HX_BUFFER_DEF";
    } else if (info.mUsage == 1) {
        macro = "HX_BUFFER_APPEND_DEF";
    }
    const auto* element = info.mNumericType != kNone
        ? RndShaderUtl::NumericTypeToHlsl(
              static_cast<RndShaderNumericType>(info.mNumericType))
        : info.mStructName;
    Print(hash, "#  define HX_USE_BUFFER_");
    Print(hash, info.mName);
    Print(hash, Separator(metal));
    PrintOptional(hash, macro);
    Print(hash, "(");
    PrintOptional(hash, element);
    Print(hash, ", ");
    PrintResourceTail(hash, info);
}

void RndShaderResourceConfig::_PrintResourceVec(
    const eastl::vector<ResourceInfo>& resources,
    bool write,
    bool metal,
    unsigned int& hash) const {
    for (const auto& info : resources) {
        if (info.mKind == kBufferResource) {
            _PrintComputeBuffer(info, write, metal, hash);
        } else if (info.mKind == kTextureResource) {
            _PrintTexture(info, write, metal, hash);
        }
    }
}

void RndShaderResourceConfig::PrintCode(unsigned int& hash) const {
    for (unsigned int type = 0; type < kNumShaderProgramTypes; ++type) {
        const unsigned int lists[] = {
            kSampledTextures, kTextures, kInputBuffers, kWritableResources,
        };
        bool any = false;
        for (const auto list : lists) {
            any = any || !mResources[list + type].empty();
        }
        if (!any) {
            continue;
        }
        Print(hash, "#if (HX_PROGRAM_TYPE == ");
        Print(hash, kProgramTypeNames[type]);
        Print(hash, ")\n");
        for (int pass = 0; pass < 2; ++pass) {
            const bool metal = pass == 0;
            Print(hash, metal ? "# if (HX_METAL == 1)\n" : "# else\n");
            PrintSamplers(hash, mResources[kSampledTextures + type], metal);
            _PrintResourceVec(mResources[kSampledTextures + type], false, metal, hash);
            _PrintResourceVec(mResources[kTextures + type], false, metal, hash);
            _PrintResourceVec(mResources[kInputBuffers + type], false, metal, hash);
            _PrintResourceVec(mResources[kWritableResources + type], true, metal, hash);
        }
        Print(hash, "# endif // ...HX_METAL\n");
        Print(hash, "#endif // ...HX_PROGRAM_TYPE\n");
    }
}
