#include "audio/core/generators/DialogGenerator.h"

Symbol DialogGenerator::sTypeId;
void* DialogGenerator::sDefaultSinkOwner;
DialogSoundSink DialogGenerator::sDefaultSink;

// Reconstructed from eboot.elf at 0x1127330.
bool DialogGenerator::Setup(const char*, const PlayArgs& args) {
    if (args.mFormat != kDialogFormat) {
        return false;
    }
    const auto& dialogArgs = static_cast<const DialogPlayArgs&>(args);
    mSink = dialogArgs.mSink ? dialogArgs.mSink : sDefaultSink;
    mSinkContext = dialogArgs.mContext;
    return true;
}

// Reconstructed from eboot.elf at 0x11273D0.
void DialogGenerator::NotifySoundCreated(const char* name) {
    if (mSink) {
        mSink(mEmitter, Symbol(name), mSinkContext);
    }
}

// Reconstructed from eboot.elf at 0x1127460.
void DialogGenerator::SetDefaultSink(const DialogSoundSink& sink) {
    if (sDefaultSinkOwner != nullptr) {
        sDefaultSink = sink;
    }
}

// Reconstructed from eboot.elf at 0x1127540.
bool DialogGenerator::IsDialog() {
    return true;
}

// Reconstructed from eboot.elf at 0x1127550.
DialogGenerator::~DialogGenerator() {}

// Reconstructed from eboot.elf at 0x11276B0.
void DialogGenerator::_InitTypeId() {
    sTypeId = Symbol("DialogGenerator");
}

// Reconstructed from eboot.elf at 0x1127700.
AudioGenerator* DialogGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x1127720.
Symbol DialogGenerator::GetTypeId() {
    return sTypeId;
}
