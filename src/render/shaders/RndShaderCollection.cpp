#include "render/shaders/RndShaderCollection.h"

#include "render/context/RndContext.h"
#include "render/system/RndInit.h"
#include "render/shaders/RndShaderProgram.h"
#include "render/system/RndDevice.h"
#include "utl/streams/BinStream.h"

namespace {

constexpr unsigned int kMinimumCacheVersion = 3;
constexpr unsigned int kMinimumProgramTypes = 4;

// Table at 0x12A9D10: the stage-mask bit, and key, of each program type.
constexpr unsigned int kProgramStageBits[kNumShaderProgramTypes] = {
    0, 1, 1, 2, 3, 4,
};

unsigned int ReadWord(BinStream& stream) {
    unsigned int value = 0;
    stream.ReadEndian(&value, sizeof(value));
    return value;
}

// Reconstructed from eboot.elf at 0x63C2B0. Each record starts with a
// discarded word, then its key.
RndShaderKey ReadKey(BinStream& stream) {
    unsigned int discarded = 0;
    stream.ReadEndian(&discarded, sizeof(discarded));
    RndShaderKey key = 0;
    stream.ReadEndian(&key, sizeof(key));
    return key;
}

bool UsesStage(unsigned int stages, unsigned int bit) {
    return ((stages >> bit) & 1U) != 0;
}

}  // namespace

RndShaderCollection::~RndShaderCollection() {
    Free();
}

void RndShaderCollection::Free() {
    for (auto& programs : mPrograms) {
        for (auto* program : programs) {
            delete program;
        }
        programs.clear();
    }
}

// Reconstructed from eboot.elf at 0x63B2B0. The cache stores a version and a
// program-type count, then each type's programs: key, binary size, and the
// binary the platform program reads directly from the stream.
bool RndShaderCollection::Load(BinStream& stream, const char* path) {
    Free();
    if (ReadWord(stream) < kMinimumCacheVersion) {
        return false;
    }
    if (ReadWord(stream) < kMinimumProgramTypes) {
        return false;
    }

    for (unsigned int type = 0; type < kNumShaderProgramTypes; ++type) {
        auto& programs = mPrograms[type];
        const auto count = ReadWord(stream);
        programs.resize(count);
        for (unsigned int i = 0; i < count; ++i) {
            const auto key = ReadKey(stream);
            const auto size = ReadWord(stream);

            // The original compares the active render API with itself; this
            // build's caches never hold another platform's programs.
            if (Rnd::PlatformGfxApi() != Rnd::PlatformGfxApi()) {
                stream.Seek(size, kSeekCur);
                continue;
            }

            auto* program =
                RndShaderProgram::New(static_cast<RndShaderProgramType>(type));
            stream.Tell();
            if (!program->Create(key, size == 0 ? nullptr : &stream, path)) {
                delete program;
                return false;
            }
            stream.Tell();
            programs[i] = program;
        }
    }
    return true;
}

RndShaderProgram* RndShaderCollection::_GetShaderProgram(
    eastl::vector<RndShaderProgram*>& programs,
    RndShaderKey key) {
    auto* first = programs.begin();
    auto count = static_cast<long>(programs.size());
    while (count > 0) {
        const auto half = count / 2;
        if (first[half]->mKey < key) {
            first += half + 1;
            count -= half + 1;
        } else {
            count = half;
        }
    }
    return first == programs.end() ? nullptr : *first;
}

// Reconstructed from eboot.elf at 0x63B500. Stages the shader uses become
// active and stages it does not are deactivated; each used stage then selects
// its program, stamped with the frame count. The original only looks up the
// stage and shading-mode names for a stripped diagnostic on failure.
bool RndShaderCollection::Select(
    RndContext& context,
    unsigned int stages,
    const RndShaderKeyGroup& keys) {
    auto& active = context.mActiveShaderStages;
    for (unsigned int type = 0; type < kNumShaderProgramTypes; ++type) {
        const auto bit = static_cast<unsigned char>(1U << type);
        if (UsesStage(stages, kProgramStageBits[type])) {
            active = static_cast<unsigned char>(active | bit);
        } else if ((active & bit) != 0) {
            context.DeactivateShaderProgramType(
                static_cast<RndShaderProgramType>(type));
        }
    }

    const auto frame = static_cast<long>(TheRndDevice()->mFrameCount);
    for (unsigned int type = 0; type < kNumShaderProgramTypes; ++type) {
        const auto bit = kProgramStageBits[type];
        if (!UsesStage(stages, bit)) {
            continue;
        }
        const auto key = keys.mKeys[bit];
        auto* program = _GetShaderProgram(mPrograms[type], key);
        if (program == nullptr || program->mKey != key || !program->mCreated) {
            return false;
        }
        program->mLastSelectFrame = frame;
        program->_SelectImpl(context);
    }
    return true;
}
