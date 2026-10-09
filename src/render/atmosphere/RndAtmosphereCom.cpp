// render/RndAtmosphereCom.o (0x451450 to 0x451C6F). The object also emits
// the registry's property callbacks (0x451B60, 0x451B80). The tiny
// initializer before it (0x451430) sets the shared header's ints for the
// previous object and is not modelled.
#include "render/atmosphere/RndAtmosphereCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "render/scene/RndSceneCom.h"

// The object's statics, in the order of its static initializer (0x451BA0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndAtmosphereCom::sId("Atmosphere");
Symbol RndAtmosphereCom::sClassName("Atmosphere");
PropRegistry RndAtmosphereCom::sPropRegistry;
ComMetaData RndAtmosphereCom::sMetaData;

// Reconstructed from eboot.elf at 0x451450. The fog ends a kilometer out.
RndAtmosphereCom::RndAtmosphereCom()
    : mEnabled(true), mStartDist(0.0F), mEndDist(gUnitsPerMeter * 1000.0F) {}

// Reconstructed from eboot.elf at 0x4514A0. The deleting destructor is at
// 0x4514B0.
RndAtmosphereCom::~RndAtmosphereCom() {}

// Reconstructed from eboot.elf at 0x4518C0.
void RndAtmosphereCom::SetStartDist(float dist) {
    mStartDist = dist;
    mEndDist = dist > mEndDist ? dist : mEndDist;
}

// Reconstructed from eboot.elf at 0x4518D0.
void RndAtmosphereCom::_SyncStartDist() {
    mEndDist = mStartDist > mEndDist ? mStartDist : mEndDist;
}

// Reconstructed from eboot.elf at 0x4518E0.
void RndAtmosphereCom::SetEndDist(float dist) {
    mEndDist = dist;
    mStartDist = dist < mStartDist ? dist : mStartDist;
}

// Reconstructed from eboot.elf at 0x4518F0.
void RndAtmosphereCom::_SyncEndDist() {
    mStartDist = mEndDist < mStartDist ? mEndDist : mStartDist;
}

// Reconstructed from eboot.elf at 0x451900.
void RndAtmosphereCom::_PostCreate() {
    RndSceneCom* scene =
        mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    if (scene != nullptr && scene->GetAtmosphere() == nullptr) {
        scene->SetAtmosphere(this);
    }
}

// Reconstructed from eboot.elf at 0x451990.
Symbol RndAtmosphereCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4519A0.
Symbol RndAtmosphereCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4519B0.
int RndAtmosphereCom::CurrentRev() const {
    return const_cast<RndAtmosphereCom*>(this)
        ->_GetPropRegistry()
        .mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4519D0.
bool RndAtmosphereCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x451A00.
Component* RndAtmosphereCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x451A10. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndAtmosphereCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndAtmosphereCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndAtmosphereCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x451B40.
PropRegistry& RndAtmosphereCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x451B50.
ComMetaData& RndAtmosphereCom::_GetMetaData() {
    return sMetaData;
}
