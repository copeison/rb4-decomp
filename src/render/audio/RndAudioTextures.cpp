#include "render/audio/RndAudioTextures.h"

#include "render/textures/RndTextureBase.h"

// The texture widths the audio analysis requests. Name not in the reference
// map.
struct RndAudioTextureWidths {
    int mWidths[2];
};

namespace {

bool WidthChanged(const RndTextureBase* texture, int requestedWidth) {
    return requestedWidth > 0 &&
        (texture == nullptr ||
         texture->mBaseDesc.mWidth != static_cast<unsigned int>(requestedWidth));
}

}  // namespace

// The source of the requested widths is not identified yet. Name not in the
// reference map.
RndAudioTextureWidths RndGetAudioTextureWidths();

// Reconstructed from eboot.elf at 0x457620.
RndAudioTextures::RndAudioTextures() {
    mTextures.mData = mTextures.mStorage;
    mTextures.mSize = 2;
    mTextures.mCapacity = 2;
    mTextures.mStorage[0] = nullptr;
    mTextures.mStorage[1] = nullptr;
    // The binary leaves the sample allocator without a name.
    mSamples.mAllocator.mName = nullptr;
}

// Reconstructed from eboot.elf at 0x4576A0. The sample vector's destructor
// frees its storage.
RndAudioTextures::~RndAudioTextures() {
    for (unsigned long index = 0; index < mTextures.mSize; ++index) {
        if (mTextures.mData[index] != nullptr) {
            delete mTextures.mData[index];
            mTextures.mData[index] = nullptr;
        }
    }
}

// Reconstructed from eboot.elf at 0x457780.
void RndAudioTextures::PrepareFrame(RndContext& context) {
    const auto widths = RndGetAudioTextureWidths();
    if (WidthChanged(mTextures.mData[0], widths.mWidths[0]) ||
        WidthChanged(mTextures.mData[1], widths.mWidths[1])) {
        Rebuild();
    }
    Update(context);
}
