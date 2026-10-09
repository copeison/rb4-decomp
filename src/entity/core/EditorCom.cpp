#include "entity/core/EditorCom.h"

// Reconstructed from eboot.elf at 0xEABC0.
void EditorCom::GrantEditorCapability(GameObject* object, EditorCapability capability) {
    static_cast<void>(object);
    mCapabilities |= capability;
}

// Reconstructed from eboot.elf at 0xEABD0.
void EditorCom::RevokeEditorCapability(GameObject* object, EditorCapability capability) {
    static_cast<void>(object);
    mCapabilities &= ~static_cast<unsigned int>(capability);
}
