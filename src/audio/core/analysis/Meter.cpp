#include "audio/core/analysis/Meter.h"

// Meter.o holds only the static initializer (0xD30F0) that builds this lock;
// the code before it in this build (0xD2FC0 to 0xD30EF) belongs to the
// preceding object.
CritSec Meter::sAudioMeterCritSec;
