#include "math/easing/Easing.h"

#include <string.h>

#include "entity/waveform/Key.h"
#include "os/system/System.h"
#include "utl/data/DataArray.h"

// The curve settings, at 0x19B0330, 0x19E6578, 0x19E657C, 0x19B0334 and
// 0x19B0338. Names not in the reference map. The elastic settings start as
// NaN until the configuration sets them.
float gEaseBackOvershoot = 2.0F;
float gEaseElasticAmplitude = __builtin_nanf("");
float gEaseElasticPeriod = __builtin_nanf("");
float gEaseStairStepPower = 5.0F;
float gEasePolynomialPower = 6.0F;

// Reconstructed from eboot.elf at 0x2122F0.
void InitEasingParameters() {
    DataArray* config = SystemConfig(Symbol("math"), Symbol("easing_parameters"));
    config->FindData(Symbol("ease_back_overshoot"), gEaseBackOvershoot, true);
    config->FindData(Symbol("ease_elastic_amplitude"), gEaseElasticAmplitude, true);
    config->FindData(Symbol("ease_elastic_period"), gEaseElasticPeriod, true);
    config->FindData(Symbol("ease_stair_step_power"), gEaseStairStepPower, true);
    config->FindData(Symbol("ease_polynomial_power"), gEasePolynomialPower, true);
}

// The labels of the interpolations from -1 ("Invalid") to the last stair
// step, at 0x18EEC60. Name not in the reference map.
static const char* const sInterpolationNames[] = {
    "Invalid",
    "None",
    "Linear",
    "Spline",
    "Hermite",
    "Sigmoid",
    "Quadratic In",
    "Quadratic Out",
    "Quadratic In/Out",
    "Quadratic Out/In",
    "Cubic In",
    "Cubic Out",
    "Cubic In/Out",
    "Cubic Out/In",
    "Quartic In",
    "Quartic Out",
    "Quartic In/Out",
    "Quartic Out/In",
    "Quintic In",
    "Quintic Out",
    "Quintic In/Out",
    "Quintic Out/In",
    "Polynomial In",
    "Polynomial Out",
    "Polynomial In/Out",
    "Polynomial Out/In",
    "Back In",
    "Back Out",
    "Back In/Out",
    "Back Out/In",
    "Bounce In",
    "Bounce Out",
    "Bounce In/Out",
    "Bounce Out/In",
    "Circular In",
    "Circular Out",
    "Circular In/Out",
    "Circular Out/In",
    "Elastic In",
    "Elastic Out",
    "Elastic In/Out",
    "Elastic Out/In",
    "Exponential In",
    "Exponential Out",
    "Exponential In/Out",
    "Exponential Out/In",
    "Sine In",
    "Sine Out",
    "Sine In/Out",
    "Sine Out/In",
    "Half Stair Step",
    "Third Stair Step",
    "Quarter Stair Step",
    "Half/Quarter Stair Step",
    "Quarter/Half Stair Step",
};

// Reconstructed from eboot.elf at 0x212420.
const char* KeyTrackBase::InterpolationToString(Interpolation interpolation) {
    const unsigned int index = static_cast<unsigned int>(interpolation + 1);
    if (index < sizeof(sInterpolationNames) / sizeof(sInterpolationNames[0])) {
        return sInterpolationNames[index];
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x212440.
KeyTrackBase::Interpolation KeyTrackBase::StringToInterpolation(const char* name) {
    for (int interpolation = -1; interpolation <= kInterpolationQuarterHalfStairStep; ++interpolation) {
        if (strcmp(name, InterpolationToString(static_cast<Interpolation>(interpolation))) == 0) {
            return static_cast<Interpolation>(interpolation);
        }
    }
    return static_cast<Interpolation>(-1);
}
