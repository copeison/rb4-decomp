#include "core/command_line/command_line.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x252BC0.
void command_line_mark_switches_handled(CommandLineArguments& arguments) {
    for (auto* argument = arguments.first; argument != arguments.last; ++argument) {
        if (argument->text[0] == '-' && !argument->handled) {
            argument->handled = true;
        }
    }
}

}  // namespace rb4
