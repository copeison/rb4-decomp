// render/RndPostProcCom.o (0x62EF10 to 0x62F9FF).
#include "render/postprocessing/chain/RndPostProcCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/postprocessing/chain/RndPostProcStageBloomCom.h"
#include "render/postprocessing/chain/RndPostProcStageCom.h"

namespace {

// The id of no object. The binary reads it from a shared header's global
// (0x1AAA798), which the object's static initializer sets to -1. Name not
// in the reference map.
constexpr unsigned int kNoObject = 0xFFFFFFFFU;

// The object of a stage id, or null for an unset id or a missing object.
// Inlined into the stage loops. Name not in the reference map.
const GameObject* FindStageObject(const Component* chain, GameObjectId id) {
    if (id.mId == kNoObject) {
        return nullptr;
    }
    return chain->mObject->mEntity->GetObject(id);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x62F930).
Symbol RndPostProcCom::sId("PProc");
Symbol RndPostProcCom::sClassName("PProc");
PropRegistry RndPostProcCom::sPropRegistry;
ComMetaData RndPostProcCom::sMetaData;

// Reconstructed from eboot.elf at 0x62EF10.
RndPostProcCom::RndPostProcCom() {}

// Reconstructed from eboot.elf at 0x62EF70. The deleting destructor is at
// 0x62EFE0.
RndPostProcCom::~RndPostProcCom() {}

// Reconstructed from eboot.elf at 0x62F060. A stage object must hold a
// stage component: the binary draws through the lookup's result unchecked.
void RndPostProcCom::Draw(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) const {
    for (unsigned long index = 0; index < mStages.size(); ++index) {
        const GameObject* object = FindStageObject(this, mStages[index]);
        if (object != nullptr) {
            object->GetBaseCom<RndPostProcStageCom>()->Draw(context, camera, params, batch);
        }
    }
}

// Reconstructed from eboot.elf at 0x62F170.
void RndPostProcCom::UpdateDrawTarget(RndSceneDrawTarget& target) const {
    for (unsigned long index = 0; index < mStages.size(); ++index) {
        const GameObject* object = FindStageObject(this, mStages[index]);
        if (object != nullptr) {
            object->GetBaseCom<RndPostProcStageCom>()->UpdateDrawTarget(target);
        }
    }
}

// Reconstructed from eboot.elf at 0x62F640.
void RndPostProcCom::_EditPoll() {}

// Reconstructed from eboot.elf at 0x62F650.
bool RndPostProcCom::_OnResourcesLoaded() {
    return true;
}

// Reconstructed from eboot.elf at 0x62F660. The stage component is searched
// for without a bound.
bool RndPostProcCom::HasBloomStage() const {
    for (const GameObjectId& id : mStages) {
        const GameObject* object = FindStageObject(this, id);
        if (object == nullptr) {
            continue;
        }
        const RndPostProcStageCom* stage = object->GetExistingBaseCom<RndPostProcStageCom>();
        if (stage->GetId() == RndPostProcStageBloomCom::sId) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x62F730.
Symbol RndPostProcCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x62F740.
Symbol RndPostProcCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x62F750.
int RndPostProcCom::CurrentRev() const {
    return const_cast<RndPostProcCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x62F770.
bool RndPostProcCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x62F7A0.
Component* RndPostProcCom::AsComponent() {
    return this;
}

/// Reconstructed from eboot.elf at 0x62F7B0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndPostProcCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndPostProcCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndPostProcCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x62F910.
PropRegistry& RndPostProcCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x62F920.
ComMetaData& RndPostProcCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x404840. The binary emits the factory
// with the class's Init (0x3F17F0); the placement factory at 0x404870
// constructs in given storage.
Component* RndPostProcCom::_Create() {
    return new RndPostProcCom();
}
