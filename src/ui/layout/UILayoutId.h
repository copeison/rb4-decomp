#pragma once

#include "utl/text/Symbol.h"

// Numeric UI layout IDs. The reference map's UI loads layouts by
// ResourcePath and has no layout ID enum, so this enum and its helpers were
// added after the map's build. Names not in the reference map; the
// enumerator names are the strings UILayoutIdToSymbol interns.
enum UILayoutId {
    kLayoutInvalid = -1,
#define UI_LAYOUT_ID(name, path) name,
#include "ui/layout/UILayoutId.inc"
#undef UI_LAYOUT_ID
};

static_assert(sizeof(UILayoutId) == 4);

// Name not in the reference map.
Symbol UILayoutIdToSymbol(UILayoutId id);  // 0xBB06A0

// Returns the first layout asset loaded directly for this ID, or nullptr when
// the executable routes the ID through fallback or deprecated handling.
// Name not in the reference map.
const char* UILayoutIdPrimaryPath(UILayoutId id);
