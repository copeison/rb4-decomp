#include "utl/text/TextStream.h"

#include "utl/text/HmxSnprintf.h"

// Reconstructed from eboot.elf at 0x258550.
TextStream::TextStream() {}

// Reconstructed from eboot.elf at 0x258560.
TextStream& TextStream::operator<<(char c) {
    char buffer[1024];
    HmxSnprintf(buffer, sizeof(buffer), "%c", c);
    Print(buffer);
    return *this;
}

// Reconstructed from eboot.elf at 0x2588E0.
TextStream& TextStream::operator<<(const char* str) {
    Print(str);
    return *this;
}

// Reconstructed from eboot.elf at 0x2589D0.
TextStream& TextStream::operator<<(unsigned long value) {
    char buffer[1024];
    HmxSnprintf(buffer, sizeof(buffer), "%lu", value);
    Print(buffer);
    return *this;
}
