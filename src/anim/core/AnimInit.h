#pragma once

// The animation system's registration (anim/AnimInit.o, which also holds
// every anim class's inline Init and factory).
namespace Anim {

// Registers the AnimResource::Event struct and every animation component
// and resource class. The application calls it at startup.
void Init();  // 0x2A7080

}  // namespace Anim
