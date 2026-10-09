#include "render/debug/overlays/RndTimersOverlay.h"

// Reconstructed from eboot.elf at 0x6E1F90.
RndCpuTimersOverlay::RndCpuTimersOverlay() : RndTimersOverlay("cpu_timers", 0) {}

// Reconstructed from eboot.elf at 0x6E1FC0 (deleting variant at 0x6E1FD0).
RndCpuTimersOverlay::~RndCpuTimersOverlay() {}

// Reconstructed from eboot.elf at 0x6E1FF0.
void RndCpuTimersOverlay::PrintHelp(TextStream& stream) {
    stream << "Displays CPU timers\n";
    RndTimersOverlay::PrintHelp(stream);
    stream << "Related commands:\n"
              "  expand_all_timers:       expands all timers\n"
              "  collapse_all_timers:     collapses all timers\n"
              "  toggle_entity_timers:    toggles per-entity timers on/off (on by default)\n"
              "  toggle_component_timers: toggles per-component timers on/off (off by default)\n";
}
