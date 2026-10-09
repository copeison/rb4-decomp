#include "render/textures/RndPixelData.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "os/memory/MemMgr.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndPixelCanvas.h"
#include "utl/streams/BinStream.h"

namespace {

struct ChannelOrder {
    unsigned int count;
    int indices[4];
};

constexpr int kConstantOne = -1;
constexpr ChannelOrder kChannelOrders[]{
    {2, {0, 1, 0, 0}},
    {2, {1, 0, 0, 0}},
    {3, {0, 1, 2, 0}},
    {3, {2, 1, 0, 0}},
    {4, {0, 1, 2, 3}},
    {4, {0, 1, 2, kConstantOne}},
    {4, {2, 1, 0, 3}},
    {4, {2, 1, 0, kConstantOne}},
    {4, {3, 0, 1, 2}},
    {4, {kConstantOne, 0, 1, 2}},
    {1, {0, 0, 0, 0}},
};

bool GetChannelOrder(unsigned int layout, ChannelOrder& order) {
    if (layout >= sizeof(kChannelOrders) / sizeof(kChannelOrders[0])) {
        return false;
    }
    order = kChannelOrders[layout];
    return true;
}

float LinearToGammaChannel(float value) {
    if (value > 0.0031308F) {
        return std::pow(value, 1.0F / 2.4F) * 1.055F - 0.055F;
    }
    return value * 12.92F;
}

Hmx::Color LinearToGamma(const Hmx::Color& color) {
    return {
        LinearToGammaChannel(color.red),
        LinearToGammaChannel(color.green),
        LinearToGammaChannel(color.blue),
        color.alpha,
    };
}

std::uint16_t FloatToHalfTruncated(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));

    const auto sign = static_cast<std::uint16_t>((bits >> 16) & 0x8000U);
    const auto exponent = (bits >> 23) & 0xFFU;
    const auto mantissa = bits & 0x7FFFFFU;
    if (exponent == 0xFFU) {
        if (mantissa == 0) {
            return static_cast<std::uint16_t>(sign | 0x7C00U);
        }
        auto payload = static_cast<std::uint16_t>(mantissa >> 13);
        if (payload == 0) {
            payload = 1;
        }
        return static_cast<std::uint16_t>(sign | 0x7C00U | payload);
    }

    const auto half_exponent = static_cast<std::int32_t>(exponent) - 112;
    if (half_exponent >= 31) {
        return static_cast<std::uint16_t>(sign | 0x7C00U);
    }
    if (half_exponent <= 0) {
        if (half_exponent <= -11) {
            return sign;
        }
        const auto significand = mantissa | 0x800000U;
        return static_cast<std::uint16_t>(
            sign | (significand >> (14 - half_exponent)));
    }
    return static_cast<std::uint16_t>(
        sign | (static_cast<std::uint32_t>(half_exponent) << 10) |
        (mantissa >> 13));
}

template <typename Value>
void WriteValue(std::uint8_t*& destination, Value value) {
    std::memcpy(destination, &value, sizeof(value));
    destination += sizeof(value);
}

std::uint32_t PackUNorm(float value, std::uint32_t maximum) {
    if (!(value <= 1.0F)) {
        return maximum;
    }
    if (value <= 0.0F) {
        return 0;
    }
    return static_cast<std::uint32_t>(value * static_cast<float>(maximum));
}

bool WriteUNormPixel(
    std::uint8_t*& destination,
    const Hmx::Color& pixel,
    const RndDataFormatInfo& format) {
    ChannelOrder order{};
    if (!GetChannelOrder(format.mOrder, order) ||
        format.mBitsPerPixel % order.count != 0) {
        return false;
    }

    const auto component_width = format.mBitsPerPixel / order.count;
    if (component_width != 8 && component_width != 16) {
        return false;
    }

    const float channels[]{pixel.red, pixel.green, pixel.blue, pixel.alpha};
    const auto maximum = component_width == 8
        ? static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::max())
        : static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max());
    for (unsigned int index = 0; index < order.count; ++index) {
        const auto channel = order.indices[index] == kConstantOne
            ? 1.0F
            : channels[order.indices[index]];
        const auto packed = PackUNorm(channel, maximum);
        if (component_width == 8) {
            WriteValue(destination, static_cast<std::uint8_t>(packed));
        } else {
            WriteValue(destination, static_cast<std::uint16_t>(packed));
        }
    }
    return true;
}

bool WriteFloatPixel(
    std::uint8_t*& destination,
    const Hmx::Color& pixel,
    const RndDataFormatInfo& format) {
    ChannelOrder order{};
    if (!GetChannelOrder(format.mOrder, order) ||
        format.mBitsPerPixel % order.count != 0) {
        return false;
    }

    const auto component_width = format.mBitsPerPixel / order.count;
    if (component_width != 16 && component_width != 32) {
        return false;
    }

    const float channels[]{pixel.red, pixel.green, pixel.blue, pixel.alpha};
    for (unsigned int index = 0; index < order.count; ++index) {
        const auto channel = order.indices[index] == kConstantOne
            ? 1.0F
            : channels[order.indices[index]];
        if (component_width == 16) {
            WriteValue(destination, FloatToHalfTruncated(channel));
        } else {
            WriteValue(destination, channel);
        }
    }
    return true;
}

}  // namespace

// Reconstructed from eboot.elf at 0x682930.
RndPixelData::RndPixelData()
    : mSize{0, 0, 0},
      mFormat(-1),
      mBuffer(nullptr),
      mBufferSize(0),
      mMip(nullptr),
      mFlags(0),
      mUnknown52{},
      mUnknown72(nullptr) {}

// Reconstructed from eboot.elf at 0x682960.
RndPixelData::RndPixelData(const RndPixelData& other, bool keepPixels)
    : RndPixelData() {
    CopyFrom(other, keepPixels);
}

// Reconstructed from eboot.elf at 0x682BC0. The deleting destructor at
// 0x682CB0 returns the object to the pool.
RndPixelData::~RndPixelData() {
    Free();
}

// Reconstructed from eboot.elf at 0x682C40.
void RndPixelData::Free() {
    delete[] static_cast<std::uint8_t*>(mBuffer);
    mBuffer = nullptr;
    mBufferSize = 0;
    delete mMip;
    mMip = nullptr;
    mSize = {0, 0, 0};
    mFormat = -1;
    operator delete(mUnknown72);
    mUnknown72 = nullptr;
}

// Reconstructed from eboot.elf at 0x6829A0. Source pixels are copied under
// the temporary heap unless `keepPixels` is set; a matching-size buffer is
// reused. Mip levels are copied recursively into pool allocations.
void RndPixelData::CopyFrom(const RndPixelData& other, bool keepPixels) {
    mFlags = other.mFlags;
    std::memcpy(mUnknown52, other.mUnknown52, sizeof(mUnknown52));
    if (other.mBuffer != nullptr) {
        unsigned int heap = 0;
        MemPushTemp(heap, true, !keepPixels);
        const auto* pixels = other.mBuffer;
        const auto size = other.mBufferSize;
        delete mMip;
        mMip = nullptr;
        const auto oldSize = mBufferSize;
        mSize = other.mSize;
        mFormat = other.mFormat;
        mBufferSize = size;
        if (oldSize != size || mBuffer == nullptr) {
            delete[] static_cast<std::uint8_t*>(mBuffer);
            mBufferSize = size;
            mBuffer = new std::uint8_t[size];
        }
        std::memcpy(mBuffer, pixels, size);
        MemPopTemp(heap);
    } else {
        const auto format = other.mFormat;
        Free();
        mSize = other.mSize;
        mFormat = format;
        mBufferSize = other.mBufferSize;
    }

    if (other.mMip != nullptr) {
        mMip = new RndPixelData;
        mMip->CopyFrom(*other.mMip, keepPixels);
    }
}

// Reconstructed from eboot.elf at 0x6830D0.
void RndPixelData::CreateEmpty(int width, int height, int depth, int format) {
    Free();
    mSize = {width, height, depth};
    mFormat = format;
}

// Reconstructed from eboot.elf at 0x682E80. The buffer holds
// width * height * depth pixels of the format's bit width; a matching-size
// buffer is reused.
void RndPixelData::Create(const Vector3i& size, int format, const void* pixels) {
    mSize = size;
    mFormat = format;
    const auto info = RndGetDataFormatInfo(format);
    const auto bytes = static_cast<unsigned long>(
        static_cast<long>(mSize.x) * mSize.y * mSize.z *
        static_cast<long>(static_cast<int>(info.mBitsPerPixel))) >> 3;
    delete mMip;
    mMip = nullptr;
    const auto oldSize = mBufferSize;
    mSize = size;
    mFormat = format;
    mBufferSize = bytes;
    if (oldSize != bytes || mBuffer == nullptr) {
        delete[] static_cast<std::uint8_t*>(mBuffer);
        mBufferSize = bytes;
        mBuffer = new std::uint8_t[bytes];
    }
    if (pixels != nullptr) {
        std::memcpy(mBuffer, pixels, bytes);
    }
}

// Reconstructed from eboot.elf at 0x684960, with conversion kernels from
// 0x6897D0 through 0x68CB1B. Unsupported channel layouts stop the
// conversion without reporting failure, as in the binary.
bool RndPixelData::ConvertFrom(const RndPixelCanvas& canvas) {
    const Vector3i size{canvas.mWidth, canvas.mHeight, canvas.mDepth};
    Create(size, mFormat, nullptr);

    const auto format = RndGetDataFormatInfo(mFormat);
    if (format.mCompression != 0) {
        return false;
    }

    const auto count = static_cast<unsigned long>(canvas.mWidth) *
        static_cast<unsigned long>(canvas.mHeight) *
        static_cast<unsigned long>(canvas.mDepth);
    if (count != 0 && canvas.mPixels == nullptr) {
        return false;
    }

    auto* destination = static_cast<std::uint8_t*>(mBuffer);
    for (unsigned long index = 0; index < count; ++index) {
        const auto pixel = format.mGamma == 2
            ? LinearToGamma(canvas.mPixels[index])
            : canvas.mPixels[index];
        const auto converted = format.mStorage == 0
            ? WriteUNormPixel(destination, pixel, format)
            : WriteFloatPixel(destination, pixel, format);
        if (!converted) {
            return true;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x683260.
void RndPixelData::FreeBuffers() {
    for (auto* level = this; level != nullptr; level = level->mMip) {
        delete[] static_cast<std::uint8_t*>(level->mBuffer);
        level->mBuffer = nullptr;
        level->mBufferSize = 0;
    }
}

// Reconstructed from eboot.elf at 0x686340.
unsigned long RndPixelData::GetTotalBytes() const {
    unsigned long bytes = 0;
    for (auto* level = this; level != nullptr; level = level->mMip) {
        bytes += level->mBufferSize;
    }
    return bytes;
}

// Reconstructed from eboot.elf at 0x6832D0. Counts the levels after this one,
// so a texture without mips reports zero.
unsigned long RndPixelData::GetNumMips() const {
    unsigned long count = static_cast<unsigned long>(-1);
    const auto* level = this;
    do {
        level = level->mMip;
        ++count;
    } while (level != nullptr);
    return count;
}

// Reconstructed from eboot.elf at 0x686B10.
void RndPixelData::LoadBuffers(BinStream& stream) {
    int revision;
    stream.ReadEndian(&revision, sizeof(revision));
    constexpr unsigned int kNoMips = 4;
    if (revision <= 5) {
        for (auto* level = this; level != nullptr; level = level->mMip) {
            const auto info = RndGetDataFormatInfo(level->mFormat);
            const auto size = static_cast<unsigned long>(
                static_cast<long>(level->mSize.x) * level->mSize.y * level->mSize.z *
                info.mBitsPerPixel) >> 3;
            level->mBufferSize = size;
            delete[] static_cast<unsigned char*>(level->mBuffer);
            level->mBufferSize = size;
            level->mBuffer = new unsigned char[size];
            stream.Read(level->mBuffer, level->mBufferSize);
            if ((mFlags & kNoMips) != 0) {
                break;
            }
        }
    } else {
        for (auto* level = this; level != nullptr; level = level->mMip) {
            int size;
            stream.ReadEndian(&size, sizeof(size));
            level->mBufferSize = size;
            delete[] static_cast<unsigned char*>(level->mBuffer);
            level->mBufferSize = size;
            level->mBuffer = new unsigned char[size];
            stream.Read(level->mBuffer, level->mBufferSize);
            if ((mFlags & kNoMips) != 0) {
                break;
            }
        }
    }
}
