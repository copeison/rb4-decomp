#pragma once

#include <atomic>
#include <cstddef>

#include "entity/props/PropArray.h"
#include "entity/resources/Resource.h"
#include "math/color/Color.h"
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
// buffer and the textures it holds (render/RndMaterialRuntimeData.o). Released
// at a frame boundary once the GPU is done with it. The layout follows the
// constructor (0x4F72A0), the constant-buffer setup (0x4F7470), the texture
// loader (0x4F82D0) and the usage hints (0x4F8D90); field names are not in
// the reference map. The lifetime, the material-parameter sync and
// SelectShader are reconstructed.
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
    // constant-buffer layout and samplers.
    bool IsValid(const RndShaderGraphResource* graph) const;  // 0x4F7720
    // The graph's scene-texture, scene-texture-capture and scene-depth
    // levels (its +0xE8, +0xEC and +0xF0), -1 without a graph, and its
    // linear-depth flag (+0xF4). The scene drawer's cull analysis reads
    // them. The map has bool UsesSceneTex, NeedsSceneTexCapture and
    // UsesSceneDepth in this order; names not in the reference map.
    int GetSceneTexUsage() const;         // 0x4F7800
    int GetSceneTexCaptureUsage() const;  // 0x4F7820
    int GetSceneDepthUsage() const;       // 0x4F7840
    bool UsesLinearDepth() const;         // 0x4F7860
    // Writes the blend parameters of mBlendMode and mBlendFactor to the
    // constant buffer. The map's signature takes a RndContext&.
    void SyncCBufferMaterialParams();  // 0x4F7880
    // Copies the exposed properties out of the material's dynamic
    // properties. The map's signature starts with a RndContext&.
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
    void LoadExposedTextures(Component& material);  // 0x4F7C60
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
    // properties and creates it. Not reconstructed: the graph's
    // FillCBufferConfig and the config's constructor at 0x63A070 are not
    // modelled.
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
    // Exposed textures by RndTextureBase::Type: 1D, 2D, cube and 2D array.
    eastl::vector<ResourcePtr<Resource>> mExposedTextures1D;
    eastl::vector<ResourcePtr<Resource>> mExposedTextures2D;
    eastl::vector<ResourcePtr<Resource>> mExposedTexturesCube;
    eastl::vector<ResourcePtr<Resource>> mExposedTexturesArray2D;
    // The texture bound to each of the graph's samplers, falling back to a
    // default texture.
    eastl::vector<RndTextureBase*> mSamplerTextures;
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
static_assert(offsetof(RndMaterialRuntimeData, mSamplerTextures) == 224);
static_assert(offsetof(RndMaterialRuntimeData, mSyncFlags) == 256);
static_assert(sizeof(RndMaterialRuntimeData) == 264);

// The resource that holds a material's runtime data in its entity, at the
// path "mat_rt:<shader graph>:<layer>_<serial>" (render/
// RndMaterialRuntimeDataResource.o). Only the constructor the material uses
// is declared. Its vtable is at 0x19113B0. The methods are not
// reconstructed. Field names are not in the reference map.
class RndMaterialRuntimeDataResource : public Resource {
public:
    explicit RndMaterialRuntimeDataResource(RndMaterialRuntimeData* data);  // 0x4F9A00

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x4F9C20
    Symbol GetId() const override;                   // slot 1: 0x4F9C30
    bool IsA(Symbol type) const override;            // slot 2: 0x4F9CD0
    bool Load(BinStream& stream, bool cached) override;  // slot 4: 0x4F9BF0
    void Save(BinStream& stream, bool cached) override;  // slot 5: 0x4F9C00
    bool Fail() const override;                      // slot 6: 0x4F9C10
    ~RndMaterialRuntimeDataResource() override;      // slots 10-11: 0x4F9A70, 0x4F9AA0

    // The held data; null when the resource failed.
    RndMaterialRuntimeData* mData;
};

static_assert(offsetof(RndMaterialRuntimeDataResource, mData) == 48);
static_assert(sizeof(RndMaterialRuntimeDataResource) == 56);
