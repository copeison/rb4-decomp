// anim/CharAnimRawResource.o (0x2DF930 to 0x2DFE8F).
#include "anim/char/CharAnimRawResource.h"

#include "anim/char/CharAnim.h"
#include "anim/char/CharAnimRaw.h"
#include "entity/resources/ResourceMetaData.h"
#include "os/platform/PlatformMgr.h"
#include "utl/streams/BinStream.h"

ResourceMetaData CharAnimRawResource::sMetaData;

// Reconstructed from eboot.elf at 0x2DF930.
CharAnimRawResource::CharAnimRawResource() : mRaw(nullptr) {}

// Reconstructed from eboot.elf at 0x2DF9A0. The deleting destructor is at
// 0x2DF9F0.
CharAnimRawResource::~CharAnimRawResource() {
    delete mRaw;
    mRaw = nullptr;
}

// Reconstructed from eboot.elf at 0x2DFA40.
void CharAnimRawResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("charanimraw"));
    metaData.mCategory = Symbol("Animations");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeFlags[5] = true;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x2DFB60. Only revision CharAnim::kRev of
// the cached file holds an animation; the source file is not read. The
// platform symbol is all that is left of an error report.
bool CharAnimRawResource::Load(BinStream& stream, bool cached) {
    delete mRaw;
    mRaw = nullptr;
    if (!cached) {
        static_cast<void>(PlatformSymbol(kPlatformPS4));
        return false;
    }
    unsigned int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != CharAnim::kRev) {
        return false;
    }
    mRaw = CharAnimRaw::New();
    if (mRaw->Load(stream, false)) {
        return true;
    }
    delete mRaw;
    mRaw = nullptr;
    return false;
}

// Reconstructed from eboot.elf at 0x2DFC50.
void CharAnimRawResource::Save(BinStream& stream, bool cached) {
    if (!cached) {
        return;
    }
    const unsigned int rev = static_cast<unsigned int>(CharAnim::kRev);
    stream.WriteEndian(&rev, 4);
    mRaw->Save(stream, true);
}

// Reconstructed from eboot.elf at 0x2DFCC0.
ResourceMetaData* CharAnimRawResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2DFCD0.
Symbol CharAnimRawResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2DFD70.
bool CharAnimRawResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2DFDA0.
bool CharAnimRawResource::Fail() const {
    return mRaw == nullptr;
}

// Reconstructed from eboot.elf at 0x2B5E30. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* CharAnimRawResource::_Create() {
    return new CharAnimRawResource();
}

// Reconstructed from eboot.elf at 0x2ADE80. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharAnimRawResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("Resource"), true);
}
