#include "render/textures/RndPixelDataCube.h"

RndPixelDataCube::RndPixelDataCube(const RndPixelDataCube& other, bool keepPixels) {
    for (int face = 0; face < kNumFaces; ++face) {
        mFaces[face].CopyFrom(other.mFaces[face], keepPixels);
    }
}

bool RndPixelDataCube::IsValid() const {
    const auto& first = mFaces[0];
    if (first.mSize.x == 0 || first.mSize.y == 0 ||
        first.mSize.x != first.mSize.y || first.mSize.z != 1) {
        return false;
    }

    const auto numMips = first.GetNumMips();
    for (int face = 1; face < kNumFaces; ++face) {
        const auto& other = mFaces[face];
        if (other.mSize.x != first.mSize.x || other.mSize.y != first.mSize.y ||
            other.mSize.z != first.mSize.z || other.mFormat != first.mFormat ||
            other.GetNumMips() != numMips) {
            return false;
        }
    }
    return true;
}

unsigned long RndPixelDataCube::GetTotalBytes() const {
    unsigned long bytes = 0;
    for (const auto& face : mFaces) {
        bytes += face.GetTotalBytes();
    }
    return bytes;
}

void RndPixelDataCube::FreeBuffers() {
    for (auto& face : mFaces) {
        face.FreeBuffers();
    }
}
