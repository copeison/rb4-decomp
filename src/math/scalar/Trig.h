#pragma once

// Fills the 256-entry sine table that Sine and FastSin read. Called from
// core_initialize.
void TrigTableInit();  // 0x219610

// Does nothing; the table is static storage.
void TrigTableTerminate();  // 0x219680

// Table sine, linearly interpolated between the 256 samples. Callers take the
// cosine as Sine(x + pi/2).
float Sine(float angle);  // 0x219690

// Table sine without interpolation: rounds to the nearest sample.
float FastSin(float angle);  // 0x219710
