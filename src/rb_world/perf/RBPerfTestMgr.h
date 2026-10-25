#pragma once

// The automated performance tests (rb_world/RBPerfTestMgr.o). Only what the
// reconstructed code calls is declared; the class is not reconstructed.
class RBPerfTestMgr {
public:
    // Whether the "perf_test_enabled" data variable is set.
    static bool PerfTestModeEnabled();  // 0x1017860
};
