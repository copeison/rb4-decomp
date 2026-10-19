// anim/CharLipsyncResource.o (0x2F6700 to 0x2F716F). The constructor
// emitted after it (0x2F7170) is CharSkeleton's, which belongs to
// anim/CharSkeleton.o.
#include "anim/char/CharLipsyncResource.h"

#include "anim/char/CharLipsync.h"
#include "entity/resources/ResourceMetaData.h"
#include "os/files/File.h"
#include "utl/files/FileUtl.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"

ResourceMetaData CharLipsyncResource::sMetaData;

// Reconstructed from eboot.elf at 0x2F6700.
CharLipsyncResource::CharLipsyncResource() : mLipsync(nullptr) {}

// Reconstructed from eboot.elf at 0x2F6770. The deleting destructor is at
// 0x2F67C0.
CharLipsyncResource::~CharLipsyncResource() {
    delete mLipsync;
    mLipsync = nullptr;
}

// Reconstructed from eboot.elf at 0x2F6810.
void CharLipsyncResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("lipsync"));
    metaData.mCategory = Symbol("Animations");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeFlags[5] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x2F6930.
bool CharLipsyncResource::Load(BinStream& stream, bool cached) {
    delete mLipsync;
    mLipsync = nullptr;
    if (!cached) {
        mLipsync = CharLipsync::LoadFromXml(GetXmlPath());
        return mLipsync != nullptr;
    }
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 0) {
        return false;
    }
    if (!gFileArchiveMode) {
        // Outside the archive the cache must be newer than the XML file.
        const FileStat xml = FileTimestamp(GetXmlPath().Str());
        const bool newer = mFileTime.mSeconds != xml.mSeconds ? mFileTime.mSeconds > xml.mSeconds
                                                              : mFileTime.mFraction > xml.mFraction;
        if (!newer) {
            return false;
        }
    }
    mLipsync = new CharLipsync();
    if (mLipsync->Load(stream)) {
        return true;
    }
    delete mLipsync;
    mLipsync = nullptr;
    return false;
}

// Reconstructed from eboot.elf at 0x2F6DD0.
ResourcePath CharLipsyncResource::GetXmlPath() const {
    char folder[512] = {};
    const char* const path = FileGetPath(mPath.Str(), folder);
    char base[512] = {};
    const char* const name = FileGetBase(mPath.Str(), base);
    FormatString format("%s/%s_lipsync.xml");
    format << path << name;
    const char* const xml = format.Str();
    ResourcePath result;
    result = xml;
    return result;
}

// Reconstructed from eboot.elf at 0x2F6FD0.
void CharLipsyncResource::Save(BinStream& stream, bool cached) {
    const int rev = 0;
    stream.WriteEndian(&rev, 4);
    if (cached) {
        mLipsync->Save(stream);
    }
}

// Reconstructed from eboot.elf at 0x2F7040.
ResourceMetaData* CharLipsyncResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2F7050.
Symbol CharLipsyncResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2F70F0.
bool CharLipsyncResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2F7120.
bool CharLipsyncResource::Fail() const {
    return mLipsync == nullptr;
}

// Reconstructed from eboot.elf at 0x2B5ED0. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* CharLipsyncResource::_Create() {
    return new CharLipsyncResource();
}

// Reconstructed from eboot.elf at 0x2AE2D0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharLipsyncResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("Resource"), true);
}
