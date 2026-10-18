// render/RndEditorDrawCom.o (0x461330 to 0x4616BE).
#include "render/editor/RndEditorDrawCom.h"

#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "render/editor/RndEditorDrawSetCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/scene/RndDrawNodeCom.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x4616A0; see RndMeshCom.cpp). Unused here.
// Names not in the reference map.
[[maybe_unused]] unsigned int gNullObjectId = 0xFFFFFFFFU;  // 0x1A86010
[[maybe_unused]] int gMeshGroupSize2D = 8;                // 0x1A86014
[[maybe_unused]] int gMeshGroupSize3D = 4;                // 0x1A86018

}  // namespace

// Reconstructed from eboot.elf at 0x461330.
RndEditorDrawCom::RndEditorDrawCom() {}

// Reconstructed from eboot.elf at 0x461370.
RndEditorDrawCom::RuntimeData::RuntimeData() : mCapabilityFlags(0) {}

// Reconstructed from eboot.elf at 0x461390. The deleting destructor is at
// 0x4613C0.
RndEditorDrawCom::~RndEditorDrawCom() {}

// Reconstructed from eboot.elf at 0x461400.
bool RndEditorDrawCom::GetWorldSphere(const RndCameraContext* camera, Sphere& sphere) const {
    bool found = false;
    if ((mRuntimeData.mCapabilityFlags & 0x20) != 0) {
        found = _GetWorldSphereImpl(camera, sphere);
    }
    if ((mRuntimeData.mCapabilityFlags & 0x40) == 0) {
        return found;
    }
    const Sphere& nodeSphere =
        mObject->GetCom<RndDrawNodeCom>()->mRuntime.mWorldSphere;
    if (found) {
        sphere.GrowToContain(nodeSphere);
    } else {
        sphere = nodeSphere;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x4614C0.
void RndEditorDrawCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    static_cast<void>(registry);
    metadata.mCategory = ComMetaData::kCategoryDebug;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mRequiredComponents.push_back(RndEditorDrawSetCom::sClassName);
    metadata.mEditorRestrictions = 0;
    metadata.mLightweight = true;
}

// Reconstructed from eboot.elf at 0x461670.
void RndEditorDrawCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
}

// Reconstructed from eboot.elf at 0x461680.
bool RndEditorDrawCom::_OnResourcesLoaded() {
    return true;
}

// Reconstructed from eboot.elf at 0x461690.
void RndEditorDrawCom::_EditorDrawImpl(
    RndContext& context,
    bool selected,
    const VectorAdapter<unsigned long>& subEditables) {
    static_cast<void>(context);
    static_cast<void>(selected);
    static_cast<void>(subEditables);
}
