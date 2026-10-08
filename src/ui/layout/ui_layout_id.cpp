#include "ui/layout/ui_layout_id.h"

#include <array>
#include <cstddef>

namespace rb4 {

namespace {

constexpr std::size_t kUiLayoutCount =
    static_cast<std::size_t>(UiLayoutId::kLayoutRockShopCustomizeTrackR2T) + 1;

const std::array<Symbol, kUiLayoutCount>& ui_layout_symbols() {
    static const std::array<Symbol, kUiLayoutCount> symbols{
#define RB4_UI_LAYOUT(name, path) Symbol{#name},
#include "ui/layout/ui_layout_list.inc"
#undef RB4_UI_LAYOUT
    };
    return symbols;
}

const std::array<const char*, kUiLayoutCount>& ui_layout_paths() {
    static const std::array<const char*, kUiLayoutCount> paths{
#define RB4_UI_LAYOUT(name, path) path,
#include "ui/layout/ui_layout_list.inc"
#undef RB4_UI_LAYOUT
    };
    return paths;
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

// Reconstructed from the layout switch at eboot.elf 0xBB5D40.
const char* ui_layout_primary_path(UiLayoutId id) {
    const auto index = static_cast<std::int32_t>(id);
    if (index < 0 || static_cast<std::size_t>(index) >= kUiLayoutCount) {
        return nullptr;
    }
    return ui_layout_paths()[static_cast<std::size_t>(index)];
}

}  // namespace rb4
