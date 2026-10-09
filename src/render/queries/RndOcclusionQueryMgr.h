#pragma once

// The registry that submits the occlusion queries. Only the startup and
// shutdown entry points that Rnd::Init and Rnd::Terminate call are
// declared.
class RndOcclusionQueryMgr {
public:
    // Builds the occlusion query sphere and its two shaders.
    static void Init();  // 0x5F7ED0
    static void Terminate();  // 0x5F7FB0
};
