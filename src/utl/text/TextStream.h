#pragma once

// Text output sink (utl/TextStream.o). Subclasses implement Print; the
// stream operators format their value and pass the text to it. Only the
// members the reconstructed code uses are declared. The vtable has three
// slots.
class TextStream {
public:
    TextStream();  // 0x258550

    // Slots 0-1: 0x258B60, 0x258B70. Inline in the map's build.
    virtual ~TextStream() {}
    // Slot 2: writes the text.
    virtual void Print(const char* str) = 0;

    // Each operator formats its value into a 1024-character buffer and
    // prints it.
    TextStream& operator<<(char c);              // 0x258560
    TextStream& operator<<(int value);           // 0x258640
    TextStream& operator<<(const char* str);     // 0x2588E0
    TextStream& operator<<(unsigned long value);  // 0x2589D0
};

static_assert(sizeof(TextStream) == 8);
