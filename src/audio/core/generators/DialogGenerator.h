#pragma once

#include <cstddef>
#include <functional>

#include "audio/core/generators/AudioGenerator.h"

// Abstract generator for spoken dialog. _InitTypeId at 0x11276B0 registers
// the type name "DialogGenerator"; the map has no object for it. It forwards
// the name of every sound a dialog event creates to the request's sink. The
// vtable is at 0x199FFC8. Its methods have not been reconstructed.
class DialogGenerator : public AudioGenerator {
public:
    // Receives the emitter, the sound name and the request's context.
    using SoundSink = std::function<void(AudioEmitterCom*, Symbol, void*)>;

    bool IsDialog() override;            // slot 18: 0x1127540
    ~DialogGenerator() override;         // slots 20-21: 0x1127550, 0x1127600
    void _InitTypeId() override;         // slot 28: 0x11276B0
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x1127700
    Symbol GetTypeId() override;         // slot 31: 0x1127720
    // Slot 32 at 0x1127330: keeps the request's sink and context.
    virtual bool Setup(const char* path, const PlayArgs& args);

    // Passes a created sound's name to the sink. At 0x11273D0. Name not in
    // the reference map.
    void NotifySoundCreated(const char* name);

    // Field names are not in the reference map.
    SoundSink mSink;
    void* mSinkContext;
};

static_assert(offsetof(DialogGenerator, mSink) == 80);
static_assert(offsetof(DialogGenerator, mSinkContext) == 128);
