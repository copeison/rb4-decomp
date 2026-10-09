#include "utl/licenses/Licenses.h"

namespace {

// The list is set up by the first notice, whichever static initializer
// runs first: the magic word marks the head as valid. At 0x19E7CA0 and
// 0x19E7CA8. Names not in the reference map.
unsigned int gLicensesMagic;
Licenses* gLicensesHead;

constexpr unsigned int kLicensesMagic = 0xFEEDBACC;

}  // namespace

// Reconstructed from eboot.elf at 0x247110.
Licenses::Licenses(const char* name, const char* text, Requirement requirement)
    : mRequirement(requirement), mName(name), mText(text) {
    if (gLicensesMagic != kLicensesMagic) {
        gLicensesMagic = kLicensesMagic;
        gLicensesHead = nullptr;
    }
    mNext = gLicensesHead;
    gLicensesHead = this;
}

// Reconstructed from eboot.elf at 0x247160.
void Licenses::PrintAll() {
    for (Licenses* notice = gLicensesHead; notice != nullptr; notice = notice->mNext) {
    }
    for (Licenses* notice = gLicensesHead; notice != nullptr; notice = notice->mNext) {
    }
}
