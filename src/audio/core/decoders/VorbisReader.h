#pragma once

// Decoder of encrypted Ogg Vorbis (Mogg) streams (audio/VorbisReader.o). The
// class has not been reconstructed; only the member MoggGeneratorManager::Init
// calls is declared.
class VorbisReader {
public:
    // Registers thirteen single-letter script functions once, guarded by the
    // flag at 0x19C9908. At 0xD2D80. The map also has InitSecurity(), whose
    // size this body matches; the name rests on the caller and is weak.
    static void Init();
};
