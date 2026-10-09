#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "math/color/Color.h"
#include "utl/text/Symbol.h"

class RndContext;
class RndMaterialRuntimeData;
struct RndSceneBatchContext;
enum RndShaderGeoType : int;

// The shader graph component on the root object of a shader graph
// resource's entity (render/RndShaderGraph.o; vtable 0x1911428, 41 slots;
// destructor 0x4FA110). Its methods are not reconstructed; only the queries
// the material component makes are declared. Each forwards to a slot of the
// graph's root node (RndShaderNode, sClassName 0x1A8C538), named here after
// the slot offset it calls.
class RndShaderGraph : public Component {
public:
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

    // The class symbol, "ShaderGraph", constructed by the static
    // initializer at 0x50ACFA.
    static Symbol sId;  // 0x1A8C1A8

    // The members from 22 up to the shader data; not modelled.
    unsigned char mMembers[226];
    // The graph's compiled shaders, which RndMaterialCom passes to
    // RndDevice::PrecacheMaterialShaders. Not decoded; the name is weak.
    void* mShaders;
};

static_assert(offsetof(RndShaderGraph, mShaders) == 248);
