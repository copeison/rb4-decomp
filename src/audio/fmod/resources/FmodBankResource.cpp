#include "audio/fmod/resources/FmodBankResource.h"

// Reconstructed from eboot.elf at 0x275030.
ResourceMetaData* FModBankResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2750E0.
bool FModBankResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x275040.
Symbol FModBankResource::GetId() const {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("FModBankResource");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x275110.
void FModBankResource::LoadFile() {
    _Load(false);
}

// Reconstructed from eboot.elf at 0x2741F0. Banks are never read from a
// stream.
bool FModBankResource::Load(BinStream&, bool) {
    return false;
}

// Reconstructed from eboot.elf at 0x274E70.
void FModBankResource::Save(BinStream&, bool) {}

// Reconstructed from eboot.elf at 0x275120.
bool FModBankResource::Fail() const {
    return mNumBanks == 0;
}
