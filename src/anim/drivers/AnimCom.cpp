// anim/AnimCom.o (0x326BE0 to 0x3276E7). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/drivers/AnimCom.h"

#include <new>

#include "anim/core/AnimOptions.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "entity/core/Entity.h"
#include "entity/core/InstanceCom.h"
#include "render/scene/RndDrawableEntityResource.h"

// The object's statics, in the order of its static initializer (0x327630),
// which first sets a shared header's null object id to -1 (0x19FB408).
Symbol AnimCom::sId("Animator");
Symbol AnimCom::sClassName("Animator");
PropRegistry AnimCom::sPropRegistry;
ComMetaData AnimCom::sMetaData;

// Reconstructed from eboot.elf at 0x326BE0.
AnimCom::AnimCom() {}

// Reconstructed from eboot.elf at 0x326C40.
void AnimCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    AnimCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    metadata.mDescription = "Com used to play animations in an entity";
    metadata.mAuthor = "James Fleming";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mAliases.push_back(Symbol("AnimCom"));
}

// Reconstructed from eboot.elf at 0x326EA0.
bool AnimCom::_OnResourcesLoaded() {
    return true;
}

// Reconstructed from eboot.elf at 0x326EB0.
void AnimCom::_Enter() {}

// Reconstructed from eboot.elf at 0x326EC0.
void AnimCom::_Poll() {}

// Inlined into slots 41 to 46.
AnimDriverCom* AnimCom::_GetDriver() const {
    const InstanceCom* const instance = mObject->GetBaseCom<InstanceCom>();
    if (instance == nullptr || instance->mRuntime.mEntity == nullptr) {
        return nullptr;
    }
    return _FindDriver(instance->mRuntime.mEntity);
}

// Reconstructed from eboot.elf at 0x326ED0.
AnimInstID AnimCom::StartAnim(const ResourcePath& path, const AnimOptions& options) {
    AnimDriverCom* const driver = _GetDriver();
    if (driver == nullptr) {
        return AnimInstID::NullID();
    }
    return driver->StartAnim(path, options);
}

// Reconstructed from eboot.elf at 0x326FB0.
AnimDriverCom* AnimCom::_FindDriver(Entity* entity) {
    return entity->GetRoot()->GetCom<AnimDriverCom>();
}

// Reconstructed from eboot.elf at 0x327010.
bool AnimCom::StopAnim(AnimInstID& id) {
    AnimDriverCom* const driver = _GetDriver();
    return driver != nullptr && driver->StopAnim(id);
}

// Reconstructed from eboot.elf at 0x3270E0.
void AnimCom::SetAnimWeight(const AnimInstID& id, float weight) {
    AnimDriverCom* const driver = _GetDriver();
    if (driver == nullptr) {
        return;
    }
    PlaybackInfo* const info = driver->GetPlaybackInfo(id);
    if (info != nullptr) {
        info->mWeight = weight;
    }
}

// Reconstructed from eboot.elf at 0x3271D0.
void AnimCom::SetAnimScale(const AnimInstID& id, float scale) {
    AnimDriverCom* const driver = _GetDriver();
    if (driver == nullptr) {
        return;
    }
    PlaybackInfo* const info = driver->GetPlaybackInfo(id);
    if (info != nullptr) {
        info->mScale = scale;
    }
}

// Reconstructed from eboot.elf at 0x3272C0.
void AnimCom::SetAnimLoop(const AnimInstID& id, bool loop) {
    AnimDriverCom* const driver = _GetDriver();
    if (driver == nullptr) {
        return;
    }
    PlaybackInfo* const info = driver->GetPlaybackInfo(id);
    if (info != nullptr) {
        info->mLooping = loop;
    }
}

// Reconstructed from eboot.elf at 0x3273A0.
void AnimCom::Clear() {
    AnimDriverCom* const driver = _GetDriver();
    if (driver != nullptr) {
        driver->Clear();
    }
}

// Reconstructed from eboot.elf at 0x327460.
Symbol AnimCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x327470.
Symbol AnimCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x327480.
int AnimCom::CurrentRev() const {
    return const_cast<AnimCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x3274A0.
bool AnimCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x3274D0.
Component* AnimCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x3274E0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* AnimCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<AnimCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) AnimCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x327600.
PropRegistry& AnimCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x327610.
ComMetaData& AnimCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x327620.
void AnimCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x2B1CD0. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* AnimCom::_Create() {
    return new AnimCom();
}

// Reconstructed from eboot.elf at 0x2A7330. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void AnimCom::Init() {
    InitComponentClass<AnimCom>();
}
