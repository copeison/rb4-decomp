// rb_meta/RBMetaStateCom.o. Only RBUILayoutToSym is reconstructed; this
// build places it at 0xBB06A0, after the map's object start.
#include "rb_meta/state/RBMetaStateCom.h"

namespace {

constexpr int kNumRBUILayouts = kLayoutRockShopCustomizeTrackR2T + 1;
static_assert(kNumRBUILayouts == 108);

}  // namespace

// Reconstructed from eboot.elf at 0xBB06A0. The names are interned into a
// guarded function-local table on the first call.
Symbol RBUILayoutToSym(RBUILayout layout) {
    if (layout == kLayoutInvalid) {
        return Symbol("kLayoutInvalid");
    }

    static const Symbol sSymbols[kNumRBUILayouts] = {
#define RB_UI_LAYOUT(name, path) Symbol(#name),
#include "rb_meta/state/RBUILayout.inc"
#undef RB_UI_LAYOUT
    };
    return sSymbols[layout];
}
