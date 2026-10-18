#pragma once

#include <atomic>
#include <cstddef>

#include "entity/props/PropArray.h"
#include "entity/resources/Resource.h"
#include "math/color/Color.h"
#include "render/materials/RndMaterialRuntimeDataResource.h"
#include "utl/containers/Vector.h"

class Component;
class RndContext;
class RndShaderCBuffer;
class RndShaderGraph;
class RndShaderGraphResource;
class RndTextureBase;
struct RndSceneBatchContext;

enum RndShaderGeoType : int;

// Per-material GPU data: the shader graph it was built from, its constant
// buffer and the textures it holds (render/RndMaterialRuntimeData.o,
// 0x4F71E0 to 0x4F998F). Released at a frame boundary once the GPU is done
// with it. Field names are not in the reference map.
class RndMaterialRuntimeData {
public:
    // Bits of mSyncFlags. Names not in the reference map.
    enum SyncFlags : unsigned int {
        // Set while the data waits in a scene drawer's GPU-data queue.
        kSyncQueued = 1,
        // Set when the exposed properties must be copied to the constant
        // buffer.
        kSyncExposedProps = 2,
    };

    explicit RndMaterialRuntimeData(RndShaderGraphResource* graph);  // 0x4F72A0
    ~RndMaterialRuntimeData();  // 0x4F7580

    // Allocates the data for the graph.
    static RndMaterialRuntimeData* New(RndShaderGraphResource* graph);  // 0x4F71E0
    // Deletes the data, or hands it to RndDevice::SyncFreeMaterialData when
    // it was used in the device's current frame.
    static void Delete(RndMaterialRuntimeData* data);  // 0x4F7210
    // Whether the data was built from the graph and still matches its
    // constant-buffer size and exposed textures.
    bool IsValid(const RndShaderGraphResource* graph) const;  // 0x4F7720
    // The graph's scene-texture, scene-texture-capture and scene-depth
    // levels, -1 without a graph, and its linear-depth flag. The scene
    // drawer's cull analysis reads them. The map has bool UsesSceneTex,
    // NeedsSceneTexCapture and UsesSceneDepth in this order; names not in
    // the reference map.
    int GetSceneTexUsage() const;         // 0x4F7800
    int GetSceneTexCaptureUsage() const;  // 0x4F7820
    int GetSceneDepthUsage() const;       // 0x4F7840
    bool UsesLinearDepth() const;         // 0x4F7860
    // Writes the blend parameters of mBlendMode and mBlendFactor to the
    // constant buffer. The map's signature takes a RndContext&.
    void SyncCBufferMaterialParams();  // 0x4F7880
    // Copies the exposed properties out of the material's dynamic
    // properties, transposing the matrices. The map's signature starts
    // with a RndContext&.
    void SyncCBufferExposedProps(const PropArray<unsigned char>& props);  // 0x4F78F0
    // Rebuilds a dirty constant buffer (its slot 2). Name not in the
    // reference map.
    void SyncCBufferStatic();  // 0x4F7C00
    // Uploads a dirty constant buffer through the context (its slot 3). The
    // map's SyncCBufferGlobals(RndContext&) fits by size; the evidence is
    // weak.
    void SyncCBufferGlobals(RndContext& context);  // 0x4F7C30
    // Loads the textures of the material's exposed texture properties. The
    // map's signature is LoadExposedTextures(Component&, ObjPtr const&).
    // Not reconstructed: the property read (0x18B390, entity/PropUtl.o) is
    // not declared.
    void LoadExposedTextures(Component& material);  // 0x4F7C60
    // Releases the exposed textures.
    void _ClearExposedTextures();  // 0x4F8770
    // Builds mUsageHints from the graph and the render state.
    void InitUsageHints();  // 0x4F8D90
    // Selects the graph's programs and binds the material's textures for
    // a batch, or the error shader when the graph has none or its select
    // fails. The map has SelectShader(RndContext&, RndShaderGeoType); this
    // build passes the batch context the textures are taken from.
    void SelectShader(
        RndContext& context,
        const RndSceneBatchContext& batch,
        RndShaderGeoType geoType);  // 0x4F8D10
    // Lays out the "Material" constant buffer from the graph's exposed
    // properties and creates it, when it has any constants.
    void _CreateCBuffer();  // 0x4F7470
    // Caches the graph's IsLit, UsesSceneTex and UsesSceneDepth in
    // mRootFlags; inlined into the constructor. Name not in the reference
    // map.
    void _CacheGraphFlags();  // 0x4F7530

    // The device frame count when the data was last used; -1 when new.
    unsigned long mFrameStamp;
    ResourcePtr<RndShaderGraphResource> mShaderGraph;
    // The shader-graph component on the resource's root object; its
    // exposed properties lay out the constant buffer.
    RndShaderGraph* mGraph;
    // The graph's IsLit, UsesSceneTex and UsesSceneDepth, cached by the
    // constructor.
    bool mRootFlags[3];
    // The material's render state, copied by RndMaterialCom when it changes:
    // "blend_mode" (-1 until set), "blend_factor" (white), "bucket",
    // "depth_prepass", "force_opaque" and "scene_mask".
    int mBlendMode;
    Hmx::Color mBlendFactor;
    int mBucket;
    bool mDepthPrepass;
    bool mForceOpaque;
    bool mSceneMask;
    // One usage-hint mask per quality level (RndConfig::mQualityLevel),
    // built by InitUsageHints.
    unsigned int mUsageHints[3];
    RndShaderCBuffer* mCBuffer;
    // Constant offsets in the "Material" buffer: gVolumetricBlendParams,
    // then the first exposed property.
    unsigned long mVolumetricBlendParams;
    unsigned long mExposedProps;
    // The loaded exposed textures by RndTextureBase::Type: 1D, 2D, cube and
    // 2D array.
    eastl::vector<ResourcePtr<Resource>> mExposedTextures1D;
    eastl::vector<ResourcePtr<Resource>> mExposedTextures2D;
    eastl::vector<ResourcePtr<Resource>> mExposedTexturesCube;
    eastl::vector<ResourcePtr<Resource>> mExposedTexturesArray2D;
    // The texture of each of the graph's exposed textures, in its order,
    // falling back to a default texture.
    eastl::vector<RndTextureBase*> mTextures;
    // SyncFlags.
    std::atomic<unsigned int> mSyncFlags;
};

static_assert(offsetof(RndMaterialRuntimeData, mShaderGraph) == 8);
static_assert(offsetof(RndMaterialRuntimeData, mGraph) == 16);
static_assert(offsetof(RndMaterialRuntimeData, mRootFlags) == 24);
static_assert(offsetof(RndMaterialRuntimeData, mBlendMode) == 28);
static_assert(offsetof(RndMaterialRuntimeData, mBlendFactor) == 32);
static_assert(offsetof(RndMaterialRuntimeData, mBucket) == 48);
static_assert(offsetof(RndMaterialRuntimeData, mDepthPrepass) == 52);
static_assert(offsetof(RndMaterialRuntimeData, mSceneMask) == 54);
static_assert(offsetof(RndMaterialRuntimeData, mUsageHints) == 56);
static_assert(offsetof(RndMaterialRuntimeData, mCBuffer) == 72);
static_assert(offsetof(RndMaterialRuntimeData, mVolumetricBlendParams) == 80);
static_assert(offsetof(RndMaterialRuntimeData, mExposedTextures1D) == 96);
static_assert(offsetof(RndMaterialRuntimeData, mExposedTexturesArray2D) == 192);
static_assert(offsetof(RndMaterialRuntimeData, mTextures) == 224);
static_assert(offsetof(RndMaterialRuntimeData, mSyncFlags) == 256);
static_assert(sizeof(RndMaterialRuntimeData) == 264);
