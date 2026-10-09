// render/RndPostProcStageCom.o (0x631930 to 0x631EEF).
#include "render/postprocessing/chain/RndPostProcStageCom.h"

#include <new>

#include "render/drawing/RndSceneDrawParams.h"
#include "render/drawing/RndSceneDrawer.h"

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
