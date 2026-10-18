// render/RndShaderGraph.o: the queries the material component makes. The
// rest of the object (0x4F9D50 onwards) is not reconstructed.
#include "render/shadergraph/RndShaderGraph.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "render/shadergraph/RndShaderNodeRoot.h"

namespace {

// The blend-mode bit of mode 5, which the forward lit test checks. Name not
// in the reference map.
constexpr unsigned int kBlendModeBit5 = 1U << 5;

}  // namespace

// Constructed by the object's static initializer (0x50ACFA); the
// object's other statics are not modelled.
Symbol RndShaderGraph::sId("ShaderGraph");

inline RndShaderNodeRoot* RndShaderGraph::GetRootNode() const {
    GameObject* const root = mObject->mEntity->GetObject(mRootNode);
    return static_cast<RndShaderNodeRoot*>(
        root->GetExistingBaseCom<RndShaderNode>());
}

// Reconstructed from eboot.elf at 0x4FD570.
bool RndShaderGraph::IsLit() const {
    return GetRootNode()->_IsLitImpl();
}

// Reconstructed from eboot.elf at 0x4FD750.
unsigned int RndShaderGraph::GetAllowedBlendModes() const {
    return GetRootNode()->_GetAllowedBlendModesImpl();
}

// Reconstructed from eboot.elf at 0x4FD7C0. A graph that reads the scene
// texture at level 1 or the linear depth always draws forward; otherwise
// it does when the root asks for it or cannot blend with mode 5.
bool RndShaderGraph::IsFwdLit(int quality) const {
    if (!GetRootNode()->_IsLitImpl() || GetRootNode()->_IsEmissiveImpl()) {
        return false;
    }
    if (mSceneTexUsage != 1 && !mUsesLinearDepth
        && !GetRootNode()->_IsAlwaysFwdLitImpl(quality)
        && (GetRootNode()->_GetAllowedBlendModesImpl() & kBlendModeBit5)
            != 0) {
        return false;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x4FD990.
bool RndShaderGraph::IsEmissive() const {
    return GetRootNode()->_IsEmissiveImpl();
}

// Reconstructed from eboot.elf at 0x4FDAB0.
bool RndShaderGraph::UsesAlphaCut() const {
    return GetRootNode()->_UsesAlphaCutImpl();
}

// Reconstructed from eboot.elf at 0x4FDB20.
bool RndShaderGraph::UsesSceneTex() const {
    return GetRootNode()->_UsesSceneTexImpl();
}

// Reconstructed from eboot.elf at 0x4FDB90.
bool RndShaderGraph::UsesSceneDepth() const {
    return GetRootNode()->_UsesSceneDepthImpl();
}

// Reconstructed from eboot.elf at 0x4FDC00.
unsigned int RndShaderGraph::GetUsageHints() const {
    return GetRootNode()->_GetUsageHintsImpl();
}

// Reconstructed from eboot.elf at 0x4FDC70.
int RndShaderGraph::GetDefaultBlendMode() const {
    return GetRootNode()->_GetDefaultBlendModeImpl();
}

// Reconstructed from eboot.elf at 0x4FDCE0.
bool RndShaderGraph::HasBlendFactor() const {
    return GetRootNode()->_HasBlendFactorImpl();
}

// Reconstructed from eboot.elf at 0x4FDD50.
Hmx::Color RndShaderGraph::GetBlendFactor() const {
    return GetRootNode()->_GetBlendFactorImpl();
}

// Reconstructed from eboot.elf at 0x4FDDC0.
bool RndShaderGraph::AllowsBucket() const {
    return GetRootNode()->_AllowsBucketImpl();
}

// Reconstructed from eboot.elf at 0x4FDE30.
bool RndShaderGraph::AllowsCullMode() const {
    return GetRootNode()->_AllowsCullModeImpl();
}

// Reconstructed from eboot.elf at 0x4FDEA0.
bool RndShaderGraph::AllowsDepthPrepass() const {
    return GetRootNode()->_AllowsDepthPrepassImpl();
}
