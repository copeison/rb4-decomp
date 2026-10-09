#include "audio/core/decoders/AudioDecoder.h"

// Reconstructed from eboot.elf at 0xD4540. The CanDecode result is not
// used; the call is what remains of a check.
AudioDecoder* AudioDecoder::NewDecoderForFormat(AudioData::EncodedFormat format) {
    AudioDecoder* decoder;
    switch (format) {
    case AudioData::kEncodedPcm:
        decoder = new PcmAudioDecoder();
        break;
    case AudioData::kEncodedMogg:
        decoder = new MoggAudioDecoder();
        break;
    default:
        return nullptr;
    }
    decoder->CanDecode(format);
    return decoder;
}
