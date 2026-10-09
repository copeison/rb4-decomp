#include "render/lighting/RndLightGlobals.h"

#include "render/buffers/RndComputeBuffer.h"
#include "render/meshes/RndMesh.h"
#include "render/shaders/RndShader.h"
#include "render/textures/RndTextureBase.h"

namespace {

constexpr const char* kTiledLightIdsCountName = "Tiled Light Ids Count";

// The first two vtable slots of an owner whose type is not recovered yet.
// Name not in the reference map.
struct UnrecoveredOwnerVtable {
    void (*mDestroy)(void* object);
    void (*mDelete)(void* object);
};

// Runs an unrecovered owner's virtual deleting destructor, as delete would.
// Name not in the reference map.
void DeleteUnrecovered(void*& object) {
    if (object != nullptr) {
        (*static_cast<UnrecoveredOwnerVtable**>(object))->mDelete(object);
    }
    object = nullptr;
}

template <class T>
void SafeDelete(T*& object) {
    delete object;
    object = nullptr;
}

void ReleaseResource(ResourcePtr<Resource>& resource) {
    if (resource.mResource != nullptr) {
        resource.mResource->ReleaseRef();
    }
    resource.mResource = nullptr;
}

// The loop re-reads the size after each deletion and leaves the deleted
// pointers in place until the vector is cleared.
void DeleteAndClear(eastl::vector<RndTextureBase*>& textures) {
    for (unsigned long index = 0; index < textures.size(); ++index) {
        delete textures[index];
    }
    textures.clear();
}

RndTextureBase* FindByWidth(
    const eastl::vector<RndTextureBase*>& textures,
    int size) {
    for (auto* texture : textures) {
        if (texture->mBaseDesc.mWidth == static_cast<unsigned int>(size)) {
            return texture;
        }
    }
    return nullptr;
}

}  // namespace

// Reconstructed from eboot.elf at 0x47EEE0.
RndLightGlobals::RndLightGlobals()
    : mSphereMesh(nullptr),
      mSpotlightMesh(nullptr),
      mSphereScale(-1.0F),
      mPrimaryGroupSize(16),
      mSecondaryGroupSize(8),
      mShaders{},
      mTiledLightIdsCount(nullptr),
      mOptionalShaders{},
      mInlineLightingData{},
      mMoreShaders{},
      mUnknown200{},
      mUnknown280{} {}

// Reconstructed from eboot.elf at 0x47EFA0. The vectors and the resource
// references release themselves.
RndLightGlobals::~RndLightGlobals() {}

// Reconstructed from eboot.elf at 0x47F030.
void RndLightGlobals::Init() {
    _InitMeshes();
    _InitShaders();
    _InitBuffers();
}

// Reconstructed from eboot.elf at 0x47F500.
void RndLightGlobals::_InitBuffers() {
    const RndComputeBuffer::Description desc{
        4,
        1,
        nullptr,
        nullptr,
        8,
        1,
        kTiledLightIdsCountName,
    };
    mTiledLightIdsCount = RndComputeBuffer::New(desc);
}

// Reconstructed from eboot.elf at 0x47F580.
void RndLightGlobals::Terminate() {
    SafeDelete(mSphereMesh);
    SafeDelete(mSpotlightMesh);
    for (auto*& shader : mShaders) {
        SafeDelete(shader);
    }
    for (auto*& shader : mOptionalShaders) {
        SafeDelete(shader);
    }
    DeleteUnrecovered(mUnknown280[2]);
    SafeDelete(mMoreShaders[0]);
    SafeDelete(mMoreShaders[1]);
    SafeDelete(mTiledLightIdsCount);
    DeleteUnrecovered(mUnknown200[0]);
    DeleteUnrecovered(mUnknown200[1]);
    DeleteAndClear(mProbeCaptureDownsampleTextures);
    DeleteAndClear(mProbeCaptureHelperTextures);
    DeleteUnrecovered(mUnknown280[0]);
    DeleteUnrecovered(mUnknown280[1]);
    ReleaseResource(mErrorLightCookie);
    ReleaseResource(mSkinDiffusion);
    ReleaseResource(mInlineLightingTextures);
    mInlineLightingData[0] = nullptr;
    mInlineLightingData[1] = nullptr;
}

// Reconstructed from eboot.elf at 0x47FD60.
RndTextureBase* RndLightGlobals::GetProbeCaptureDownsampleTexture(int size) {
    return FindByWidth(mProbeCaptureDownsampleTextures, size);
}

// Reconstructed from eboot.elf at 0x47FDA0.
RndTextureBase* RndLightGlobals::GetProbeCaptureHelperTexture(int size) {
    return FindByWidth(mProbeCaptureHelperTextures, size);
}
