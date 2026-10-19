#include "persistence/options/SystemOptions.h"

#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/streams/RevisionStream.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

namespace {

// The limit of the audio and video offsets, in milliseconds. Name not in
// the reference map.
constexpr float kMaxOffset = 300.0f;

// The defaults Reset sets and Load restores for uncalibrated saves. Names
// not in the reference map.
constexpr float kDefaultOverscan = 1.0f;
constexpr float kDefaultAudioOffset = -110.0f;
constexpr float kDefaultVideoOffset = -75.0f;
constexpr float kDefaultDialogVolume = 70.0f;
constexpr float kDefaultGammaValue = 0.5f;

// The last revision whose offsets Load resets. Name not in the reference
// map.
constexpr int kLastUncalibratedRevision = 2;
// The revision that also stored two strings, which Load skips. Name not in
// the reference map.
constexpr int kRevisionWithStrings = 4;

// The offset limited to kMaxOffset either way, as the binary's max and
// select compute it. Name not in the reference map.
inline float ClampOffset(float offset) {
    const float atLeast = offset > -kMaxOffset ? offset : -kMaxOffset;
    return kMaxOffset < offset ? kMaxOffset : atLeast;
}

// Whether a float setter should store the value. Name not in the reference
// map.
inline bool OptionChanged(float current, float value) {
    return __builtin_fabsf(current - value) > 0.0001f;
}

}  // namespace

// Reconstructed from eboot.elf at 0x11B1FE0.
SystemOptions::SystemOptions()
    : mOverscan(kDefaultOverscan),
      mAudioOffset(0.0f),
      mVideoOffset(0.0f),
      mDialogVolume(kDefaultDialogVolume),
      mGammaValue(kDefaultGammaValue),
      mLoaded(false),
      mHasCalibrated(false) {
    Reset();
}

// Reconstructed from eboot.elf at 0x11B2A20.
SystemOptions::~SystemOptions() {}

// Reconstructed from eboot.elf at 0x11B2050.
void SystemOptions::Reset() {
    SetOverscan(kDefaultOverscan, true);
    SetAudioOffset(kDefaultAudioOffset, true);
    SetVideoOffset(kDefaultVideoOffset, true);
    SetDialogVolume(kDefaultDialogVolume, true);
    SetGammaValue(kDefaultGammaValue, true);
    SetHasCalibrated(false);
}

// Reconstructed from eboot.elf at 0x11B2100.
void SystemOptions::SetAudioOffset(float offset, bool force) {
    const float clamped = ClampOffset(offset);
    if (force || OptionChanged(mAudioOffset, clamped)) {
        mAudioOffset = clamped;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0x11B2150.
void SystemOptions::SetVideoOffset(float offset, bool force) {
    const float clamped = ClampOffset(offset);
    if (force || OptionChanged(mVideoOffset, clamped)) {
        mVideoOffset = clamped;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0x11B21A0.
void SystemOptions::SetDialogVolume(float volume, bool force) {
    if (force || OptionChanged(mDialogVolume, volume)) {
        mDialogVolume = volume;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0x11B21D0.
void SystemOptions::SetGammaValue(float gamma, bool force) {
    if (force || OptionChanged(mGammaValue, gamma)) {
        mGammaValue = gamma;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0x11B2200.
void SystemOptions::SetHasCalibrated(bool hasCalibrated) {
    if (hasCalibrated != mHasCalibrated) {
        mHasCalibrated = hasCalibrated;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0x11B2220.
void SystemOptions::Save(BinStream& stream) const {
    RevisionStream revisionStream(stream, kRevision, 0);
    revisionStream.BeginWrite();
    float value = mOverscan;
    revisionStream.WriteEndian(&value, sizeof(value));
    value = mAudioOffset;
    revisionStream.WriteEndian(&value, sizeof(value));
    value = mVideoOffset;
    revisionStream.WriteEndian(&value, sizeof(value));
    value = mDialogVolume;
    revisionStream.WriteEndian(&value, sizeof(value));
    value = mGammaValue;
    revisionStream.WriteEndian(&value, sizeof(value));
    const bool hasCalibrated = mHasCalibrated;
    revisionStream.Write(&hasCalibrated, sizeof(hasCalibrated));
}

// Reconstructed from eboot.elf at 0x11B2310.
void SystemOptions::Load(BinStream& stream) {
    RevisionStream revisionStream(stream, kRevision, 0);
    if (revisionStream.BeginRead()) {
        float value = mOverscan;
        revisionStream >> Revision::AsOfOp<float>(&value, 1);
        SetOverscan(value, false);
        value = mAudioOffset;
        revisionStream >> Revision::AsOfOp<float>(&value, 1);
        SetAudioOffset(value, false);
        value = mVideoOffset;
        revisionStream >> Revision::AsOfOp<float>(&value, 1);
        SetVideoOffset(value, false);
        value = mDialogVolume;
        revisionStream >> Revision::AsOfOp<float>(&value, 1);
        SetDialogVolume(value, false);
        value = mGammaValue;
        revisionStream >> Revision::AsOfOp<float>(&value, 1);
        SetGammaValue(value, false);
        bool hasCalibrated;
        revisionStream >> Revision::AsOfOp<bool>(&hasCalibrated, 2);
        SetHasCalibrated(hasCalibrated);
        if (!hasCalibrated) {
            SetAudioOffset(kDefaultAudioOffset, false);
            SetVideoOffset(kDefaultVideoOffset, false);
        }
        if (revisionStream.mReadRevision <= kLastUncalibratedRevision) {
            SetAudioOffset(kDefaultAudioOffset, false);
            SetVideoOffset(kDefaultVideoOffset, false);
            SetHasCalibrated(false);
        }
        if (revisionStream.mReadRevision == kRevisionWithStrings) {
            String unused;
            String unused2;
            revisionStream >> unused;
            revisionStream >> unused2;
        }
    }
    mLoaded = true;
}

// Reconstructed from eboot.elf at 0x11B2770.
void SystemOptions::MakeDirty() {
    static Symbol systemOptionsDirty;
    if (systemOptionsDirty == Symbol()) {
        systemOptionsDirty = Symbol("system_options_dirty");
    }
    Message<0> msg(systemOptionsDirty);
    Export(msg.mData);
    mDirty = true;
}

// Reconstructed from eboot.elf at 0x11B2850.
DataNode SystemOptions::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol onSystemOptionsLoaded;
    if (onSystemOptionsLoaded == Symbol()) {
        onSystemOptionsLoaded = Symbol("on_system_options_loaded");
    }
    if (type == onSystemOptionsLoaded) {
        _OnSystemOptionsLoaded();
        return DataNode(0);
    }
    return DataNode();
}
