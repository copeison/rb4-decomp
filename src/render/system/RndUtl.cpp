#include "render/system/RndUtl.h"

#include "render/drawing/RndDrawInstanceCom.h"

namespace {

// The level names (0x1901E10). Name not in the reference map.
const char* const kLodNames[kNumSceneLods] = {
    "High",
    "Medium",
    "Low",
};

}  // namespace

// Reconstructed from eboot.elf at 0x4423A0.
const char* RndUtl::GetLodName(RndSceneLod lod) {
    return lod < kNumSceneLods ? kLodNames[lod] : nullptr;
}
