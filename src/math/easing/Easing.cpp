#include "math/easing/Easing.h"

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
