#include "startup.h"

// Reconstructed from eboot.elf at 0x3C0.
// Names are descriptive until original symbols are recovered.

namespace rb4 {

// 0x3C0
int game_main(int argc, char** argv, char** envp) {
    (void)argc;
    (void)argv;
    (void)envp;

    if (game_initialize()) {
        while (game_run_frame()) {
        }
    }

    return 0;
}

}  // namespace rb4
