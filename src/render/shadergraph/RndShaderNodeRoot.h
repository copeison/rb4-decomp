#pragma once

#include "math/color/Color.h"
#include "render/shadergraph/RndShaderNode.h"

// The base of a shader graph's output nodes (render/RndShaderNodeRoot.o):
// the graph's root object holds one, and RndShaderGraph's queries forward
// to its slots. Only the vtable is modelled; RndShaderNodeRoot's own
// (0x191C8E0, 81 slots) carries the defaults noted below, which the root
// classes (ShaderNodeRootDecal, ShaderNodeRootDecalLit, ...) override. The
// map's older build has the Impl methods in the order IsLit, IsAlwaysFwdLit,
// IsEmissive, UsesAlphaCut, ModifiesVertexPosition, GetUsageHints,
// GetAllowedBlendModes, GetDefaultBlendMode, PrintRootPrelude and
// PrintRootCoda; this build reorders and adds to them.
class RndShaderNodeRoot : public RndShaderNode {
public:
    // Slot 61 at 0x567360: false.
    virtual bool _IsLitImpl() const;
    // Slot 62 at 0x567370: false. Whether the graph needs the forward lit
    // path at the quality level. The map's _IsAlwaysFwdLitImpl() takes no
    // parameter.
    virtual bool _IsAlwaysFwdLitImpl(int quality) const;
    // Slot 63 at 0x567380: false.
    virtual bool _UsesAlphaCutImpl() const;
    // Slots 64 and 65 at 0x567390, 0x5673A0: false. Names not in the
    // reference map.
    virtual bool _UsesSceneTexImpl() const;
    virtual bool _UsesSceneDepthImpl() const;
    // Slot 66 at 0x5673B0: false.
    virtual bool _IsEmissiveImpl() const;
    // Slot 67 at 0x5673C0: 0.
    virtual unsigned int _GetUsageHintsImpl() const;
    // Slot 68 at 0x5673D0: one bit per RndBlendMode; the default allows
    // only mode 5 (0x20).
    virtual unsigned int _GetAllowedBlendModesImpl() const;
    // Slot 69 at 0x5673E0: 5.
    virtual int _GetDefaultBlendModeImpl() const;
    // Slots 70 and 71 at 0x5673F0, 0x567400: false and
    // Hmx::Color::GetWhite(). Names not in the reference map.
    virtual bool _HasBlendFactorImpl() const;
    virtual Hmx::Color _GetBlendFactorImpl() const;
    // Slots 72-74 at 0x567460-0x567480: true. Names not in the reference
    // map; see the RndShaderGraph queries.
    virtual bool _AllowsBucketImpl() const;
    virtual bool _AllowsCullModeImpl() const;
    virtual bool _AllowsDepthPrepassImpl() const;
    // Slot 75 at 0x567490: true. The graph's shader source skips the
    // shading modes (0-18) this rejects (0x502410). Name not in the
    // reference map.
    virtual bool _SupportsShadingModeImpl(
        RndShadingMode mode,
        bool usesSceneTex) const;
    // Slot 76 at 0x5674A0: -1; ShaderNodeRootDecalLit returns 1 and
    // ShaderNodeRootDecalUnlit 0. Name not in the reference map; weak
    // evidence.
    virtual int _GetDecalLightingImpl() const;
    // Slots 77 and 78 at 0x566A70, 0x566BD0.
    virtual void _PrintRootPreludeImpl(
        RndShaderProgramType type,
        TextStream& stream,
        EntityPtr entity,
        const RndShaderGraph& graph);
    virtual void _PrintRootCodaImpl(
        RndShaderProgramType type,
        TextStream& stream,
        EntityPtr entity,
        const RndShaderGraph& graph,
        unsigned int flags);
    // Slots 79 and 80 at 0x5674B0, 0x5674C0, empty. The overrides found
    // (0x56D0E0, 0x56D130) print more shader source for one program type.
    // Names and parameters past the stream are not in the reference map;
    // weak evidence.
    virtual void _PrintRootHeaderImpl(
        RndShaderProgramType type,
        unsigned int flags,
        TextStream& stream);
    virtual void _PrintRootBodyImpl(
        RndShaderProgramType type,
        unsigned int flags,
        TextStream& stream,
        EntityPtr entity);
};
