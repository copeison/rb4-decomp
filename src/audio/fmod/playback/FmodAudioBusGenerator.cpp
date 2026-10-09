#include "audio/fmod/playback/FmodAudioBusGenerator.h"

#include <cstring>
#include <unistd.h>

#include "audio/fmod/io/FmodRecordingAudioRenderTarget.h"
#include "audio/core/generators/GeneratorPool.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "math/transform/Transform.h"

namespace {

constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE kEventCallbacks =
    FMOD_STUDIO_EVENT_CALLBACK_DESTROYED |
    FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND |
    FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND;

FMOD::Studio::EventInstance* AsEventInstance(FMOD_STUDIO_EVENTINSTANCE* event) {
    return reinterpret_cast<FMOD::Studio::EventInstance*>(event);
}

}  // namespace

FMOD_DSP_DESCRIPTION FmodAudioBusGenerator::sDspDescription = {
    110,
    "HMXRawAudioBus",
    0x00010000,
    0,
    1,
    FmodAudioBusGenerator::_DspCreate,
    nullptr,
    nullptr,
    nullptr,
    FmodAudioBusGenerator::_DspProcess,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

Symbol FmodAudioBusGenerator::sTypeId("");

// Reconstructed from eboot.elf at 0x266F10.
FmodAudioBusGenerator::FmodAudioBusGenerator(AudioBus* source)
    : AudioBusGenerator(source),
      mDSP(nullptr),
      mChannel(nullptr),
      mChannelGroup(nullptr),
      mStudioBus(nullptr),
      mEventInstance(nullptr),
      mEventCallbackDone(false),
      mResetWord(0),
      mKilling(false) {}

// Reconstructed from eboot.elf at 0x266F60, which tail-calls the base
// destructor.
FmodAudioBusGenerator::~FmodAudioBusGenerator() {}

// Reconstructed from eboot.elf at 0x266CB0.
FMOD_RESULT FmodAudioBusGenerator::_DspCreate(FMOD_DSP_STATE* state) {
    static_cast<FMOD::DSP*>(state->instance)
        ->setChannelFormat(FMOD_CHANNELMASK_STEREO, 2, FMOD_SPEAKERMODE_STEREO);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x266CD0. Interleaves the last rendered
// stereo block into FMOD's output, or writes silence when none is ready.
FMOD_RESULT FmodAudioBusGenerator::_DspProcess(
    FMOD_DSP_STATE* state,
    unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY*,
    FMOD_DSP_BUFFER_ARRAY* outputs,
    bool,
    FMOD_DSP_PROCESS_OPERATION operation) {
    if (operation == FMOD_DSP_PROCESS_QUERY) {
        if (outputs != nullptr) {
            outputs->speakermode = FMOD_SPEAKERMODE_STEREO;
            outputs->buffernumchannels[0] = 2;
            outputs->bufferchannelmask[0] = FMOD_CHANNELMASK_STEREO;
        }
        return FMOD_OK;
    }

    float* output = outputs->buffers[0];
    FmodAudioBusGenerator* generator = nullptr;
    if (static_cast<FMOD::DSP*>(state->instance)
                ->getUserData(reinterpret_cast<void**>(&generator)) != FMOD_OK ||
        generator == nullptr) {
        std::memset(output, 0, length * 2 * sizeof(float));
        return FMOD_OK;
    }

    AudioMixer* mixer = generator->mMixer;
    mixer->Lock();
    if (!generator->mBuffer.mHasSamples) {
        std::memset(output, 0, length * 2 * sizeof(float));
    } else {
        const float* left = generator->mBuffer.mChannelData[0];
        const float* right = generator->mBuffer.mChannelData[1];
        for (unsigned int index = 0; index < length; ++index) {
            output[index * 2] = left[index];
            output[index * 2 + 1] = right[index];
        }
        generator->mBuffer.mHasSamples = false;
    }
    mixer->Unlock();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x266FD0. A non-oneshot Studio event hosts
// the DSP when one is requested; otherwise the DSP plays on a paused
// low-level channel, optionally routed to a Studio bus.
bool FmodAudioBusGenerator::Setup(
    AudioBus* source, const PlayArgs& args, AudioBusCallable* callback) {
    AudioBusGenerator::Setup(source, args, callback);
    mKilling = false;
    mEventCallbackDone = false;
    mFrequency = 0.0F;
    mDSP = nullptr;
    mChannel = nullptr;
    mChannelGroup = nullptr;
    mStudioBus = nullptr;

    FModSystem* system = FModSystemForTarget(mRenderTarget);
    FMOD::Studio::System* studio = system != nullptr ? system->mStudioSystem : nullptr;
    FMOD::System* lowLevel = studio != nullptr ? system->mLowLevelSystem : nullptr;
    lowLevel->createDSP(&sDspDescription, &mDSP);
    mDSP->setUserData(this);

    FMOD::Studio::EventDescription* description = nullptr;
    if (args.mRoute == PlayArgs::kRouteEvent) {
        bool oneshot = false;
        if (studio->getEvent(args.mRoutePath.Str(), &description) != FMOD_OK ||
            (description->isOneshot(&oneshot), oneshot)) {
            description = nullptr;
        }
        if (description != nullptr) {
            description->createInstance(&mEventInstance);
            mEventInstance->setUserData(this);
            if (args.mParameters != nullptr) {
                for (auto* parameter = args.mParameters->begin();
                     parameter != args.mParameters->end();
                     ++parameter) {
                    SetParameter(parameter->mName, parameter->mValue);
                }
            }
            mEventInstance->setCallback(_EventProgrammerCallback, kEventCallbacks);
            mEventInstance->setPaused(args.mStartPaused);
            mEventInstance->start();
            return true;
        }
    }

    lowLevel->playDSP(mDSP, nullptr, true, &mChannel);
    if (args.mRoute == PlayArgs::kRouteBus) {
        if (studio->getBus(args.mRoutePath.Str(), &mStudioBus) != FMOD_OK) {
            mStudioBus = nullptr;
        } else {
            mStudioBus->getChannelGroup(&mChannelGroup);
            mChannel->setChannelGroup(mChannelGroup);
        }
    }
    if (mEmitter != nullptr) {
        if (mEmitter->Is3D()) {
            mChannel->setMode(FMOD_3D);
            mChannel->set3DSpread(args.mSpread);
        }
        if (mEmitter != nullptr && mEmitter->GetMixGroup() != nullptr) {
            // The mix group's channel group is inlined as null in this build.
            FMOD::ChannelGroup* parentGroup = nullptr;
            if (mChannelGroup != nullptr) {
                parentGroup->addGroup(mChannelGroup, true, nullptr);
            } else {
                mChannel->setChannelGroup(parentGroup);
            }
        }
    }
    mChannel->getFrequency(&mFrequency);
    mChannel->setPaused(args.mStartPaused);
    mMixer->AddCallable(this);
    return true;
}

// Reconstructed from eboot.elf at 0x267310. Destroy callbacks mark the event
// safe to release. Creating the programmer sound binds the HMX plugins to
// this generator and inserts the DSP at the tail of the event's channel
// group.
FMOD_RESULT FmodAudioBusGenerator::_EventProgrammerCallback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD_STUDIO_EVENTINSTANCE* event,
    void*) {
    FMOD::Studio::EventInstance* instance = AsEventInstance(event);
    FmodAudioBusGenerator* generator = nullptr;
    if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED ||
        type == FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND) {
        instance->getUserData(reinterpret_cast<void**>(&generator));
        if (generator != nullptr) {
            generator->mEventCallbackDone = true;
        }
        return FMOD_OK;
    }
    if (type != FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND) {
        return FMOD_OK;
    }

    instance->getUserData(reinterpret_cast<void**>(&generator));
    if (generator->mEventCallbackDone) {
        return FMOD_OK;
    }
    if (generator->mState != kStateStopping) {
        FMOD::ChannelGroup* channelGroup = nullptr;
        instance->getChannelGroup(&channelGroup);
        BindPluginsToGenerator(channelGroup, generator);

        generator->mFrequency = static_cast<float>(generator->mRenderTarget->mSampleRate);
        channelGroup->addDSP(FMOD_CHANNELCONTROL_DSP_TAIL, generator->mDSP);
        AudioEmitterCom* emitter = generator->mEmitter;
        if (emitter != nullptr && emitter->GetMixGroup() != nullptr) {
            // The mix group's channel group is inlined as null in this build.
            FMOD::ChannelGroup* parentGroup = nullptr;
            parentGroup->addGroup(channelGroup, true, nullptr);
        }
        generator->mMixer->AddCallable(generator);
    }
    generator->mEventCallbackDone = true;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2675A0 (0x267780 adjusts from the
// AudioBusCallable base). Advances the gain and mute ramps once per block
// until the block is rendered; a finished fade marked to stop stops the
// voice. The callback is prepared at the channel frequency and dropped once
// it declines.
bool FmodAudioBusGenerator::_PrepareToMakeSamples(
    int numSamples, float, int mixCount, int block, bool lastBlock) {
    if (mState != kStatePlaying) {
        return false;
    }
    if (mBuffer.mHasSamples) {
        return true;
    }
    mGainRamp.Advance();
    mMuteRamp.Advance();
    if (mGainRamp.mProgress == 1.0F && mGainFadeMode == kPostFadeStop) {
        mState = kStateStopping;
        return false;
    }
    if (mCallback == nullptr) {
        return false;
    }
    if (!mCallback->_PrepareToMakeSamples(numSamples, mFrequency, mixCount, block, lastBlock)) {
        mCallback = nullptr;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x267790 (0x267BA0 adjusts from the
// AudioBusCallable base). Renders the source into this block's part of the
// block buffer and applies the gain and mute; a voice that is not playing
// renders silence. The last block marks the buffer ready for _DspProcess.
bool FmodAudioBusGenerator::_MakeSamples(int numSamples, float, int, int block, bool lastBlock) {
    AudioBuffer<float> view;
    view.Configure(
        AudioBufferConfig(mBuffer.mConfig.mNumChannels, numSamples, 0.0F, false), AudioBufferBase::kCleanupNone);
    view.SetChannelData(mBuffer, block * numSamples);
    bool result;
    if (mState == kStatePlaying) {
        result = true;
        if (!mBuffer.mHasSamples) {
            mSource->Process(view);
            float gain = mMuteRamp.mValue * mGainRamp.mValue;
            if (gain != 1.0F) {
                for (int channel = 0; channel < view.mNumChannels; ++channel) {
                    for (int frame = 0; frame < view.mNumFrames; ++frame) {
                        view.mChannelData[channel][frame] *= gain;
                    }
                }
            }
            if (lastBlock) {
                mBuffer.mHasSamples = true;
            }
        }
    } else {
        view.Clear();
        result = mState != kStateStopped && mState != kStateStopping;
    }
    return result;
}

// Reconstructed from eboot.elf at 0x267BB0.
void FmodAudioBusGenerator::Pause() {
    if (mState == kStatePaused || mState == kStateStopped || mState == kStateStopping) {
        return;
    }
    if (mChannel != nullptr) {
        mChannel->setPaused(true);
    } else if (mEventInstance != nullptr) {
        mEventInstance->setPaused(true);
    }
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x267C00.
void FmodAudioBusGenerator::Continue() {
    if (mState == kStateStopped || mState == kStateStopping) {
        return;
    }
    if (mChannel != nullptr) {
        mChannel->setPaused(false);
        mState = kStatePlaying;
    } else if (mEventInstance != nullptr) {
        mEventInstance->setPaused(false);
        mState = kStatePlaying;
    } else {
        mState = kStateReady;
    }
}

// Reconstructed from eboot.elf at 0x267C60.
float FmodAudioBusGenerator::GetElapsedMs() {
    unsigned int position = 0;
    if (mChannel == nullptr) {
        return 0.0F;
    }
    mChannel->getPosition(&position, FMOD_TIMEUNIT_MS);
    return static_cast<float>(position);
}

// Reconstructed from eboot.elf at 0x267CC0.
float FmodAudioBusGenerator::GetTimelineMs() {
    if (mChannel != nullptr) {
        return GetElapsedMs();
    }
    if (mEventInstance == nullptr) {
        return 0.0F;
    }
    int position;
    mEventInstance->getTimelinePosition(&position);
    return static_cast<float>(position);
}

// Reconstructed from eboot.elf at 0x267D30.
void FmodAudioBusGenerator::SeekToMs(float ms) {
    if (mChannel != nullptr) {
        mChannel->setPosition(static_cast<unsigned int>(ms), FMOD_TIMEUNIT_MS);
    } else if (mEventInstance != nullptr) {
        mEventInstance->setTimelinePosition(static_cast<int>(ms));
    }
}

// Reconstructed from eboot.elf at 0x267D70. Finishes a pending stop, or keeps
// the voice's 3D position current.
bool FmodAudioBusGenerator::Poll() {
    if (mState == kStateStopped) {
        return false;
    }
    if (mKilling) {
        return true;
    }
    if (mState == kStateStopping) {
        _StopChannel();
        _StopEventInstance();
        return mState != kStateStopped;
    }
    _UpdateWorldXfm();
    return true;
}

// Reconstructed from eboot.elf at 0x267E50.
void FmodAudioBusGenerator::_StopChannelAndEventInstance() {
    _StopChannel();
    _StopEventInstance();
}

// Reconstructed from eboot.elf at 0x267E70.
void FmodAudioBusGenerator::_UpdateWorldXfm() {
    if ((mChannel == nullptr && mEventInstance == nullptr) || mEmitter == nullptr) {
        return;
    }
    FMOD_3D_ATTRIBUTES attributes;
    Convert(mEmitter->GetWorldXfm(), attributes);
    if (mChannel != nullptr) {
        mChannel->set3DAttributes(&attributes.position, &attributes.velocity, nullptr);
    } else {
        mEventInstance->set3DAttributes(&attributes);
    }
}

// Reconstructed from eboot.elf at 0x267F10. While a fade to silence runs,
// stopping only refreshes the fade rate from the source; otherwise the voice
// is released on its next poll.
void FmodAudioBusGenerator::Stop() {
    if (mState == kStateStopped) {
        return;
    }
    if (mGainFadeMode != 1) {
        mState = kStateStopping;
        return;
    }
    mGainRamp.SetDurationMs(kMinGainRampMs, BlocksPerSecond());
}

// Reconstructed from eboot.elf at 0x267FB0. The channel and DSP are released
// by FModSystem on its next premix.
void FmodAudioBusGenerator::_StopChannel() {
    if (mChannel == nullptr) {
        return;
    }
    mState = kStateStopping;
    _DeativateBusAndClearDSPUserData();
    static_cast<FModSystem*>(mRenderTarget)->DeferRelease(mChannel, mDSP);
    mChannel = nullptr;
    mChannelGroup = nullptr;
    mStudioBus = nullptr;
    mDSP = nullptr;
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x268080. The event is released only after
// its destroy callback has fired. An invalid handle means FMOD has already
// freed the event's objects.
void FmodAudioBusGenerator::_StopEventInstance() {
    if (mEventInstance == nullptr) {
        return;
    }
    mState = kStateStopping;
    if (!mEventCallbackDone) {
        return;
    }
    _DeativateBusAndClearDSPUserData();
    FMOD::ChannelGroup* channelGroup = nullptr;
    const FMOD_RESULT result = mEventInstance->getChannelGroup(&channelGroup);
    if (result == FMOD_OK) {
        channelGroup->removeDSP(mDSP);
    } else if (result == FMOD_ERR_INVALID_HANDLE) {
        mChannelGroup = nullptr;
        mStudioBus = nullptr;
        mEventInstance = nullptr;
        mDSP = nullptr;
        mState = kStateStopped;
        return;
    }
    mEventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    mEventInstance->release();
    mChannelGroup = nullptr;
    mStudioBus = nullptr;
    mEventInstance = nullptr;
    if (mDSP != nullptr) {
        mDSP->release();
    }
    mDSP = nullptr;
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x2681E0. The mixer lock keeps _DspProcess
// from reading the generator while its DSP is detached.
void FmodAudioBusGenerator::_DeativateBusAndClearDSPUserData() {
    if (mMixer != nullptr) {
        mMixer->RemoveCallable(this);
        if (mMixer != nullptr) {
            mMixer->Lock();
            mDSP->setUserData(nullptr);
            mMixer->Unlock();
            return;
        }
    }
    mDSP->setUserData(nullptr);
}

// Reconstructed from eboot.elf at 0x268260.
void FmodAudioBusGenerator::_ReleaseDSP() {
    mChannelGroup = nullptr;
    mStudioBus = nullptr;
    if (mDSP != nullptr) {
        mDSP->release();
    }
    mDSP = nullptr;
}

// Reconstructed from eboot.elf at 0x2682A0. Pumps Studio updates until the
// event allows teardown. Without a live FMOD system the handles are dropped.
void FmodAudioBusGenerator::Kill() {
    mKilling = true;
    _StopChannel();
    _StopEventInstance();
    FModSystem* system = FModSystemForTarget(mRenderTarget);
    if (system != nullptr && system->mStudioSystem != nullptr &&
        system->mLowLevelSystem != nullptr) {
        FMOD::Studio::System* studio = system->mStudioSystem;
        while (mState != kStateStopped) {
            usleep(1000);
            studio->update();
            _StopChannel();
            _StopEventInstance();
        }
    } else {
        mState = kStateStopping;
        if (mMixer != nullptr) {
            mMixer->RemoveCallable(this);
        }
        mDSP = nullptr;
        mChannel = nullptr;
        mChannelGroup = nullptr;
        mStudioBus = nullptr;
        mEventInstance = nullptr;
        mState = kStateStopped;
    }
    mKilling = false;
}

// Reconstructed from eboot.elf at 0x268380.
void FmodAudioBusGenerator::Release() {
    GeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x2683F0.
bool FmodAudioBusGenerator::SetParameter(Symbol name, float value) {
    return mEventInstance != nullptr &&
        mEventInstance->setParameterValue(name.Str(), value) == FMOD_OK;
}

// Reconstructed from eboot.elf at 0x268410.
bool FmodAudioBusGenerator::GetParameter(Symbol name, float& value) {
    if (mEventInstance == nullptr) {
        return false;
    }
    FMOD::Studio::ParameterInstance* parameter = nullptr;
    return mEventInstance->getParameter(name.Str(), &parameter) == FMOD_OK &&
        parameter->getValue(&value) == FMOD_OK;
}

// Reconstructed from eboot.elf at 0x268490.
void* FmodAudioBusGenerator::GetPluginData(const char* name) {
    if (mEventInstance == nullptr) {
        return nullptr;
    }
    FMOD::ChannelGroup* channelGroup = nullptr;
    if (mEventInstance->getChannelGroup(&channelGroup) != FMOD_OK || channelGroup == nullptr) {
        return nullptr;
    }
    return FindPluginData(channelGroup, name);
}

// Reconstructed from eboot.elf at 0x268BF0.
void FmodAudioBusGenerator::_InitTypeId() {
    sTypeId = Symbol("FmodAudioBusGenerator");
}

// Reconstructed from eboot.elf at 0x268C40.
AudioGenerator* FmodAudioBusGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x268C60.
Symbol FmodAudioBusGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x268500. Bus generators are only created
// through _GetGenerator.
AudioGenerator* FmodAudioBusGeneratorManager::Play(const PlayArgs&) {
    return nullptr;
}

// Reconstructed from eboot.elf at 0x268480.
void FmodAudioBusGeneratorManager::Init() {
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x268510.
int FmodAudioBusGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x268520.
Symbol FmodAudioBusGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x2685C0.
Symbol FmodAudioBusGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".fmod_bus");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0x268660.
AudioGenerator* FmodAudioBusGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x2686D0.
void FmodAudioBusGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x268740.
void FmodAudioBusGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x2687B0.
void FmodAudioBusGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x268900.
void FmodAudioBusGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x268910.
void FmodAudioBusGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x268A60.
bool FmodAudioBusGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x268B10.
FmodAudioBusGeneratorManager::~FmodAudioBusGeneratorManager() {}

// Reconstructed from eboot.elf at 0x268B40.
FmodAudioBusGenerator* FmodAudioBusGeneratorManager::_GetGenerator(
    AudioRenderTarget* target, AudioEmitterCom* emitter) {
    return GeneratorPool::Allocate<FmodAudioBusGenerator>(*this, target, emitter);
}
