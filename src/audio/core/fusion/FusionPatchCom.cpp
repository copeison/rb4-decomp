#include "audio/core/fusion/FusionPatchCom.h"

// Reconstructed from eboot.elf at 0x76A60. The binary reads the note and
// velocity limits as signed bytes (the map's AddKeyZone takes them as signed
// char), so the casts keep its comparisons.
bool FusionPatchCom::KeyzoneSettings::ContainsNoteAndVelocity(
    unsigned char note, unsigned char velocity) const {
    return note >= static_cast<signed char>(mMinNote) && note <= static_cast<signed char>(mMaxNote) &&
           velocity >= static_cast<signed char>(mMinVelocity) &&
           velocity <= static_cast<signed char>(mMaxVelocity);
}
