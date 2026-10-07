#pragma once

#include <cstddef>

namespace rb4 {

struct CommandLineArgument {
    const char* text;
    bool handled;
};

struct CommandLineArguments {
    CommandLineArgument* first;
    CommandLineArgument* last;
    CommandLineArgument* capacity;
};

static_assert(offsetof(CommandLineArgument, handled) == 8);
static_assert(sizeof(CommandLineArgument) == 16);

void command_line_mark_switches_handled(CommandLineArguments& arguments);

}  // namespace rb4
