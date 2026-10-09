#pragma once

#include <cstdint>

#include "utl/text/Symbol.h"

namespace rb4 {

enum class UiLayoutId : std::int32_t {
    kLayoutInvalid = -1,
#define RB4_UI_LAYOUT(name, path) name,
#include "ui/layout/ui_layout_list.inc"
#undef RB4_UI_LAYOUT
};

Symbol ui_layout_id_to_symbol(UiLayoutId id);

// Returns the first layout asset loaded directly for this ID, or nullptr when
// the executable routes the ID through fallback/deprecated handling.
const char* ui_layout_primary_path(UiLayoutId id);

}  // namespace rb4
