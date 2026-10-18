#pragma once

// UI system startup and shutdown (ui/UI.o).
namespace UI {
// Creates theUI, registers the layout resource and the UI component
// classes, and registers Terminate as an exit callback.
void Init();  // 0x891990
// Terminates and deletes theUI.
void Terminate();  // 0x894D20
}  // namespace UI
