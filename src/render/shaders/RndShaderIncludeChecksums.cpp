#include "render/shaders/RndShaderIncludeChecksums.h"

#include <cstring>

// Reconstructed from eboot.elf at 0x63EB70.
bool RndShaderIncludeChecksums::Check(const ChecksumArray& checksums) const {
    for (const auto* cached = checksums.mBegin; cached != checksums.mEnd;
         ++cached) {
        auto* first = mChecksums.mBegin;
        auto count = mChecksums.mEnd - mChecksums.mBegin;
        while (count > 0) {
            const auto half = count / 2;
            auto* middle = first + half;
            if (middle->mName != cached->mName &&
                strcasecmp(middle->mName, cached->mName) < 0) {
                first = middle + 1;
                count -= half + 1;
            } else {
                count = half;
            }
        }
        if (first == mChecksums.mEnd || first->mName != cached->mName ||
            first->mChecksum != cached->mChecksum) {
            return false;
        }
    }
    return true;
}
