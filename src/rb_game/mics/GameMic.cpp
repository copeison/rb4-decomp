#include "rb_game/mics/GameMic.h"

#include "utl/streams/MemStream.h"

// Reconstructed from eboot.elf at 0xA390E0.
void GameMic::Reset() {}

// Reconstructed from eboot.elf at 0xA39120.
void GameMic::AccessContinuousSamples(const short*& samples, int& count) const {
    samples = mContinuousSamples;
    count = mContinuousCount;
}

// Reconstructed from eboot.elf at 0xA39140.
void GameMic::KillThread() {}

// Reconstructed from eboot.elf at 0xA39150.
void GameMic::StartDetection() {
    if (mInputSampleRate != 0) {
        mInputStream->Seek(0, kSeekBegin);
    }
}

// Reconstructed from eboot.elf at 0xA391B0.
void GameMic::UpdateThread() {}
