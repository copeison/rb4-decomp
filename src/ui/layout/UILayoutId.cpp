#include "ui/layout/UILayoutId.h"

namespace {

constexpr int kUILayoutCount = kLayoutRockShopCustomizeTrackR2T + 1;
static_assert(kUILayoutCount == 108);

}  // namespace

// Reconstructed from eboot.elf at 0xBB06A0. The names are interned into a
// guarded function-local table on the first call.
Symbol UILayoutIdToSymbol(UILayoutId id) {
    if (id == kLayoutInvalid) {
        return Symbol("kLayoutInvalid");
    }

    static const Symbol sSymbols[kUILayoutCount] = {
#define UI_LAYOUT_ID(name, path) Symbol(#name),
#include "ui/layout/UILayoutId.inc"
#undef UI_LAYOUT_ID
    };
    return sSymbols[id];
}

// Reconstructed from the layout switch at eboot.elf 0xBB5D40.
const char* UILayoutIdPrimaryPath(UILayoutId id) {
    static const char* const sPaths[kUILayoutCount] = {
#define UI_LAYOUT_ID(name, path) path,
#include "ui/layout/UILayoutId.inc"
#undef UI_LAYOUT_ID
    };

    if (id < 0 || id >= kUILayoutCount) {
        return nullptr;
    }
    return sPaths[id];
}
