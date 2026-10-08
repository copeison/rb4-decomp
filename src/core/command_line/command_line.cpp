#include "core/command_line/command_line.h"

#include <cstring>

namespace rb4 {

// Reconstructed from eboot.elf at 0x252660.
const char* command_line_switch_value(
    CommandLineArguments& arguments,
    const char* name,
    const char* default_value) {
    for (auto* argument = arguments.first;
         argument != arguments.last;
         ++argument) {
        if (argument->text[0] == '-' &&
            std::strcmp(argument->text + 1, name) == 0) {
            argument->handled = true;
            return (argument + 1)->text;
        }
    }
    return default_value;
}

// Reconstructed from eboot.elf at 0x252BC0.
void command_line_mark_switches_handled(CommandLineArguments& arguments) {
    for (auto* argument = arguments.first; argument != arguments.last; ++argument) {
        if (argument->text[0] == '-' && !argument->handled) {
            argument->handled = true;
        }
    }
}

}  // namespace rb4
