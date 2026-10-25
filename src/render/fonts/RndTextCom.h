#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The text component (render/RndTextCom.o; its static initializer is at
// 0x67A3F0, class id "Text"). Only the class symbols the UI combo boxes
// name as their text objects' required component are declared; the class
// is not reconstructed.
class RndTextCom : public Component {
public:
    static Symbol sId;  // 0x1AAD878, "Text"
    // The class symbol GameObject::CreateComponent takes; also "Text". Name
    // not in the reference map.
    static Symbol sClassName;  // 0x1AAD880

    // Set the text and the localization token, marking the text for
    // rebuilding when the component shows them. Not reconstructed; the
    // names follow the map's order of the setters.
    void SetTextUtf8(const char* text);  // 0x66A120
    void SetTextToken(Symbol token);     // 0x66A1C0
};
