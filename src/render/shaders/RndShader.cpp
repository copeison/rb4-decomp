#include "render/shaders/RndShader.h"

#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "render/context/RndContext.h"
#include "render/system/RndCapabilities.h"
#include "render/shaders/RndShaderIncludeChecksums.h"
#include "render/shaders/RndShaderUtl.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderError.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "utl/streams/FileStream.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

namespace {

constexpr unsigned long kCBufferSlot = 8;
constexpr std::size_t kActivePlatformConfig = 7;
constexpr unsigned int kSixSliceFeature = 0x4;
// Only the vertex, tessellation, geometry, and pixel permutations are hashed.
constexpr unsigned int kHashedProgramTypes = 5;

// Serializes program loading. The static initializer at 0x639810 creates it.
// Name not in the reference map.
CritSec gShaderLoadCritSec;

// Render-target slices per context slice mode, from the table at 0x12A99A0.
constexpr unsigned int kSliceCounts[] = {1, 2, 6, 1, 1, 1, 1, 1, 1, 1, 1};

RndShaderMgr& ShaderMgr() {
    return TheRndDevice()->mShaderMgr;
}

unsigned int ReadWord(BinStream& stream) {
    unsigned int value = 0;
    stream.ReadEndian(&value, sizeof(value));
    return value;
}

void HashWord(unsigned int& hash, unsigned int value) {
    for (unsigned int shift = 0; shift < 32; shift += 8) {
        CrcPrintByte(
            hash, static_cast<unsigned char>(value >> shift));
    }
}

void HashKey(unsigned int& hash, RndShaderKey key) {
    for (unsigned int shift = 0; shift < 64; shift += 8) {
        CrcPrintByte(
            hash, static_cast<unsigned char>(key >> shift));
    }
}

struct ChecksumVisit {
    RndShaderProgramType mType;
    unsigned int* mHash;
    const RndShader* mShader;
};

// Reconstructed from eboot.elf at 0x639640, the call operator of the
// _ChecksumDefines visitor (vtable 0x192EFC8). Only one- and six-slice
// permutations count; six slices need shader and platform support, and a
// single-slice geometry program must be declared by the shader. The shader
// has the final say.
void ChecksumPermutation(
    void* context,
    const RndShaderMacro*,
    unsigned long,
    RndShaderKey key) {
    auto& visit = *static_cast<ChecksumVisit*>(context);
    const auto& shader = *visit.mShader;
    const auto slices = shader.mNumRTSlices.GetValue(key);
    if (slices != 1 && slices != 6) {
        return;
    }
    if (slices >= 2) {
        if (!shader._SupportsRTSlicing()) {
            return;
        }
        const auto& platform =
            TheRndDevice()->mCapabilities[kActivePlatformConfig];
        if ((platform.mFeatureFlags & kSixSliceFeature) == 0) {
            return;
        }
    } else if (visit.mType == kShaderProgramGeometry &&
               !shader._UsesCustomGeometryShader()) {
        return;
    }
    if (!shader._UsesShaderKeyImpl(visit.mType, key)) {
        return;
    }
    HashKey(*visit.mHash, key);
}

// Reconstructed from eboot.elf at 0x50CA90 and the shrink path of 0x638A40.
// New entries have the empty name and value zero.
void ResizeIncludes(
    RndShaderIncludeChecksums::ChecksumArray& includes,
    std::size_t count) {
    const auto size = static_cast<std::size_t>(includes.mEnd - includes.mBegin);
    if (count <= size) {
        includes.mEnd = includes.mBegin + count;
        return;
    }
    const auto capacity =
        static_cast<std::size_t>(includes.mCapacity - includes.mBegin);
    if (count > capacity) {
        auto newCapacity = size == 0 ? std::size_t{1} : size * 2;
        if (newCapacity < count) {
            newCapacity = count;
        }
        auto* storage = static_cast<RndShaderIncludeChecksums::Checksum*>(
            HmxAllocator::gStlAllocator.allocate(
                newCapacity * sizeof(RndShaderIncludeChecksums::Checksum)));
        for (std::size_t i = 0; i < size; ++i) {
            storage[i] = includes.mBegin[i];
        }
        if (includes.mBegin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                includes.mBegin,
                capacity * sizeof(RndShaderIncludeChecksums::Checksum));
        }
        includes.mBegin = storage;
        includes.mEnd = storage + size;
        includes.mCapacity = storage + newCapacity;
    }
    const Symbol empty("");
    for (auto* include = includes.mEnd; include != includes.mBegin + count;
         ++include) {
        include->mName = empty.Str();
        include->mChecksum = 0;
    }
    includes.mEnd = includes.mBegin + count;
}

void FreeIncludes(RndShaderIncludeChecksums::ChecksumArray& includes) {
    if (includes.mBegin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            includes.mBegin,
            static_cast<std::size_t>(includes.mCapacity - includes.mBegin) *
                sizeof(RndShaderIncludeChecksums::Checksum));
    }
    includes = {};
}

}  // namespace

RndShader* RndShader::FromLink(RndShaderLink* link) {
    return reinterpret_cast<RndShader*>(
        reinterpret_cast<char*>(link) - offsetof(RndShader, mLink));
}

// Reconstructed from eboot.elf at 0x6380B0.
RndShader::RndShader()
    : mShaderStages(0),
      mCollectionLoaded(false),
      mFilePath(nullptr),
      mFixedDefines(nullptr),
      mDefines(nullptr),
      mCBufferConfig(nullptr),
      mResourceConfig(nullptr),
      mNumRTSlices{} {
    mLink.mNext = &mLink;
    mLink.mPrev = &mLink;
}

// Reconstructed from eboot.elf at 0x638110.
RndShader::~RndShader() {
    delete mFixedDefines;
    mFixedDefines = nullptr;
    delete mDefines;
    mDefines = nullptr;
    delete mCBufferConfig;
    mCBufferConfig = nullptr;
    delete mResourceConfig;
    mResourceConfig = nullptr;

    mLink.mNext->mPrev = mLink.mPrev;
    mLink.mPrev->mNext = mLink.mNext;
}

// Reconstructed from eboot.elf at 0x450850.
int RndShader::_GetLoadOption() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x6388E0.
int RndShader::_GetShaderStages() const {
    return kShaderStagesGraphics;
}

// Reconstructed from eboot.elf at 0x6388F0.
bool RndShader::_UsesShaderKeyImpl(RndShaderProgramType, RndShaderKey) const {
    return true;
}

// Reconstructed from eboot.elf at 0x638900.
void RndShader::_SelectErrorShader(RndContext& context) const {
    ShaderMgr().mErrorShader
        ->Select(context, kShaderGeoTypeDefault);
}

// Reconstructed from eboot.elf at 0x450870.
bool RndShader::_SupportsRTSlicing() const {
    return false;
}

// Reconstructed from eboot.elf at 0x450880.
bool RndShader::_UsesCustomGeometryShader() const {
    return false;
}

// Reconstructed from eboot.elf at 0x638270.
void RndShader::InitConfig() {
    if (mCBufferConfig != nullptr) {
        return;
    }
    mShaderStages = _GetShaderStages();
    mFixedDefines = new RndShaderFixedDefines;
    mDefines = new RndShaderDefinesGroup;
    mCBufferConfig = new RndShaderCBufferConfig(
        _GetClassNameImpl(), kCBufferSlot, mShaderStages, 0);
    mResourceConfig = new RndShaderResourceConfig;
    mNumRTSlices =
        mDefines->GetGlobalDefines().Add(Symbol("HX_NUM_RT_SLICES"), 0, 7);
    _InitConfigImpl(*mFixedDefines, *mDefines, *mCBufferConfig, *mResourceConfig);
}

// Reconstructed from eboot.elf at 0x6383D0.
void RndShader::Init() {
    InitConfig();
    const auto& params = TheRndDevice()->mInitParams;
    switch (_GetLoadOption()) {
    case 0:
        if (!params.mInitRendering) {
            return;
        }
        break;
    case 1:
        if (!params.mInitOptionalShaders) {
            return;
        }
        break;
    default:
        return;
    }
    _InitShaderCollection();
}

// Reconstructed from eboot.elf at 0x6388C0 and 0x63B210.
void RndShader::Reload() {
    mCollection.Free();
    mCollectionLoaded = false;
}

// Reconstructed from eboot.elf at 0x638A20.
void RndShader::_Register() {
    auto& mgr = ShaderMgr();
    auto* anchor = mgr.mShaders;
    auto* last = anchor->mPrev;
    mLink.mNext = anchor;
    mLink.mPrev = last;
    last->mNext = &mLink;
    anchor->mPrev = &mLink;
    if (mgr.mInitialized != 0) {
        Init();
    }
}

// Reconstructed from eboot.elf at 0x638430. Retail builds cannot compile
// shaders: a missing, stale, or mismatched cache falls back to loading the
// generated cache unvalidated. In archive mode caches are trusted. The
// stripped compile path's side-effect-free queries are omitted.
void RndShader::_InitShaderCollection() {
    ScopedCritSec lock(gShaderLoadCritSec);
    if (mCollectionLoaded) {
        return;
    }
    mFilePath = _GetShaderFilePath();

    bool rebuild = false;
    String generated("");
    FileStat stat{};
    Symbol source("");
    FileResolvePath(source, mFilePath);
    if (FileFindGenerated(source, "", generated, rebuild, stat)) {
        const bool archive = gFileArchiveMode != 0;
        bool failed = true;
        if (!rebuild || archive) {
            failed = !_LoadCached(generated.c_str(), !archive);
        }
        if (failed && !archive) {
            _LoadCached(generated.c_str(), false);
        }
    }
    mCollectionLoaded = true;
}

// Reconstructed from eboot.elf at 0x638A40. A cache starts with a non-zero
// marker, the stage mask, and four checksums, then the include checksums it
// was built with, then the programs. Unvalidated loads skip the checks.
bool RndShader::_LoadCached(const char* path, bool validate) {
    FileStream stream(path, kRead, false);
    if (stream.Fail()) {
        return false;
    }
    if (ReadWord(stream) == 0 ||
        static_cast<int>(ReadWord(stream)) != _GetShaderStages()) {
        return false;
    }
    const auto fixedChecksum = ReadWord(stream);
    const auto definesChecksum = ReadWord(stream);
    const auto configChecksum = ReadWord(stream);
    const auto sourceChecksum = ReadWord(stream);

    RndShaderIncludeChecksums::ChecksumArray includes{};
    unsigned int heap = 0;
    MemPushTemp(heap, true, true);
    ResizeIncludes(includes, ReadWord(stream));
    for (auto* include = includes.mBegin; include != includes.mEnd; ++include) {
        Symbol name(include->mName);
        stream >> name;
        include->mName = name.Str();
        stream.ReadEndian(&include->mChecksum, sizeof(include->mChecksum));
    }
    MemPopTemp(heap);

    bool valid = true;
    if (validate) {
        auto& mgr = ShaderMgr();
        valid = fixedChecksum ==
                static_cast<unsigned int>(mgr.mFixedDefinesChecksum) &&
            definesChecksum == _ChecksumDefines();
        if (valid) {
            auto hash = kCrcTextStreamBasis;
            mCBufferConfig->PrintCode(hash);
            mResourceConfig->PrintCode(hash);
            valid = configChecksum == hash &&
                sourceChecksum == RndShaderUtl::ChecksumSourceCodeFile(
                                      _GetShaderFilePath()) &&
                mgr.mIncludeChecksums->Check(includes);
        }
    }
    bool loaded = false;
    if (valid) {
        loaded = mCollection.Load(stream, mFilePath);
    }
    FreeIncludes(includes);
    return loaded;
}

// Reconstructed from eboot.elf at 0x638D70. Hashes everything that shapes the
// set of programs: the fixed defines, every define's name and range, and the
// key of each valid permutation of the graphics program types.
unsigned int RndShader::_ChecksumDefines() const {
    auto hash = kCrcTextStreamBasis;
    mFixedDefines->PrintCode(hash);

    for (const auto& defines : mDefines->mDefines) {
        HashWord(hash, static_cast<unsigned int>(defines.mEntries.size()));
        for (const auto& entry : defines.mEntries) {
            CrcPrint(hash, entry.mName.c_str());
            HashWord(hash, static_cast<unsigned int>(entry.mFirst));
            HashWord(hash, static_cast<unsigned int>(entry.mEnd));
        }
    }

    const auto stages = _GetShaderStages();
    for (unsigned int type = 0; type < kHashedProgramTypes; ++type) {
        const auto programType = static_cast<RndShaderProgramType>(type);
        if (!ShaderStagesInclude(stages, programType)) {
            continue;
        }
        ChecksumVisit visit{programType, &hash, this};
        mDefines->Visit(programType, ChecksumPermutation, &visit);
    }
    return hash;
}

// Reconstructed from eboot.elf at 0x638920. Single-slice draws drop the
// geometry program unless the shader declares one.
bool RndShader::_SelectShaderCollection(
    RndContext& context,
    RndShaderKeyGroup& keys) {
    if (!mCollectionLoaded) {
        _InitShaderCollection();
    }

    const auto mode = context.mTargetMode;
    unsigned int slices = 0;
    if (mode == -1) {
        slices = 1;
    } else if (static_cast<unsigned int>(mode) <
               sizeof(kSliceCounts) / sizeof(*kSliceCounts)) {
        slices = kSliceCounts[mode];
    }

    const auto field = static_cast<RndShaderKey>(
        (slices - static_cast<unsigned int>(mNumRTSlices.mFirst))
        << mNumRTSlices.mShift) << 32;
    const auto mask = static_cast<RndShaderKey>(mNumRTSlices.mMask) << 32;
    for (auto& key : keys.mKeys) {
        key = (key & ~mask) | field;
    }

    auto stages = mShaderStages;
    if (slices == 1 && !_UsesCustomGeometryShader()) {
        stages &= ~0x4;
    }
    if (mCollection.Select(context, static_cast<unsigned int>(stages), keys)) {
        return true;
    }
    _SelectErrorShader(context);
    return false;
}

// Reconstructed from eboot.elf at 0x63C1C0.
RndShaderCompute::RndShaderCompute() {}

// Reconstructed from eboot.elf at 0x63C1F0.
RndShaderCompute::~RndShaderCompute() {}

// Reconstructed from eboot.elf at 0x63C220.
int RndShaderCompute::_GetShaderStages() const {
    return kShaderStagesCompute;
}
