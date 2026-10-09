#pragma once

// Engine system startup and per-frame service (os/System.o). Only the entry
// points the main loop calls are declared so far.

// Loads the main configuration and installs the system config handlers.
void SystemInit(const char* config_path);  // 0x368000
// Updates the core runtime, input assignments, platform services, timers and
// file state once per frame.
void SystemPoll();  // 0x369940
