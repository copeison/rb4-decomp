// anim/CharAnimResource.o (0x2DFE90 to 0x2E14DF). The binary emits the
// typed resource loads it uses here: LoadUnique<CharAnimRawResource>
// (0x2E09A0) and GetOrLoad<T> for CharAnimResource, CharAnimRawResource and
// CharBoneMaskResource (0x2E0AF0, 0x2E0C40, 0x2E1330).
#include "anim/char/CharAnimResource.h"

#include "anim/char/CharAnim.h"
#include "anim/char/CharAnimCom.h"
#include "anim/char/CharAnimRaw.h"
#include "anim/char/CharAnimRawResource.h"
#include "anim/char/CharBoneMaskCom.h"
#include "anim/char/CharBoneMaskResource.h"
#include "anim/skeleton/CharSkeleton.h"
#include "anim/skeleton/CharSkeletonResource.h"
#include "entity/core/Entity.h"
#include "entity/progress/LoadProgress.h"
#include "entity/resources/ResourceMetaData.h"
#include "os/system/System.h"
#include "utl/data/DataArray.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"

// The object's statics, in the order of its static initializer (0x2E1480).
ResourceMetaData CharAnimResource::sMetaData;
CharAnimResource::CacheVisitor CharAnimResource::sCacheVisitor;

namespace {

// "disable_char_anim_caching" from the "anim" config, read by _Init.
bool sIsCachingDisabled;  // 0x19F8770

// The beats per keyframe-aligned unit of the beat-aligned units
// (beatAlign1, beatAlign4, beatAlign2), by Units minus 2 (0x126FD7C).
const float kTimeAlignments[3] = {1.0f, 4.0f, 2.0f};

}  // namespace

// Reconstructed from eboot.elf at 0x2DFE90.
CharAnimResource::CharAnimResource() : mAnim(nullptr) {}

// Reconstructed from eboot.elf at 0x2DFED0.
void CharAnimResource::InitForExport() {
    mAnim = new CharAnim();
}

// Reconstructed from eboot.elf at 0x2DFF00. The deleting destructor is at
// 0x2DFF60.
CharAnimResource::~CharAnimResource() {
    delete mAnim;
    mAnim = nullptr;
}

// Reconstructed from eboot.elf at 0x2DFE20.
void CharAnimResource::RegisterCacheVisitor(CacheVisitor visitor) {
    sCacheVisitor = static_cast<CacheVisitor&&>(visitor);
}

// Reconstructed from eboot.elf at 0x2DFFC0.
void CharAnimResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("charanim"));
    metaData.mCategory = Symbol("Animations");
    metaData.mTypeOption = 1;
    DataArray* const config = SystemConfig(Symbol("anim"));
    bool disabled = false;
    config->FindData(Symbol("disable_char_anim_caching"), disabled, false);
    sIsCachingDisabled = disabled;
}

// Reconstructed from eboot.elf at 0x2E0130.
bool CharAnimResource::IsCachingDisabled() {
    return sIsCachingDisabled;
}

// Reconstructed from eboot.elf at 0x2E0140.
bool CharAnimResource::_LoadEntity(BinStream& stream, bool cached) {
    delete mAnim;
    mAnim = nullptr;
    return EntityResource::_LoadEntity(stream, cached);
}

// Reconstructed from eboot.elf at 0x2E01A0. Building reports four steps of
// the "CharAnimResource" load-progress task. A bind-relative animation is
// relative to a one-frame raw animation of the mask skeleton's bind pose,
// whose bones keep the raw animation's inverse parent scale flags.
bool CharAnimResource::_PostLoad(BinStream& stream, bool cached) {
    mCom = mEntity->GetRoot()->GetCom<CharAnimCom>();
    if (cached) {
        unsigned int rev = 0;
        stream.ReadEndian(&rev, 4);
        if (rev != CharAnim::kRev) {
            return false;
        }
        mAnim = CharAnim::New();
        if (mAnim->Load(stream, true)) {
            return true;
        }
        delete mAnim;
        mAnim = nullptr;
        return false;
    }
    if (sIsCachingDisabled) {
        return false;
    }
    ResourcePath boneMaskPath;
    boneMaskPath = mCom->mBoneMaskFile.Str();
    if (boneMaskPath.mPath == Symbol()) {
        return mAnim != nullptr;
    }
    ResourcePath sourcePath;
    sourcePath = mCom->mAnimSourceFile.Str();
    FormatString description("Generating %s...");
    description << sourcePath.mPath;
    LoadProgressBegin("CharAnimResource", description.Str(), 4, 0);

    bool failed = true;
    ResourcePtr<CharBoneMaskResource> boneMask =
        mEntity->mResource->TryGetInline<CharBoneMaskResource>(boneMaskPath);
    if (boneMask == nullptr) {
        boneMask = Resource::GetOrLoad<CharBoneMaskResource>(boneMaskPath, false);
    }
    if (boneMask != nullptr && !boneMask->Fail()) {
        LoadProgressStep("CharAnimResource", 1);
        ResourcePtr<CharAnimRawResource> source = Resource::LoadUnique<CharAnimRawResource>(sourcePath, false);
        LoadProgressStep("CharAnimResource", 1);
        if (source != nullptr && !source->Fail()) {
            CharBoneMaskCom* const mask = boneMask->mEntity->GetRoot()->GetCom<CharBoneMaskCom>();
            CharAnimRaw* const raw = source->mRaw;
            ResourcePath relativePath;
            const int relative = mCom->GetRelative(relativePath);
            CharAnimRaw* relativeRaw = nullptr;
            ResourcePtr<CharAnimResource> other;
            ResourcePtr<CharAnimRawResource> otherSource;
            bool build = true;
            if (relativePath.mPath != Symbol()) {
                if (relativePath == mPath) {
                    relativeRaw = source->mRaw;
                } else {
                    other = Resource::GetOrLoad<CharAnimResource>(relativePath, false);
                    if (other->Fail()) {
                        build = false;
                    } else {
                        const CharAnimCom* const otherCom = other->mEntity->GetRoot()->GetCom<CharAnimCom>();
                        ResourcePath otherSourcePath;
                        otherSourcePath = otherCom->mAnimSourceFile.Str();
                        otherSource = Resource::GetOrLoad<CharAnimRawResource>(otherSourcePath, false);
                        relativeRaw = otherSource->mRaw;
                    }
                }
            } else if (relative == CharAnimCom::kRelativeBind) {
                relativeRaw = CharAnimRaw::New();
                const CharSkeleton* const skeleton = mask->GetSkeletonResource()->mSkeleton;
                {
                    const TKeyTrack<float> beats;
                    relativeRaw->Init(skeleton->mNames.size(), 1, 30.0f, beats);
                }
                for (unsigned long bone = 0; bone < skeleton->mNames.size(); ++bone) {
                    const unsigned long index = raw->FindBoneIndex(skeleton->mNames[bone]);
                    const bool inverseParentScale =
                        index != static_cast<unsigned long>(-1) ? raw->mBones[index].mInverseParentScale : false;
                    relativeRaw->SetBone(
                        bone, skeleton->mNames[bone].Str(), skeleton->mParentIndices[bone], inverseParentScale, true);
                    relativeRaw->SetBoneSample(bone, 0, skeleton->mLocalTransforms[bone]);
                }
            }
            if (build) {
                LoadProgressStep("CharAnimResource", 1);
                mAnim = CharAnim::New();
                const bool created = mAnim->Create(mask, raw, relativeRaw, source->mPath);
                LoadProgressStep("CharAnimResource", 1);
                if (relative == CharAnimCom::kRelativeBind && relativeRaw != nullptr) {
                    delete relativeRaw;
                }
                if (created) {
                    if (sCacheVisitor) {
                        sCacheVisitor(this, mAnim, raw);
                    }
                    failed = false;
                } else {
                    delete mAnim;
                    mAnim = nullptr;
                }
            }
        }
    }
    LoadProgressEnd("CharAnimResource");
    if (failed) {
        return false;
    }
    return mAnim != nullptr;
}

// Reconstructed from eboot.elf at 0x2E0D90.
void CharAnimResource::Save(BinStream& stream, bool cached) {
    EntityResource::Save(stream, cached);
    if (cached) {
        const unsigned int rev = static_cast<unsigned int>(CharAnim::kRev);
        stream.WriteEndian(&rev, 4);
        mAnim->Save(stream, true);
    }
}

// Reconstructed from eboot.elf at 0x2E0E10.
GameObject* CharAnimResource::GetObject(Entity* entity) const {
    static_cast<void>(entity);
    return mEntity->GetRoot();
}

// Reconstructed from eboot.elf at 0x2E0E50.
bool CharAnimResource::IsLooping() const {
    return mCom->mIsLooping;
}

// Reconstructed from eboot.elf at 0x2E0E70.
void CharAnimResource::SetLooping(bool looping) {
    mCom->mIsLooping = looping;
}

// Reconstructed from eboot.elf at 0x2E0E90.
float CharAnimResource::GetStartFrame() const {
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2E0EB0.
float CharAnimResource::GetEndFrame() const {
    return static_cast<float>(mAnim->mKeyframes.mNumKeyframes) + -1.0f;
}

// Reconstructed from eboot.elf at 0x2E0EF0.
TimeUnits CharAnimResource::GetUnits() const {
    return mCom->mPlaybackUnits != CharAnim::kRealTime ? kTimeUnitsBeats : kTimeUnitsSeconds;
}

// Reconstructed from eboot.elf at 0x2E0F30.
float CharAnimResource::GetTimeFromFrame(float frame, TimeUnits units) const {
    if (units == kNumTimeUnits) {
        units = GetUnits();
    }
    if (units == kTimeUnitsBeats) {
        return mAnim->GetBeatFromFrame(frame);
    }
    return frame / mAnim->mFramesPerSecond;
}

// Reconstructed from eboot.elf at 0x2E0FD0.
float CharAnimResource::GetFrameFromTime(float time, TimeUnits units) const {
    if (units == kNumTimeUnits) {
        units = GetUnits();
    }
    if (units == kTimeUnitsBeats) {
        return mAnim->GetFrameFromBeat(time);
    }
    return time * mAnim->mFramesPerSecond;
}

// Reconstructed from eboot.elf at 0x2E1070.
float CharAnimResource::GetTimeAlignment(TimeUnits units) const {
    if (units == kNumTimeUnits) {
        units = GetUnits();
    }
    if (units == kTimeUnitsBeats) {
        const unsigned int index = static_cast<unsigned int>(mCom->mPlaybackUnits - CharAnim::kBeatAlign1);
        if (index <= 2) {
            return kTimeAlignments[index];
        }
    }
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2E1120.
const PropArray<AnimResource::Event>* CharAnimResource::GetEvents(Entity* entity) const {
    static_cast<void>(entity);
    return &mCom->mEvents;
}

// Reconstructed from eboot.elf at 0x2E1140.
float CharAnimResource::GetEventFrameOffset() const {
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2E1160.
AnimResource::BlendMode CharAnimResource::GetBlendMode() const {
    return mAnim->mBlendMode;
}

// Reconstructed from eboot.elf at 0x2E1180.
unsigned long CharAnimResource::GetSubTreeIndex() const {
    return mAnim->mSubTreeIndex;
}

// Reconstructed from eboot.elf at 0x2E11A0.
Entity* CharAnimResource::CreateEntity() {
    Entity* const entity = EntityResource::CreateEntity();
    mCom = static_cast<CharAnimCom*>(entity->GetRoot()->CreateComponent(CharAnimCom::sClassName, false));
    return entity;
}

// Reconstructed from eboot.elf at 0x2E11E0.
ResourceMetaData* CharAnimResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2E11F0.
Symbol CharAnimResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2E1290.
bool CharAnimResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2E12C0.
bool CharAnimResource::Fail() const {
    if (mEntity == nullptr) {
        return true;
    }
    return mAnim == nullptr;
}

// Reconstructed from eboot.elf at 0x2E12E0.
Resource* CharAnimResource::AsResource() {
    return this;
}

// Reconstructed from eboot.elf at 0x2E12F0.
void* CharAnimResource::GetCharData() const {
    return mAnim;
}

// Reconstructed from eboot.elf at 0x2B5E00. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* CharAnimResource::_Create() {
    return new CharAnimResource();
}

// Reconstructed from eboot.elf at 0x2ADD10. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharAnimResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("EntityResource"), true);
}
