// render/RndPostProcStageCom.o (0x631930 to 0x631EEF).
#include "render/postprocessing/chain/RndPostProcStageCom.h"

#include <new>

#include "entity/props/PropMetadata.h"
#include "os/memory/MemMgr.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/scene/RndSceneResource.h"

// The object's statics, in the order of its static initializer (0x631E20).
Symbol RndPostProcStageCom::sId("PostProcStage");
Symbol RndPostProcStageCom::sClassName("PostProcStage");
PropRegistry RndPostProcStageCom::sPropRegistry;
ComMetaData RndPostProcStageCom::sMetaData;

// Reconstructed from eboot.elf at 0x631930.
RndPostProcStageCom::RndPostProcStageCom() : mEnabled(true) {}

// Reconstructed from eboot.elf at 0x631960. The deleting destructor is at
// 0x631970.
RndPostProcStageCom::~RndPostProcStageCom() {}

// Reconstructed from eboot.elf at 0x631990. A stage of a scene's post
// processing implements the "PostProcStage" interface.
void RndPostProcStageCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Com that holds a single stage of a scene's PostProc.";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    metadata.mInterface = sClassName;
    metadata.mEditorRestrictions = 0;
    RndPostProcStageCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    BoolMetadata& enabled = TypeSpecificMetadata(
        registry.RegisterProp("enabled", offsetof(RndPostProcStageCom, mEnabled), kPropertyBool, 1),
        prototype.mEnabled);
    enabled.mDefault = prototype.mEnabled;
    static_cast<void>(Symbol("prop"));
    enabled.mHelp = String("Should this stage draw?");
}

// Reconstructed from eboot.elf at 0x631BD0.
void RndPostProcStageCom::Draw(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) {
    if (mEnabled) {
        _DrawImpl(context, camera, params, batch);
    }
}

// Reconstructed from eboot.elf at 0x631BF0.
void RndPostProcStageCom::UpdateDrawTarget(RndSceneDrawTarget& target) {
    if (mEnabled) {
        _UpdateDrawTarget(target);
    }
}

// Reconstructed from eboot.elf at 0x631C10.
void RndPostProcStageCom::_EditPoll() {}

// Reconstructed from eboot.elf at 0x631C20.
bool RndPostProcStageCom::_OnResourcesLoaded() {
    return true;
}

// Reconstructed from eboot.elf at 0x631C30.
Symbol RndPostProcStageCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x631C40.
Symbol RndPostProcStageCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x631C50.
int RndPostProcStageCom::CurrentRev() const {
    return const_cast<RndPostProcStageCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x631C70.
bool RndPostProcStageCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x631CA0.
Component* RndPostProcStageCom::AsComponent() {
    return this;
}

/// Reconstructed from eboot.elf at 0x631CB0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndPostProcStageCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndPostProcStageCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndPostProcStageCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x631DE0.
PropRegistry& RndPostProcStageCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x631DF0.
ComMetaData& RndPostProcStageCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x631E00.
void RndPostProcStageCom::_DrawImpl(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) {
    static_cast<void>(context);
    static_cast<void>(camera);
    static_cast<void>(params);
    static_cast<void>(batch);
}

// Reconstructed from eboot.elf at 0x631E10.
void RndPostProcStageCom::_UpdateDrawTarget(RndSceneDrawTarget& target) {
    static_cast<void>(target);
}

// Reconstructed from eboot.elf at 0x4047F0. The binary emits the factory
// with the class's Init (0x3F1440); the placement factory at 0x404820
// constructs in given storage.
Component* RndPostProcStageCom::_Create() {
    return new RndPostProcStageCom();
}

// Reconstructed from eboot.elf at 0x3F1440. The binary emits the registration
// in render/RndInit.o. The class is a base that is not created itself.
void RndPostProcStageCom::Init() {
    InitComponentClass<RndPostProcStageCom>(false, true);
}

// Reconstructed from eboot.elf at 0x6304F0. The binary emits it with the
// first stage class that calls it, in render/RndPostProcStageBloomCom.o.
void RndPostProcStageCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("RndPostProcStageCom", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}
