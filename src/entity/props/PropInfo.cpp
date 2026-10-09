#include "entity/props/PropInfo.h"

#include "entity/props/PropMetadata.h"

// Reconstructed from eboot.elf at 0x12DBE0.
PropInfo::PropInfo() : mOffset(-1), mDynamicOffset(-1), mType(kPropertyNone), mCount(0), mMetadata() {}

// Reconstructed from eboot.elf at 0x12DC00.
bool PropInfo::IsSaved() const {
    return mOffset >= 0 && mMetadata->mFlags[1];
}

// Reconstructed from eboot.elf at 0x12DC20.
const PropInfo& PropInfo::PropArrayItemInfo() const {
    return static_cast<const ArrayMetadata*>(mMetadata.Get())->mItemInfo;
}

// Reconstructed from eboot.elf at 0x12DC70.
PropInfo::PropMetadataPtr& PropInfo::PropMetadataPtr::operator=(PropMetadata* metadata) {
    if (mPtr != metadata) {
        if (mPtr != nullptr && mPtr->mRefCount-- == 1) {
            delete mPtr;
        }
        mPtr = metadata;
        if (metadata != nullptr) {
            ++metadata->mRefCount;
        }
    }
    return *this;
}

// Reconstructed from eboot.elf at 0x12DCB0.
PropInfo::PropMetadataPtr::PropMetadataPtr(const PropMetadataPtr& other) : mPtr(nullptr) {
    if (other.mPtr != nullptr) {
        mPtr = other.mPtr;
        ++mPtr->mRefCount;
    }
}

// Reconstructed from eboot.elf at 0x12DCD0.
PropInfo::PropMetadataPtr::~PropMetadataPtr() {
    if (mPtr != nullptr) {
        if (mPtr->mRefCount-- == 1) {
            delete mPtr;
        }
        mPtr = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x12DD00.
PropInfo::PropMetadataPtr& PropInfo::PropMetadataPtr::operator=(const PropMetadataPtr& other) {
    PropMetadata* const metadata = other.mPtr;
    if (mPtr != metadata) {
        if (mPtr != nullptr && mPtr->mRefCount-- == 1) {
            delete mPtr;
        }
        mPtr = metadata;
        if (metadata != nullptr) {
            ++metadata->mRefCount;
        }
    }
    return *this;
}
