// render/RndShaderNodeTime.o: only the scene globals are reconstructed.
#include "render/shaders/nodes/RndShaderNodeTime.h"

#include "audio/core/components/AudioEmitterCom.h"
#include "audio/core/music/SongPos.h"
#include "audio/core/system/SoundManager.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "utl/time/TimeMgr.h"

namespace {

// Added to the music position the shaders receive. Name not in the
// reference map; why the offset is there is not known.
constexpr float kMusicTickOffset = 0.11F;

}  // namespace

// Reconstructed from eboot.elf at 0x5A52D0. gTime holds the clock's
// seconds, UI seconds and beat, then the default 2D emitter's music
// content tick, or zero without the emitter.
void RndShaderNodeTime::SetGlobalConstants(RndShaderCBuffer& cbuffer) {
    const TimeMgr::Clock& clock = TheTimeMgr->mClock;
    RndDevice* const device = TheRndDevice();
    const float seconds = clock.Seconds();
    const float uiSeconds = clock.UISeconds();
    const float beat = clock.Beat();
    float musicTick = 0.0F;
    Component* const emitter = theSoundManager.GetDefault2DEmitterComponent();
    if (emitter != nullptr) {
        const SongPos pos = static_cast<AudioEmitterCom*>(emitter)
                                ->GetCurrentMusicContentPos();
        musicTick = pos.mTotalTick + kMusicTickOffset;
    }
    auto* const time = static_cast<float*>(cbuffer.mData)
        + device->mShaderMgr.mTime * 4;
    time[0] = seconds;
    time[1] = uiSeconds;
    time[2] = beat;
    time[3] = musicTick;
    cbuffer.mSyncPending = true;
}
