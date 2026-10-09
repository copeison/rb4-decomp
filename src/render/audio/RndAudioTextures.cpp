#include "render/audio/RndAudioTextures.h"

#include <cstring>

#include "audio/core/analysis/AudioAnalysis.h"
#include "os/platform/PlatformMgr.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Rows per texture, one per analysis slot. Name not in the reference map.
constexpr int kAnalysisRows = AudioAnalysis::kNumSlots;

// Resource kind whose default wrap and filter modes the textures use. Name
// not in the reference map.
constexpr unsigned int kAudioTextureKind = 0x1D;

bool WidthChanged(const RndTextureBase* texture, int requestedWidth) {
    return requestedWidth > 0 &&
        (texture == nullptr ||
         texture->mBaseDesc.mWidth != static_cast<unsigned int>(requestedWidth));
}

// Writes a slot's values into its texture row, repeating each value when the
// row is wider than the values. Inlined twice into Update. Name not in the
// reference map.
void CopyToRow(RndTexture2D& texture, int slot, const float* values, int count) {
    const int width = texture.mPixels.mSize.x;
    const int repeat = width / count;
    float* row = static_cast<float*>(texture.mPixels.mBuffer) + slot * width;
    if (repeat == 1) {
        std::memcpy(row, values, sizeof(float) * count);
        return;
    }
    for (int index = 0; index < count; ++index) {
        for (int copy = 0; copy < repeat; ++copy) {
            *row++ = values[index];
        }
    }
}

}  // namespace

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
    // The binary sets the size without constructing the elements, which
    // GetRequestedWidths zeroes first.
    FixedVector<int, 2> widths;
    widths.mSize = 2;
    GetRequestedWidths(widths);
    if (WidthChanged(mTextures.mData[0], widths[0]) ||
        WidthChanged(mTextures.mData[1], widths[1])) {
        Rebuild();
    }
    Update(context);
}

// Reconstructed from eboot.elf at 0x4578B0. The old textures are deleted but
// their slots keep the pointers, and a slot whose requested width is zero is
// not replaced. Both textures are built from one description, whose pixel
// level is recreated for each.
void RndAudioTextures::Rebuild() {
    for (unsigned long index = 0; index < mTextures.mSize; ++index) {
        if (mTextures.mData[index] != nullptr) {
            delete mTextures.mData[index];
        }
    }

    RndTexture2D::Description desc;
    desc.mName = "Audio Analysis";
    desc.mRequestedFormat.mWrapMode = TextureDefaultWrapMode(kAudioTextureKind);
    desc.mRequestedFormat.mFilterMode =
        TextureDefaultFilterMode(kAudioTextureKind);
    desc.mRequestedFormat.mSettings[5] = 1;
    desc.mRequestedFormat.mFlags = 1;

    // Linear float data of any bit width the platform supports.
    const RndDataFormatInfo floatFormat{0, 10, 2, 1, -1};
    const int dataFormat = RndFindSupportedDataFormat(floatFormat, kPlatformPS4);

    FixedVector<int, 2> widths;
    widths.mSize = 2;
    GetRequestedWidths(widths);

    int sampleCount = widths[0] > 0 ? widths[0] : 0;
    if (sampleCount < widths[1]) {
        sampleCount = widths[1];
    }
    mSamples.resize(static_cast<unsigned int>(sampleCount));
    if (sampleCount > 0) {
        // The binary writes eight values whatever the count.
        float* samples = mSamples.data();
        for (int index = 0; index < kAnalysisRows; ++index) {
            samples[index] = (index & 1) != 0 ? 0.75f : 0.25f;
        }
    }

    for (unsigned long texture = 0; texture < 2; ++texture) {
        const int width = widths[texture];
        if (width == 0) {
            continue;
        }
        desc.mPixels.Create(width, kAnalysisRows, 1, dataFormat, nullptr);
        const int pixelWidth = desc.mPixels.mSize.x;
        const int repeat = pixelWidth / kAnalysisRows;
        auto* pixels = static_cast<float*>(desc.mPixels.mBuffer);
        const float* samples = mSamples.data();
        for (int row = 0; row < kAnalysisRows; ++row) {
            float* out = pixels + row * pixelWidth;
            for (int index = 0; index < kAnalysisRows; ++index) {
                for (int copy = 0; copy < repeat; ++copy) {
                    *out++ = samples[index];
                }
            }
        }
        mTextures.mData[texture] = RndTexture2D::New(desc);
    }
}

// Reconstructed from eboot.elf at 0x458110. The textures are used without a
// null check.
void RndAudioTextures::Update(RndContext& context) {
    for (int slot = 0; slot < AudioAnalysis::kNumSlots; ++slot) {
        AudioAnalysis* analysis = AudioAnalysis::Get(slot);
        if (!analysis->IsActive()) {
            continue;
        }
        if (analysis->FFTEnabled()) {
            auto* texture = static_cast<RndTexture2D*>(mTextures.mData[0]);
            analysis->GetFFTResults(mSamples.data(), analysis->FFTSize());
            CopyToRow(*texture, slot, mSamples.data(), analysis->FFTSize());
        }
        if (analysis->SemitoneFilterBankEnabled()) {
            auto* texture = static_cast<RndTexture2D*>(mTextures.mData[1]);
            analysis->GetSemitoneResults(
                mSamples.data(), analysis->SemitoneRange());
            CopyToRow(*texture, slot, mSamples.data(), analysis->SemitoneRange());
        }
    }

    for (RndTextureBase* texture : mTextures) {
        if (texture != nullptr) {
            texture->_SyncDynamicImpl(context);
        }
    }
}

// Reconstructed from eboot.elf at 0x4584B0.
void RndAudioTextures::GetRequestedWidths(FixedVector<int, 2>& widths) const {
    widths[0] = 0;
    widths[1] = 0;
    for (int slot = 0; slot < AudioAnalysis::kNumSlots; ++slot) {
        const AudioAnalysis* analysis = AudioAnalysis::Get(slot);
        if (!analysis->IsActive()) {
            continue;
        }
        if (analysis->FFTEnabled()) {
            const int size = analysis->FFTSize();
            widths[0] = widths[0] < size ? size : widths[0];
        }
        if (analysis->SemitoneFilterBankEnabled()) {
            const int range = analysis->SemitoneRange();
            widths[1] = widths[1] < range ? range : widths[1];
        }
    }
}
