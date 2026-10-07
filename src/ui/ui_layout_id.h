#pragma once

#include <cstdint>

#include "../core/symbol.h"

namespace rb4 {

enum class UiLayoutId : std::int32_t {
    kLayoutInvalid = -1,
#define RB4_UI_LAYOUT(name) name,
#include "ui_layout_list.inc"
#undef RB4_UI_LAYOUT
};

Symbol ui_layout_id_to_symbol(UiLayoutId id);

}  // namespace rb4
