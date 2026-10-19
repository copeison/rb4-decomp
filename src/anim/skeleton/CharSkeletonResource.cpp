// anim/CharSkeletonResource.o (0x2FB6D0 to 0x2FBBFB).
#include "anim/skeleton/CharSkeletonResource.h"

#include "anim/skeleton/CharSkeleton.h"
#include "entity/resources/ResourceMetaData.h"
#include "os/platform/PlatformMgr.h"
#include "utl/streams/BinStream.h"

ResourceMetaData CharSkeletonResource::sMetaData;

// Reconstructed from eboot.elf at 0x2FB6D0.
CharSkeletonResource::CharSkeletonResource() : mSkeleton(nullptr), mFBXPath("") {}

// Reconstructed from eboot.elf at 0x2FB750.
void CharSkeletonResource::SetFBXPath(const char* path) {
    mFBXPath = path;
    delete mSkeleton;
    mSkeleton = nullptr;
    mSkeleton = CharSkeleton::New();
}

// Reconstructed from eboot.elf at 0x2FB7A0. The deleting destructor is at
// 0x2FB7F0.
CharSkeletonResource::~CharSkeletonResource() {
    delete mSkeleton;
    mSkeleton = nullptr;
}

// Reconstructed from eboot.elf at 0x2FB840.
void CharSkeletonResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("skel"));
    metaData.mCategory = Symbol("Characters");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeFlags[5] = true;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x2FB960. Only revision 1 of the cached
// file holds a skeleton; the source file is not read. The platform symbol
// is all that is left of an error report.
bool CharSkeletonResource::Load(BinStream& stream, bool cached) {
    delete mSkeleton;
    mSkeleton = nullptr;
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    stream >> mFBXPath.mPath;
    if (!cached) {
        static_cast<void>(PlatformSymbol(kPlatformPS4));
        return false;
    }
    if (rev != 1) {
        return false;
    }
    mSkeleton = CharSkeleton::New();
    if (mSkeleton->Load(stream)) {
        return true;
    }
    delete mSkeleton;
    mSkeleton = nullptr;
    return false;
}

// Reconstructed from eboot.elf at 0x2FBA50.
void CharSkeletonResource::Save(BinStream& stream, bool cached) {
    int rev = 1;
    stream.WriteEndian(&rev, 4);
    stream << mFBXPath.mPath;
    if (cached) {
        mSkeleton->Save(stream);
    }
}

// Reconstructed from eboot.elf at 0x2FBAD0.
ResourceMetaData* CharSkeletonResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2FBAE0.
Symbol CharSkeletonResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2FBB80.
bool CharSkeletonResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2FBBB0.
bool CharSkeletonResource::Fail() const {
    return mSkeleton == nullptr;
}

// Reconstructed from eboot.elf at 0x2B5E60. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* CharSkeletonResource::_Create() {
    return new CharSkeletonResource();
}

// Reconstructed from eboot.elf at 0x2ADFF0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharSkeletonResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("Resource"), true);
}
