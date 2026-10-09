#pragma once

// Game startup and main loop (rockband/Main.o). The map gives App no
// instance members, constructor or global, so its methods are static.
class App {
public:
    // Registers the game's types and services, initializes rendering and
    // loads the startup layout. The binary does not read argc or argv.
    static bool Initialize(int argc, char** argv);  // 0xA0
    // Runs one main-loop iteration. Returns whether another frame should run.
    static bool RunOneFrame();                      // 0x190
    // Inlined into main in this binary (0x3C0).
    static void Run(int argc, char** argv) {
        if (Initialize(argc, argv)) {
            while (RunOneFrame()) {
            }
        }
    }
};
