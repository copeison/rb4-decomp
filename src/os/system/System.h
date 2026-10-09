#pragma once

// Engine system startup and per-frame service (os/System.o). Only the entry
// points the main loop calls are declared so far.

#include "utl/text/Symbol.h"

class DataArray;

// Loads the main configuration and installs the system config handlers.
void SystemInit(const char* config_path);  // 0x368000
// The array at the path in the system configuration.
DataArray* SystemConfig(Symbol key);                          // 0x368B00
DataArray* SystemConfig(Symbol key1, Symbol key2);            // 0x368CD0
DataArray* SystemConfig(Symbol key1, Symbol key2, Symbol key3);  // 0x369A90
// Updates the core runtime, input assignments, platform services, timers and
// file state once per frame.
void SystemPoll();  // 0x369940
