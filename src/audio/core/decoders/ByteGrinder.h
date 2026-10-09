#pragma once

// Registers the script functions of the Mogg key derivation and builds its
// tables (audio/ByteGrinder.o). At 0xC3BE0. The function is matched to the
// map's only ByteGrinder.o function by its position and its obfuscated
// one- and two-letter function names; the evidence is weak.
void ByteGrinderInit();
