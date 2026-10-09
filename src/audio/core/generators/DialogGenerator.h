#pragma once

#include <cstddef>
#include <functional>

#include "audio/core/generators/AudioGenerator.h"

// Receives the emitter, the sound name and the request's context. Name not in
// the reference map.
using DialogSoundSink = std::function<void(AudioEmitterCom*, Symbol, void*)>;

// Request for a dialog generator, marked by PlayArgs::mFormat 4. Names not in
// the reference map.
struct DialogPlayArgs : public PlayArgs {
    // Optional sink for the names of the sounds the event creates.
    DialogSoundSink mSink;
    void* mContext;
};

static_assert(offsetof(DialogPlayArgs, mSink) == 112);
static_assert(offsetof(DialogPlayArgs, mContext) == 160);

// Abstract generator for spoken dialog. _InitTypeId at 0x11276B0 registers
// the type name "DialogGenerator"; the map has no object for it. It forwards
// the name of every sound a dialog event creates to the request's sink. The
// vtable is at 0x199FFC8.
class DialogGenerator : public AudioGenerator {
public:
    // The PlayArgs::mFormat of a DialogPlayArgs. Name not in the reference
    // map.
    static constexpr int kDialogFormat = 4;

    bool IsDialog() override;            // slot 18: 0x1127540
    ~DialogGenerator() override;         // slots 20-21: 0x1127550, 0x1127600
    void _InitTypeId() override;         // slot 28: 0x11276B0
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x1127700
    Symbol GetTypeId() override;         // slot 31: 0x1127720
    // Slot 32 at 0x1127330: keeps the request's sink, or the default sink when
    // it has none, and its context. Only dialog requests are accepted.
    virtual bool Setup(const char* path, const PlayArgs& args);

    // Passes a created sound's name to the sink. At 0x11273D0. Name not in
    // the reference map.
    void NotifySoundCreated(const char* name);
    // Replaces the default sink. At 0x1127460, with no callers in this
    // build; it only stores the sink while sDefaultSinkOwner is set. Name not
    // in the reference map.
    static void SetDefaultSink(const DialogSoundSink& sink);

    // Names not in the reference map.
    static Symbol sTypeId;                // 0x1B5C1E8
    static void* sDefaultSinkOwner;       // 0x1B5C1F0, never written in this build
    static DialogSoundSink sDefaultSink;  // 0x1B5C200

    // Field names are not in the reference map.
    DialogSoundSink mSink;
    void* mSinkContext;
};

static_assert(offsetof(DialogGenerator, mSink) == 80);
static_assert(offsetof(DialogGenerator, mSinkContext) == 128);
