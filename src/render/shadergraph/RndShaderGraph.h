#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "math/color/Color.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class RndContext;
class RndMaterialRuntimeData;
class RndShaderCBufferConfig;
class RndShaderNodeRoot;
struct RndSceneBatchContext;
enum RndShaderGeoType : int;
enum RndShaderNumericType : unsigned int;

// The shader graph component on the root object of a shader graph
// resource's entity (render/RndShaderGraph.o; vtable 0x1911428, 41 slots;
// destructor 0x4FA110). Only the queries the material component makes are
// reconstructed (RndShaderGraph.cpp). Each forwards to a slot of the graph's
// root node (RndShaderNodeRoot), found through RndShaderNode::sClassName.
class RndShaderGraph : public Component {
public:
    // The root node on the graph's entity, which must exist: the object's
    // component table is searched without a bound. Inlined into every
    // query below; the map has GetRootNode() and GetRootNode() const.
    RndShaderNodeRoot* GetRootNode() const;

    // Root-node slot 61 (+488).
    bool IsLit() const;  // 0x4FD570
    // Root-node slots 64 and 65 (+512, +520); the material data caches them
    // beside IsLit. The map's UsesSceneTex and UsesSceneDepth; the order is
    // inferred.
    bool UsesSceneTex() const;    // 0x4FDB20
    bool UsesSceneDepth() const;  // 0x4FDB90
    // Root-node slot 68 (+544): one bit per RndBlendMode the graph allows.
    unsigned int GetAllowedBlendModes() const;  // 0x4FD750
    // Root-node slot 69 (+552).
    int GetDefaultBlendMode() const;  // 0x4FDC70
    // Root-node slots 70 and 71 (+560, +568): whether the graph sets the
    // blend factor, and the factor. Names not in the reference map.
    bool HasBlendFactor() const;              // 0x4FDCE0
    Hmx::Color GetBlendFactor() const;        // 0x4FDD50
    // Root-node slots 72-74 (+576, +584, +592): whether the material may
    // pick its "bucket", its "cull_mode" and its "depth_prepass"; the
    // material resets them otherwise. Names not in the reference map; the
    // evidence is the material's resets only.
    bool AllowsBucket() const;        // 0x4FDDC0
    bool AllowsCullMode() const;      // 0x4FDE30
    bool AllowsDepthPrepass() const;  // 0x4FDEA0
    // Selects the graph's programs for the geometry type and binds the
    // material's textures and constants for the batch; false when no
    // program is available. The map has Select(RndContext&,
    // RndMaterialRuntimeData&, RndShaderGeoType); this build also passes
    // the batch context.
    bool Select(
        RndContext& context,
        const RndSceneBatchContext& batch,
        RndMaterialRuntimeData& data,
        RndShaderGeoType geoType);  // 0x4FE630
    // Used by RndMaterialRuntimeData::InitUsageHints. The map's root
    // queries are, in order, IsLit, IsSometimesFwdLit, IsAlwaysFwdLit,
    // GetAllowedBlendModes, IsEmissive, UsesAlphaCut,
    // ModifiesVertexPosition and GetUsageHints; the names below follow
    // that order and are weak. Whether the graph draws forward lit at the
    // quality level: lit, not root slot 66, and the scene texture at
    // level 1, linear depth, root slot 62 or no kSource blend mode.
    bool IsFwdLit(int quality) const;  // 0x4FD7C0
    // Root-node slot 66 (+528).
    bool IsEmissive() const;  // 0x4FD990
    // Root-node slot 63 (+504).
    bool UsesAlphaCut() const;  // 0x4FDAB0
    // Root-node slot 67 (+536).
    unsigned int GetUsageHints() const;  // 0x4FDC00
    // Lays the "Material" constant buffer out: gVolumetricBlendParams, then
    // each exposed property. The offsets of both are returned when the
    // pointers are given. The map's signature has a third offset.
    void FillCBufferConfig(
        RndShaderCBufferConfig& config,
        unsigned long* volumetricBlendParams,
        unsigned long* exposedProps) const;  // 0x4FF110

    // The class symbol, "ShaderGraph", constructed by the static
    // initializer at 0x50ACFA.
    static Symbol sId;  // 0x1A8C1A8

    // The elements of the graph's property arrays, in the map's order of
    // the Add* methods; the constructor (0x4F9D50) gives their sizes. Only
    // the fields RndMaterialRuntimeData reads are named; field names are
    // not in the reference map.
    struct ExposedProperty {
        unsigned char mOpaque[72];
    };
    struct ExposedTextureData {
        // The material property that holds the texture's path, and the
        // struct property it sits in, or the empty symbol.
        Symbol mProperty;
        Symbol mStruct;
        // The texture's RndTextureBase::Type, and the RndDefaults textures
        // drawn while the path is empty and when the texture fails.
        int mType;
        int mUnsetTexture;
        int mFailedTexture;
        unsigned char mOpaque[28];
    };
    struct UnexposedTextureData {
        unsigned char mOpaque[24];
    };
    struct TexSamplerData {
        unsigned char mOpaque[80];
    };
    struct ComputeBufferData {
        unsigned char mOpaque[40];
    };
    // Where an exposed property's value sits in the material's dynamic
    // properties, in "Material" constant-buffer order. Name not in the
    // reference map.
    struct ExposedPropLayout {
        RndShaderNumericType mType;
        unsigned long mOffset;
    };

    // The graph's root node, found by GetRootNode. +22, +23 and +28 to +31
    // are not modelled. Name not in the reference map.
    unsigned char mMembers22[2];
    GameObjectId mRootNode;
    unsigned char mMembers28[4];
    PropArray<ExposedProperty> mExposedProperties;
    PropArray<ExposedTextureData> mExposedTextures;
    PropArray<UnexposedTextureData> mUnexposedTextures;
    PropArray<TexSamplerData> mTexSamplers;
    PropArray<ComputeBufferData> mComputeBuffers;
    // The scene-texture, scene-texture-capture and scene-depth levels
    // (-1 until built) and whether the graph reads linear depth. Names not
    // in the reference map.
    int mSceneTexUsage;
    int mSceneTexCaptureUsage;
    int mSceneDepthUsage;
    bool mUsesLinearDepth;
    // +245 (set by the constructor) to +248; not modelled.
    unsigned char mMembers245[3];
    // The graph's compiled shaders, which RndMaterialCom passes to
    // RndDevice::PrecacheMaterialShaders. Not decoded; the name is weak.
    void* mShaders;
    // +256 to +312; not modelled.
    unsigned char mMembers256[56];
    eastl::vector<ExposedPropLayout> mExposedPropLayout;
};

static_assert(sizeof(RndShaderGraph::ExposedTextureData) == 56);
static_assert(offsetof(RndShaderGraph, mRootNode) == 24);
static_assert(offsetof(RndShaderGraph, mExposedProperties) == 32);
static_assert(offsetof(RndShaderGraph, mExposedTextures) == 72);
static_assert(offsetof(RndShaderGraph, mComputeBuffers) == 192);
static_assert(offsetof(RndShaderGraph, mSceneTexUsage) == 232);
static_assert(offsetof(RndShaderGraph, mUsesLinearDepth) == 244);
static_assert(offsetof(RndShaderGraph, mShaders) == 248);
static_assert(offsetof(RndShaderGraph, mExposedPropLayout) == 312);
