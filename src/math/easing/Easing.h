#pragma once

// The easing curves (math/Easing.o). Only the settings are reconstructed.

// Reads the math easing_parameters block of the system configuration:
// ease_back_overshoot, ease_elastic_amplitude and the other curve settings.
// Name not in the reference map.
void InitEasingParameters();  // 0x2122F0

// The curve settings InitEasingParameters reads. Names not in the reference
// map.
extern float gEaseBackOvershoot;
extern float gEaseElasticAmplitude;
extern float gEaseElasticPeriod;
extern float gEaseStairStepPower;
extern float gEasePolynomialPower;
