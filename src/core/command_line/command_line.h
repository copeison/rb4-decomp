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

extern CommandLineArguments arguments;

const char* command_line_switch_value(
    CommandLineArguments& arguments,
    const char* name,
    const char* default_value);
void command_line_mark_switches_handled(CommandLineArguments& arguments);

}  // namespace rb4
