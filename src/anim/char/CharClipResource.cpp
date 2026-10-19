// anim/CharClipResource.o (0x2EFF80 to 0x2F135F): the clip configuration's
// functions, then the resource. The class's registration and factory are
// emitted in anim/AnimInit.o. The object's static initializer is at
// 0x2F1300.
#include "anim/char/CharClipResource.h"

#include "anim/char/CharAnim.h"
#include "anim/char/CharClipCom.h"
#include "entity/core/Entity.h"
#include "entity/resources/ResourceMetaData.h"
#include "os/files/File.h"
#include "utl/streams/BinStream.h"
#include "utl/streams/FileStream.h"

namespace {

// The -1 of a shared header that the static initializer sets (0x19F8D90):
// the id of no object, which GetObject and GetEvents compare against. Name
// not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// Reads a bool stored as a byte. Inlined into the readers. Name not in the
// reference map.
bool ReadBool(BinStream& stream) {
    unsigned char value;
    stream.Read(&value, 1);
    return value != 0;
}

// Writes a bool as a byte. Inlined into the writers. Name not in the
// reference map.
void WriteBool(BinStream& stream, bool value) {
    const unsigned char byte = value;
    stream.Write(&byte, 1);
}

// Reconstructed from eboot.elf at 0x2F04F0. Reads the bone masks the
// writer stored with revision 1: the count and each mask. Revision 0 stored
// the count and a word that is skipped, and no masks. The writer is inlined
// into operator<< (0x2F01A0). Name not in the reference map.
BinStream& ReadBoneMasks(BinStream& stream, PropArray<CharBoneChannelMask>& masks) {
    int rev;
    stream.ReadEndian(&rev, 4);
    if (rev != 0) {
        unsigned int count;
        stream.ReadEndian(&count, 4);
        masks.Resize(count);
        for (unsigned int i = 0; i < count; ++i) {
            stream >> masks[i];
        }
    } else {
        unsigned int count = 0;
        stream.ReadEndian(&count, 4);
        int unused = 0;
        stream.ReadEndian(&unused, 4);
        masks.Resize(count);
    }
    return stream;
}

}  // namespace

ResourceMetaData CharClipResource::sMetaData;
std::function<void(CharClipResource*)> CharClipResource::sBuildCallback;

// Reconstructed from eboot.elf at 0x2EFF80.
CharClipConfig::CharClipConfig(unsigned long beatTrackSize)
    : mSourceFbx(),
      mStartFrame(0),
      mEndFrame(0),
      mBeatTrack(),
      mRelativity(0),
      mRelativeSourceFbx(),
      mRelativeFrame(0),
      mBlendMode(AnimResource::kBlendLayered),
      mSubTreeIndex(0),
      mBlendFromRoot(false),
      mBones() {
    mBeatTrack.Resize(beatTrackSize);
}

// Reconstructed from eboot.elf at 0x2F0030.
CharClipConfig::BeatTrackEntry::BeatTrackEntry() : mBeat(0.0f), mFrame(0.0f) {}

// Reconstructed from eboot.elf at 0x2F0080.
bool CharClipConfig::operator==(const CharClipConfig& other) const {
    if (mSourceFbx != other.mSourceFbx || mStartFrame != other.mStartFrame || mEndFrame != other.mEndFrame) {
        return false;
    }
    const unsigned int beats = mBeatTrack.size();
    if (beats != other.mBeatTrack.size()) {
        return false;
    }
    for (unsigned long i = 0; i < beats; ++i) {
        if (mBeatTrack[i].mBeat != other.mBeatTrack[i].mBeat || mBeatTrack[i].mFrame != other.mBeatTrack[i].mFrame) {
            return false;
        }
    }
    if (mRelativity != other.mRelativity || mRelativeSourceFbx != other.mRelativeSourceFbx
        || mRelativeFrame != other.mRelativeFrame || mBlendMode != other.mBlendMode
        || mSubTreeIndex != other.mSubTreeIndex || mBlendFromRoot != other.mBlendFromRoot) {
        return false;
    }
    const unsigned int bones = mBones.size();
    if (bones != other.mBones.size()) {
        return false;
    }
    for (unsigned int i = 0; i < mBones.size(); ++i) {
        if (mBones[i] != other.mBones[i]) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2F01A0. Only the first two beat/frame
// pairs are written; the subtree index is written as a word.
BinStream& operator<<(BinStream& stream, const CharClipConfig& config) {
    int rev = 2;
    stream.WriteEndian(&rev, 4);
    stream << config.mSourceFbx.mPath;
    stream.WriteEndian(&config.mStartFrame, 4);
    stream.WriteEndian(&config.mEndFrame, 4);
    stream.WriteEndian(&config.mBeatTrack[0].mBeat, 4);
    stream.WriteEndian(&config.mBeatTrack[0].mFrame, 4);
    stream.WriteEndian(&config.mBeatTrack[1].mBeat, 4);
    stream.WriteEndian(&config.mBeatTrack[1].mFrame, 4);
    stream.WriteEndian(&config.mRelativity, 4);
    stream << config.mRelativeSourceFbx.mPath;
    stream.WriteEndian(&config.mRelativeFrame, 4);
    const int blendMode = config.mBlendMode;
    stream.WriteEndian(&blendMode, 4);
    WriteBool(stream, config.mBlendFromRoot);
    const int bonesRev = 1;
    stream.WriteEndian(&bonesRev, 4);
    const unsigned int count = config.mBones.size();
    stream.WriteEndian(&count, 4);
    for (unsigned long i = 0; i < count; ++i) {
        stream << config.mBones[i];
    }
    const unsigned int subTreeIndex = static_cast<unsigned int>(config.mSubTreeIndex);
    stream.WriteEndian(&subTreeIndex, 4);
    return stream;
}

// Reconstructed from eboot.elf at 0x2F0380. The beat/frame pairs are read
// into the first two entries the configuration already has.
BinStream& operator>>(BinStream& stream, CharClipConfig& config) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    stream >> config.mSourceFbx.mPath;
    stream.ReadEndian(&config.mStartFrame, 4);
    stream.ReadEndian(&config.mEndFrame, 4);
    if (rev >= 2) {
        stream.ReadEndian(&config.mBeatTrack[0].mBeat, 4);
        stream.ReadEndian(&config.mBeatTrack[0].mFrame, 4);
        stream.ReadEndian(&config.mBeatTrack[1].mBeat, 4);
        stream.ReadEndian(&config.mBeatTrack[1].mFrame, 4);
    }
    int value;
    stream.ReadEndian(&value, 4);
    config.mRelativity = value;
    stream >> config.mRelativeSourceFbx.mPath;
    stream.ReadEndian(&config.mRelativeFrame, 4);
    stream.ReadEndian(&value, 4);
    config.mBlendMode = static_cast<AnimResource::BlendMode>(value);
    config.mBlendFromRoot = ReadBool(stream);
    ReadBoneMasks(stream, config.mBones);
    if (rev > 0) {
        unsigned int subTreeIndex;
        stream.ReadEndian(&subTreeIndex, 4);
        config.mSubTreeIndex = subTreeIndex;
    }
    return stream;
}

// Reconstructed from eboot.elf at 0x2F0600.
void CharClipResource::SetBuildCallback(const std::function<void(CharClipResource*)>& callback) {
    sBuildCallback = callback;
}

// Reconstructed from eboot.elf at 0x2F0670.
CharClipResource::CharClipResource()
    : mConfig(2),
      mAnim(nullptr),
      mUnits(0),
      mLooping(false),
      mSourceStartFrame(0),
      mSourceEndFrame(0),
      mRelativeStartFrame(0),
      mRelativeEndFrame(0),
      mObjectId{0xFFFFFFFFU} {}

// Reconstructed from eboot.elf at 0x2F0720. The deleting destructor is at
// 0x2F0810.
CharClipResource::~CharClipResource() {
    delete mAnim;
    mAnim = nullptr;
}

// Reconstructed from eboot.elf at 0x2F0830.
void CharClipResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("charclip"));
    metaData.mCategory = Symbol("Animations");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x2F0950.
void CharClipResource::Save(BinStream& stream, bool cached) {
    int rev = 1;
    stream.WriteEndian(&rev, 4);
    stream << mConfig;
    CharAnim* const anim = mAnim;
    WriteBool(stream, anim != nullptr);
    if (anim != nullptr) {
        anim->Save(stream, cached);
    }
    stream.WriteEndian(&mSourceStartFrame, 4);
    stream.WriteEndian(&mSourceEndFrame, 4);
    stream.WriteEndian(&mRelativeStartFrame, 4);
    stream.WriteEndian(&mRelativeEndFrame, 4);
}

// Reconstructed from eboot.elf at 0x2F0A70. A file before revision 1 has
// no frame bounds, and its CharAnim is dropped.
bool CharClipResource::Load(BinStream& stream, bool cached) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    stream >> mConfig;
    if (ReadBool(stream)) {
        mAnim = new CharAnim();
        if (!mAnim->Load(stream, cached)) {
            delete mAnim;
            mAnim = nullptr;
        }
    }
    if (rev <= 0) {
        delete mAnim;
        mAnim = nullptr;
    } else {
        stream.ReadEndian(&mSourceStartFrame, 4);
        stream.ReadEndian(&mSourceEndFrame, 4);
        stream.ReadEndian(&mRelativeStartFrame, 4);
        stream.ReadEndian(&mRelativeEndFrame, 4);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2F0BD0. A missing or empty cached file
// leaves the clip without a CharAnim.
void CharClipResource::LoadFile() {
    ResourcePath path;
    path = GetCachedResourcePath(mPath, "", true);
    FileStream stream(path.Str(), kRead, false);
    if (!stream.Fail() && stream.Size() != 0) {
        Load(stream, true);
        mFileTime = FileTimestamp(path.Str());
    }
}

// Reconstructed from eboot.elf at 0x2F0CB0.
bool CharClipResource::Fail() const {
    return mAnim == nullptr;
}

// Reconstructed from eboot.elf at 0x2F0CC0.
CharClipConfig& CharClipResource::GetConfig() {
    return mConfig;
}

// Reconstructed from eboot.elf at 0x2F0CD0.
void CharClipResource::SetUnits(int units) {
    mUnits = units;
}

// Reconstructed from eboot.elf at 0x2F0CE0.
GameObject* CharClipResource::GetObject(Entity* entity) const {
    if (gNullObjectId.mId == mObjectId.mId) {
        return nullptr;
    }
    return entity->GetObject(mObjectId);
}

// Reconstructed from eboot.elf at 0x2F0D60.
bool CharClipResource::IsLooping() const {
    return mLooping;
}

// Reconstructed from eboot.elf at 0x2F0D80.
void CharClipResource::SetLooping(bool looping) {
    mLooping = looping;
}

// Reconstructed from eboot.elf at 0x2F0DA0.
float CharClipResource::GetStartFrame() const {
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2F0DC0.
float CharClipResource::GetEndFrame() const {
    return static_cast<float>(mAnim->mKeyframes.mNumKeyframes) - 1.0f;
}

// Reconstructed from eboot.elf at 0x2F0E00.
float CharClipResource::GetAuthoredStartFrame() const {
    return static_cast<float>(mSourceStartFrame);
}

// Reconstructed from eboot.elf at 0x2F0E20.
float CharClipResource::GetAuthoredEndFrame() const {
    return static_cast<float>(mSourceEndFrame);
}

// Reconstructed from eboot.elf at 0x2F0E40.
TimeUnits CharClipResource::GetUnits() const {
    return static_cast<TimeUnits>(mUnits != 0);
}

// Reconstructed from eboot.elf at 0x2F0E60.
float CharClipResource::GetTimeFromFrame(float frame, TimeUnits units) const {
    if (units == kNumTimeUnits) {
        units = GetUnits();
    }
    if (units == kTimeUnitsBeats) {
        return mAnim->GetBeatFromFrame(frame);
    }
    return frame / mAnim->mFramesPerSecond;
}

// Reconstructed from eboot.elf at 0x2F0F00.
float CharClipResource::GetFrameFromTime(float time, TimeUnits units) const {
    if (units == kNumTimeUnits) {
        units = GetUnits();
    }
    if (units == kTimeUnitsBeats) {
        return mAnim->GetFrameFromBeat(time);
    }
    return time * mAnim->mFramesPerSecond;
}

// Reconstructed from eboot.elf at 0x2F0FA0. The compiler turns the cases
// into a table at 0x127129C.
float CharClipResource::GetTimeAlignment(TimeUnits units) const {
    if (units == kNumTimeUnits) {
        units = GetUnits();
    }
    if (units == kTimeUnitsBeats) {
        switch (mUnits) {
        case 2:
            return 1.0f;
        case 3:
            return 4.0f;
        case 4:
            return 2.0f;
        default:
            break;
        }
    }
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2F1040.
const PropArray<AnimResource::Event>* CharClipResource::GetEvents(Entity* entity) const {
    GameObject* const object = GetObject(entity);
    if (object == nullptr) {
        return nullptr;
    }
    const CharClipCom* const clip = object->GetCom<CharClipCom>();
    return clip != nullptr ? &clip->mEvents : nullptr;
}

// Reconstructed from eboot.elf at 0x2F1170.
float CharClipResource::GetEventFrameOffset() const {
    return static_cast<float>(mConfig.mStartFrame);
}

// Reconstructed from eboot.elf at 0x2F1190.
AnimResource::BlendMode CharClipResource::GetBlendMode() const {
    return mAnim->mBlendMode;
}

// Reconstructed from eboot.elf at 0x2F11B0.
unsigned long CharClipResource::GetSubTreeIndex() const {
    return mAnim->mSubTreeIndex;
}

// Reconstructed from eboot.elf at 0x2F11D0. Empty in this build.
void CharClipResource::Create(const CharClipConfig& config, CharSkeletonResource* skeleton) {
    static_cast<void>(config);
    static_cast<void>(skeleton);
}

// Reconstructed from eboot.elf at 0x2F11E0.
ResourceMetaData* CharClipResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2F11F0.
Symbol CharClipResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2F1290.
bool CharClipResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2F12C0.
Resource* CharClipResource::AsResource() {
    return this;
}

// Reconstructed from eboot.elf at 0x2F12D0.
void* CharClipResource::GetCharData() const {
    return mAnim;
}

// Reconstructed from eboot.elf at 0x2B7370. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* CharClipResource::_Create() {
    return new CharClipResource();
}

// Reconstructed from eboot.elf at 0x2AFCC0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharClipResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("Resource"), true);
}
