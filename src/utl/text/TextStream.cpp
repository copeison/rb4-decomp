#include "utl/text/TextStream.h"

// Reconstructed from eboot.elf at 0x258550.
TextStream::TextStream() {}

// Reconstructed from eboot.elf at 0x2588E0.
TextStream& TextStream::operator<<(const char* str) {
    Print(str);
    return *this;
}
