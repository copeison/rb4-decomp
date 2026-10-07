#include "ui_layout_id.h"

#include <array>
#include <cstddef>

namespace rb4 {

namespace {

constexpr std::size_t kUiLayoutCount =
    static_cast<std::size_t>(UiLayoutId::kLayoutRockShopCustomizeTrackR2T) + 1;

const std::array<Symbol, kUiLayoutCount>& ui_layout_symbols() {
    static const std::array<Symbol, kUiLayoutCount> symbols{
#define RB4_UI_LAYOUT(name) Symbol{#name},
#include "ui_layout_list.inc"
#undef RB4_UI_LAYOUT
    };
    return symbols;
}

}  // namespace

static_assert(kUiLayoutCount == 108);

// Reconstructed from eboot.elf at 0xBB06A0.
Symbol ui_layout_id_to_symbol(UiLayoutId id) {
    if (id == UiLayoutId::kLayoutInvalid) {
        return Symbol{"kLayoutInvalid"};
    }

    return ui_layout_symbols()[static_cast<std::size_t>(id)];
}

}  // namespace rb4
