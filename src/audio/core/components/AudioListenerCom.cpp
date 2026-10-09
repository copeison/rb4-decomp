// audio/AudioListenerCom.o (0x392E0 to 0x39F5F). The object also emits
// Component::_Exit's empty body at 0x39E30.
#include "audio/core/components/AudioListenerCom.h"

#include "audio/core/system/SoundManager.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/vector/Vector3.h"
#include "utl/data/DataArray.h"

// The object's statics, in the order of its static initializer (0x39E40).
// The int it first sets to -1 (0x19C7C78) comes from a shared header and is
// not modelled.
Symbol AudioListenerCom::sId("AudioListener");
Symbol AudioListenerCom::sClassName("AudioListener");
PropRegistry AudioListenerCom::sPropRegistry;
ComMetaData AudioListenerCom::sMetaData;
CritSec AudioListenerCom::sActiveListenerLock;
AudioListenerCom* AudioListenerCom::sActiveListener;

// Reconstructed from eboot.elf at 0x392E0. A linked subscription leaves its
// source before the link unlinks itself.
AudioListenerCom::~AudioListenerCom() {
    if (sActiveListener == this) {
        sActiveListener = nullptr;
    }
    LinkedList::Node& link = mEntityActiveSink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mEntityActiveSink.mSource->RemoveSink(&mEntityActiveSink);
    }
}

// Reconstructed from eboot.elf at 0x39360.
void AudioListenerCom::MakeInactive() {
    if (sActiveListener == this) {
        sActiveListener = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x39400.
void AudioListenerCom::_EditEnter() {
    _Enter();
}

// Reconstructed from eboot.elf at 0x39410.
void AudioListenerCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x39750.
void AudioListenerCom::_Enter() {
    if (sActiveListener == nullptr) {
        sActiveListener = this;
    }
    Entity* scene = FindSceneEntity(mObject->mEntity);
    if (scene == nullptr) {
        return;
    }
    static Symbol event;
    if (event == Symbol()) {
        event = Symbol("entity_active_changed");
    }
    static Symbol handler;
    if (handler == Symbol()) {
        handler = Symbol("on_entity_active_changed");
    }
    // Component begins with MsgSink's slots; the model does not derive it.
    scene->AddSink(&mEntityActiveSink, reinterpret_cast<MsgSink*>(this), event, handler);
}

// Reconstructed from eboot.elf at 0x39890.
void AudioListenerCom::MakeActiveIfNone() {
    if (sActiveListener == nullptr) {
        sActiveListener = this;
    }
}

// Reconstructed from eboot.elf at 0x398B0. The parent entity is read
// through the out-of-line accessor at 0xF0770.
Entity* FindSceneEntity(Entity* entity) {
    EntityResource* resource = entity->mResource;
    if (resource == nullptr) {
        return nullptr;
    }
    static Symbol sceneType;
    if (sceneType == Symbol()) {
        sceneType = Symbol("RndSceneResource");
    }
    if (resource->IsA(sceneType)) {
        return entity;
    }
    if (entity->mParentObject == nullptr) {
        return nullptr;
    }
    return FindSceneEntity(entity->mParentObject->mEntity);
}

// Reconstructed from eboot.elf at 0x399B0.
void AudioListenerCom::_Poll() {
    ScopedCritSec lock(sActiveListenerLock);
    if (sActiveListener == nullptr) {
        sActiveListener = this;
    } else if (sActiveListener != this) {
        return;
    }
    const Transform xfm = mObject->GetCom<TransCom>()->mWorldXfm;
    theSoundManager.UpdateActiveListener(xfm, Vector3::sZero);
}

// Reconstructed from eboot.elf at 0x39AB0.
void AudioListenerCom::MakeActive() {
    sActiveListener = this;
}

// Reconstructed from eboot.elf at 0x39AC0.
void AudioListenerCom::SetHardwareMapId(int id) {
    mHardwareMapId = id;
}

// Reconstructed from eboot.elf at 0x39AD0.
DataNode AudioListenerCom::_OnEntityActiveChanged(bool active) {
    if (active) {
        if (sActiveListener == nullptr) {
            sActiveListener = this;
        }
    } else if (sActiveListener == this) {
        sActiveListener = nullptr;
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x39B10.
DataNode AudioListenerCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol onEntityActiveChanged;
    if (onEntityActiveChanged == Symbol()) {
        onEntityActiveChanged = Symbol("on_entity_active_changed");
    }
    if (type != onEntityActiveChanged) {
        return DataNode();
    }
    return _OnEntityActiveChanged(msg->Int(2) != 0);
}

// Reconstructed from eboot.elf at 0x39C30.
Symbol AudioListenerCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x39C40.
Symbol AudioListenerCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x39C50.
int AudioListenerCom::CurrentRev() const {
    return const_cast<AudioListenerCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x39C70.
bool AudioListenerCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x39CA0.
Component* AudioListenerCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x39E10.
PropRegistry& AudioListenerCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x39E20.
ComMetaData& AudioListenerCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xD280. The binary emits the factory in
// audio/SoundManager.o with the class's Init.
Component* AudioListenerCom::_Create() {
    return new AudioListenerCom();
}
