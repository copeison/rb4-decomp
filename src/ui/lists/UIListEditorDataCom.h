#pragma once

#include "render/editor/RndEditorDrawCom.h"

// The editor-draw counterpart of UIListCom ("RndEditorDrawCom counterpart
// to UIListCom"); its class symbol is "UIListEditorData" (0x1AC95B0). It
// follows UIListItemCom in the binary (0x8C7510 on), and the map, which is
// older, has no object for it. Only the registration UI::Init calls is
// declared; the class is not reconstructed. Name not in the reference
// map.
class UIListEditorDataCom : public RndEditorDrawCom {
public:
    // Registers the class: its property registry, metadata and factory.
    // Emitted in ui/UI.o at 0x894960. Not reconstructed: the registration
    // helpers it inlines are not modelled.
    static void Init();
};
