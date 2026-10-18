// render/RndDrawNodeEditCom.o (0x6C9450 to 0x6C9B9A).
#include "render/scene/RndDrawNodeEditCom.h"

#include <new>

#include "entity/core/GameObject.h"
#include "entity/core/TransEntityResource.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/options/RndSelectionOptionsCom.h"
#include "render/scene/RndDrawNodeCom.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x6C9AD0; see RndMeshCom.cpp). Unused here.
// Names not in the reference map.
[[maybe_unused]] unsigned int gNullObjectId = 0xFFFFFFFFU;  // 0x1AB0838
[[maybe_unused]] int gMeshGroupSize2D = 8;                // 0x1AB083C
[[maybe_unused]] int gMeshGroupSize3D = 4;                // 0x1AB0840

}  // namespace

// The object's statics, in the order of its static initializer (0x6C9AD0).
Symbol RndDrawNodeEditCom::sId("DrawNodeEditorData");
Symbol RndDrawNodeEditCom::sClassName("DrawNodeEditorData");
PropRegistry RndDrawNodeEditCom::sPropRegistry;
ComMetaData RndDrawNodeEditCom::sMetaData;

// Reconstructed from eboot.elf at 0x6C9450.
RndDrawNodeEditCom::RndDrawNodeEditCom() {}

// Reconstructed from eboot.elf at 0x6C9480. The deleting destructor is at
// 0x6C9490.
RndDrawNodeEditCom::~RndDrawNodeEditCom() {}

// Reconstructed from eboot.elf at 0x6C94B0.
unsigned int RndDrawNodeEditCom::_GetCapabilityFlagsImpl() const {
    return 0x42;
}

// Reconstructed from eboot.elf at 0x6C94C0.
bool RndDrawNodeEditCom::_FilterShowingImpl(
    const RndCameraContext* camera,
    bool selected) const {
    static_cast<void>(camera);
    if (!selected) {
        return false;
    }
    const RndSelectionOptionsCom* options = theRndSelectionOpts;
    if (options->mRuntimeData.mSuppressed) {
        return false;
    }
    return options->mShowBoundingSpheres;
}

// Reconstructed from eboot.elf at 0x6C94F0.
void RndDrawNodeEditCom::_EditorDrawImpl(
    RndContext& context,
    bool selected,
    const VectorAdapter<unsigned long>& subEditables) {
    static_cast<void>(selected);
    static_cast<void>(subEditables);
    const Sphere& sphere = mObject->GetCom<RndDrawNodeCom>()->mRuntime.mWorldSphere;
    RndDrawUtl::MeshParams params;
    params.mColor = Hmx::Color(1.0F, 1.0F, 0.0F, 0.3F);
    params.mBlendMode = RndBlendMode::kSourceAlpha;
    params.mDepthMode = 3;
    params.mUnread = true;
    params.mKeepShader = false;
    params.mKeepState = false;
    RndDrawUtl::DrawSphereMesh(context, sphere, params);
}

// Reconstructed from eboot.elf at 0x6C95F0.
void RndDrawNodeEditCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndEditorDrawCom::_Init(registry, metadata);
    metadata.mDescription = "RndEditorDrawCom companion to RndDrawNodeCom";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(TransEntityResource::Id());
    metadata.mDefaultFor.push_back(RndDrawNodeCom::sClassName);
    metadata.mRequiredComponents.push_back(RndDrawNodeCom::sClassName);
}

// Reconstructed from eboot.elf at 0x6C98F0.
Symbol RndDrawNodeEditCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6C9900.
Symbol RndDrawNodeEditCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6C9910.
int RndDrawNodeEditCom::CurrentRev() const {
    return const_cast<RndDrawNodeEditCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6C9930.
bool RndDrawNodeEditCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6C9960.
Component* RndDrawNodeEditCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6C9970. The copy goes at the buffer's
// next 8-byte boundary and starts a fresh RuntimeData.
char* RndDrawNodeEditCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndDrawNodeEditCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndDrawNodeEditCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6C9AB0.
PropRegistry& RndDrawNodeEditCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6C9AC0.
ComMetaData& RndDrawNodeEditCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x406050. The binary emits the factory
// with the other render factories.
Component* RndDrawNodeEditCom::_Create() {
    return new RndDrawNodeEditCom();
}
