#pragma once

// Helpers for the editor's in-scene drawing. Only the startup and shutdown
// entry points that Rnd::Init and Rnd::Terminate call are declared.
class RndEditorDrawUtl {
public:
    // Loads the control-point texture and builds the arrow mesh.
    static void Init();  // 0x461D10
    static void Terminate();  // 0x461F50
};
