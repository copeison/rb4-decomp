// anim/PropAnimResource.o (0x2FD210 to 0x2FFD5F). The object also emits
// AnimResource's default slots 3, 9 and 10 (0x2FFC00-0x2FFC20) and, at its
// end, out-of-line copies of PropKeysBoolCom's StaticKeysType and
// StaticKeysSubType (0x2FFCB0, 0x2FFCC0). Not reconstructed:
// _GetSongBeatSource (0x2FE040), which reaches into the song classes, and
// an initializer of a header's symbol at 0x19F9938 (0x2FFC40).
#include "anim/props/PropAnimResource.h"

#include <float.h>

#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "anim/props/PropAnimCom.h"
#include "anim/props/PropKeysCom.h"
#include "anim/props/PropKeysQuatCom.h"
#include "anim/props/PropKeysRotationCom.h"
#include "audio/core/music/BeatSource.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/resources/ResourceMetaData.h"
#include "entity/waveform/RotationKey.h"
#include "rb_audio/song/RBSongResource.h"
#include "utl/data/DataNode.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"

// The object's statics, in the order of its static initializer (0x2FFC70).
ResourceMetaData PropAnimResource::sMetaData;

namespace {

// PropAnimCom's default "time_units": the timeline of the entity the
// animation is inlined in. Name not in the reference map.
constexpr int kDefaultTimeUnits = 8;

// The invalid object id, which the binary reads from the object's copy of a
// shared header's static (0x19F9948). Name not in the reference map.
constexpr unsigned int kInvalidObjectId = 0xFFFFFFFFU;

// The key component of the object: its component of the "PropKeys"
// interface, or null. Inlined into each search. Name not in the reference
// map.
PropKeysBaseCom* FindKeysCom(const GameObject* object) {
    const Symbol keysInterface("PropKeys");
    if (keysInterface == Symbol()) {
        return nullptr;
    }
    for (const GameObject::ComIndex& index : object->mComs) {
        if (index.mBaseId == keysInterface) {
            return static_cast<PropKeysBaseCom*>(index.mCom);
        }
    }
    return nullptr;
}

// The key component of an object that must have one: the table is searched
// without a bound. Inlined into ApplyFrame and OffsetKeys. Name not in the
// reference map.
PropKeysBaseCom* GetExistingKeysCom(const GameObject* object) {
    const Symbol keysInterface("PropKeys");
    const GameObject::ComIndex* index = object->mComs.data();
    while (index->mBaseId != keysInterface) {
        ++index;
    }
    return static_cast<PropKeysBaseCom*>(index->mCom);
}

}  // namespace

// Reconstructed from eboot.elf at 0x2FD220. The complete destructor at
// 0x2FD210 is a jump to EntityResource's.
PropAnimResource::~PropAnimResource() {}

// Reconstructed from eboot.elf at 0x2FD240.
void PropAnimResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("propanim"));
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x2FD340.
void PropAnimResource::Save(BinStream& stream, bool cached) {
    EntityResource::Save(stream, cached);
    const int rev = kRev;
    stream.WriteEndian(&rev, 4);
}

// Reconstructed from eboot.elf at 0x2FD3A0. Each quaternion keys' property
// is compared with "rotation" (local static at 0x19F99A8) and the result is
// not used: only the transforms' rotations had quaternion keys.
bool PropAnimResource::_LoadEntity(BinStream& stream, bool cached) {
    if (!EntityResource::_LoadEntity(stream, cached)) {
        return false;
    }
    if (gEntityRev >= 33) {
        stream.ReadEndian(&mRev, 4);
    }
    if (mRev > 1) {
        return true;
    }
    Entity* const entity = mEntity;
    for (GameObject* object = entity->NextObject(nullptr, PropKeysQuatCom::sId); object != nullptr;
         object = entity->NextObject(object, PropKeysQuatCom::sId)) {
        const PropKeysQuatCom* const quatKeys = object->GetCom<PropKeysQuatCom>();
        const PropRef prop = quatKeys->mProp;
        static Symbol rotation;
        if (rotation == Symbol()) {
            rotation = Symbol("rotation");
        }
        static_cast<void>(prop.mPath == PropPath(rotation));
        const TKeyTrack<Hmx::Quat> track = *quatKeys;
        object->DestroyComponent(PropKeysQuatCom::sId, true);
        PropKeysBaseCom* keys = nullptr;
        PropKeysBaseCom::NewKeysType(entity, kPropertyEuler, Symbol(), prop, &keys, object);
        TKeyTrack<RotationKey>& rotationKeys = *static_cast<PropKeysRotationCom*>(keys);
        const TKey<Hmx::Quat>* const oldKeys = static_cast<const TKey<Hmx::Quat>*>(track.mKeys.mData);
        for (unsigned long i = 0; i < track.mKeys.mSize; ++i) {
            rotationKeys.AddKey(RotationKey(oldKeys[i].mValue), oldKeys[i].mFrame, false, false);
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2FD860. The interface's thunk at
// 0x2FD880 repeats the body.
GameObject* PropAnimResource::GetObject(Entity* entity) const {
    static_cast<void>(entity);
    return mEntity->GetRoot();
}

// Reconstructed from eboot.elf at 0x2FD8A0.
bool PropAnimResource::IsLooping() const {
    const PropAnimCom* const com = GetPropAnimCom();
    return com != nullptr && com->mLooping;
}

// Reconstructed from eboot.elf at 0x2FD8C0.
PropAnimCom* PropAnimResource::GetPropAnimCom() const {
    if (mEntity == nullptr) {
        static_cast<void>(MakeErrorName());
        return nullptr;
    }
    return mEntity->GetRoot()->GetCom<PropAnimCom>();
}

// Reconstructed from eboot.elf at 0x2FD9E0.
void PropAnimResource::SetLooping(bool looping) {
    PropAnimCom* const com = GetPropAnimCom();
    if (com != nullptr) {
        com->mLooping = looping;
    }
}

// Reconstructed from eboot.elf at 0x2FDA30.
float PropAnimResource::GetStartFrame() const {
    const PropAnimCom* const com = GetPropAnimCom();
    if (com == nullptr) {
        return 0.0f;
    }
    return com->mFrameRangeStart == FLT_MAX ? 0.0f : com->mFrameRangeStart;
}

// Reconstructed from eboot.elf at 0x2FDA90.
float PropAnimResource::GetEndFrame() const {
    const PropAnimCom* const com = GetPropAnimCom();
    if (com == nullptr) {
        return 0.0f;
    }
    return com->mFrameRangeEnd == -FLT_MAX ? 0.0f : com->mFrameRangeEnd;
}

// Reconstructed from eboot.elf at 0x2FDAF0.
TimeUnits PropAnimResource::GetUnits() const {
    return static_cast<TimeUnits>(AnimDriver::GetBaseUnits(_GetRawUnits(false)));
}

// Reconstructed from eboot.elf at 0x2FDBB0. The PropAnimCom must exist.
int PropAnimResource::_GetRawUnits(bool keepDefault) const {
    int units = mEntity->GetRoot()->GetExistingCom<PropAnimCom>()->mTimeUnits;
    if (units == kDefaultTimeUnits && !keepDefault) {
        const AnimDriverCom* const driver = mInlineOwner->mEntity->GetRoot()->GetCom<AnimDriverCom>();
        if (driver != nullptr) {
            units = driver->mDefaultTimeline;
        }
    }
    return units;
}

// Reconstructed from eboot.elf at 0x2FDD20. The PropAnimCom must exist.
void PropAnimResource::SetUnits(int units) {
    mEntity->GetRoot()->GetExistingCom<PropAnimCom>()->mTimeUnits = units;
}

// Reconstructed from eboot.elf at 0x2FDD70.
float PropAnimResource::GetTimeFromFrame(float frame, TimeUnits units) const {
    const int rawUnits = _GetRawUnits(false);
    const TimeUnits ownUnits = static_cast<TimeUnits>(AnimDriver::GetBaseUnits(rawUnits));
    float time = frame / AnimDriver::GetDefaultFramesPerUnit(static_cast<TimeUnits>(rawUnits));
    if (units == kNumTimeUnits) {
        units = ownUnits;
    }
    if ((ownUnits == kTimeUnitsBeats) != (units == kTimeUnitsBeats) && mInlineOwner != nullptr &&
        mInlineOwner->IsA(RBSongResource::Id())) {
        const BeatSource* const beats = _GetSongBeatSource();
        if (units == kTimeUnitsBeats) {
            time = beats->TickToBeat(beats->MsToTick(time * 1000.0f));
        } else {
            time = beats->TickToMs(beats->BeatToTick(time)) * 0.001f;
        }
    }
    return time;
}

// Reconstructed from eboot.elf at 0x2FDF80.
bool PropAnimResource::_IsInSong() const {
    return mInlineOwner != nullptr && mInlineOwner->IsA(RBSongResource::Id());
}

// Reconstructed from eboot.elf at 0x2FE140.
float PropAnimResource::GetFrameFromTime(float time, TimeUnits units) const {
    const int rawUnits = _GetRawUnits(false);
    const TimeUnits ownUnits = static_cast<TimeUnits>(AnimDriver::GetBaseUnits(rawUnits));
    if (units == kNumTimeUnits) {
        units = ownUnits;
    }
    if ((units == kTimeUnitsBeats) != (ownUnits == kTimeUnitsBeats) && mInlineOwner != nullptr &&
        mInlineOwner->IsA(RBSongResource::Id())) {
        const BeatSource* const beats = _GetSongBeatSource();
        if (units == kTimeUnitsBeats) {
            time = beats->TickToMs(beats->BeatToTick(time)) * 0.001f;
        } else {
            time = beats->TickToBeat(beats->MsToTick(time * 1000.0f));
        }
    }
    return time * AnimDriver::GetDefaultFramesPerUnit(static_cast<TimeUnits>(rawUnits));
}

// Reconstructed from eboot.elf at 0x2FE380.
const char* PropAnimResource::MakeErrorName() const {
    if (mInlineOwner != nullptr) {
        return (FormatString("%s:%s") << mInlineOwner->mPath.mPath << mPath.mPath).Str();
    }
    return mPath.mPath.Str();
}

// Reconstructed from eboot.elf at 0x2FE420. The interface's thunk at
// 0x2FE430 repeats the body.
const PropArray<AnimResource::Event>* PropAnimResource::GetEvents(Entity* entity) const {
    static_cast<void>(entity);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x2FE440.
float PropAnimResource::GetEventFrameOffset() const {
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2FE460.
void PropAnimResource::UpdateFrameRange() {
    PropAnimCom* const com = GetPropAnimCom();
    if (com != nullptr) {
        com->UpdateFrameRange();
    }
}

// Reconstructed from eboot.elf at 0x2FE480.
void PropAnimResource::SetTargetOrder(const eastl::vector<GameObjectId>& order) {
    PropAnimCom* const com = GetPropAnimCom();
    if (com != nullptr) {
        com->SetTargetOrder(order);
    }
}

// Reconstructed from eboot.elf at 0x2FE4B0.
void PropAnimResource::GetTargetOrder(eastl::vector<GameObjectId>& order) const {
    const PropAnimCom* const com = GetPropAnimCom();
    if (com != nullptr) {
        com->GetTargetOrder(order);
        return;
    }
    order.clear();
}

// Reconstructed from eboot.elf at 0x2FE4F0.
PropKeysBaseCom* PropAnimResource::AddKeys(PropertyType type, const PropRef& prop) {
    return _AddKeys(type, Symbol(), prop);
}

// Reconstructed from eboot.elf at 0x2FE510. The overlap's error message,
// with both properties' descriptions, is made and dropped.
PropKeysBaseCom* PropAnimResource::_AddKeys(PropertyType type, Symbol subType, const PropRef& prop) {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return nullptr;
    }
    GameObjectId keysObject = {kInvalidObjectId};
    PropKeysBaseCom* const existing = FindKeys(prop, keysObject);
    if (existing != nullptr) {
        return existing;
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr; object = entity->NextObject(object, Symbol())) {
        if (object == entity->GetRoot()) {
            continue;
        }
        const PropKeysBaseCom* const keys = FindKeysCom(object);
        if (keys->mProp.mObject.mId != prop.mObject.mId || keys->mProp.mCom != prop.mCom) {
            continue;
        }
        if (prop.mPath.Contains(keys->mProp.mPath) || keys->mProp.mPath.Contains(prop.mPath)) {
            static_cast<void>(MakeErrorName());
            static_cast<void>(prop.Description(entity));
            static_cast<void>(keys->mProp.Description(entity));
            return nullptr;
        }
    }
    PropKeysBaseCom* keys = nullptr;
    PropKeysBaseCom::NewKeysType(entity, type, subType, prop, &keys, nullptr);
    return keys;
}

// Reconstructed from eboot.elf at 0x2FE7F0.
eastl::vector<PropKeysBaseCom*> PropAnimResource::AddKeys(const ResourcePtr<PropAnimResource>& source) {
    eastl::vector<PropKeysBaseCom*> sourceKeys;
    source.Get()->HarvestAllKeys(sourceKeys, true);
    eastl::vector<PropKeysBaseCom*> added;
    added.reserve(sourceKeys.size());
    for (PropKeysBaseCom* const keys : sourceKeys) {
        PropKeysBaseCom* const newKeys = _AddKeys(keys->GetKeysType(), Symbol(), keys->mProp);
        added.push_back(newKeys);
        DataNode value;
        keys->GetValueAtFrame(0.0f, value);
        newKeys->SetValueAtFrame(0.0f, value, false);
    }
    return added;
}

// Reconstructed from eboot.elf at 0x2FEAE0. Room is made for every object
// but the root first.
void PropAnimResource::HarvestAllKeys(eastl::vector<PropKeysBaseCom*>& keys, bool includeIncomplete) const {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return;
    }
    keys.reserve(entity->NumObjects() - 1);
    for (GameObject* object = entity->BeginObject(); object != nullptr; object = entity->NextObject(object, Symbol())) {
        if (object == entity->GetRoot()) {
            continue;
        }
        PropKeysBaseCom* const objectKeys = FindKeysCom(object);
        if (includeIncomplete || objectKeys->mProp.IsComplete()) {
            keys.push_back(objectKeys);
        }
    }
}

// Reconstructed from eboot.elf at 0x2FEEB0.
PropKeysBaseCom* PropAnimResource::FindKeys(const PropRef& prop, GameObjectId& object) const {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return nullptr;
    }
    for (GameObject* candidate = entity->BeginObject(); candidate != nullptr;
         candidate = entity->NextObject(candidate, Symbol())) {
        if (candidate == entity->GetRoot()) {
            continue;
        }
        PropKeysBaseCom* const keys = FindKeysCom(candidate);
        if (keys->mProp == prop) {
            object = candidate->mId;
            return keys;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x2FEFE0.
PropKeysBaseCom* PropAnimResource::_FindNextKeys(GameObjectId object, Symbol com, GameObjectId& keysObject) const {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return nullptr;
    }
    for (GameObject* candidate = entity->BeginObject(); candidate != nullptr;
         candidate = entity->NextObject(candidate, Symbol())) {
        if (candidate == entity->GetRoot()) {
            continue;
        }
        PropKeysBaseCom* const keys = FindKeysCom(candidate);
        if (keys->mProp.mObject.mId == object.mId && (com == Symbol() || keys->mProp.mCom == com)) {
            keysObject = candidate->mId;
            return keys;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x2FF1B0.
void PropAnimResource::RemoveAllKeys() {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return;
    }
    GameObject* object = entity->BeginObject();
    while (object != nullptr) {
        GameObject* const next = entity->NextObject(object, Symbol());
        if (object != entity->GetRoot()) {
            entity->DestroyObject(object->mId);
        }
        object = next;
    }
}

// Reconstructed from eboot.elf at 0x2FF220.
bool PropAnimResource::RemoveKeys(const PropRef& prop) {
    GameObjectId keysObject = {kInvalidObjectId};
    if (FindKeys(prop, keysObject) == nullptr || mEntity == nullptr) {
        return false;
    }
    mEntity->DestroyObject(keysObject);
    return true;
}

// Reconstructed from eboot.elf at 0x2FF280.
bool PropAnimResource::RemoveKeys(GameObjectId object) {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return false;
    }
    GameObjectId keysObject = {kInvalidObjectId};
    const bool found = _FindNextKeys(object, Symbol(""), keysObject) != nullptr;
    if (found) {
        do {
            entity->DestroyObject(keysObject);
        } while (_FindNextKeys(object, Symbol(""), keysObject) != nullptr);
    }
    eastl::vector<GameObjectId> order;
    GetTargetOrder(order);
    for (GameObjectId* id = order.begin(); id != order.end(); ++id) {
        if (id->mId == object.mId) {
            order.erase(id);
            break;
        }
    }
    SetTargetOrder(order);
    return found;
}

// Reconstructed from eboot.elf at 0x2FF410.
bool PropAnimResource::RemoveKeys(GameObjectId object, Symbol com) {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return false;
    }
    GameObjectId keysObject = {kInvalidObjectId};
    const bool found = _FindNextKeys(object, com, keysObject) != nullptr;
    if (found) {
        do {
            entity->DestroyObject(keysObject);
        } while (_FindNextKeys(object, com, keysObject) != nullptr);
    }
    return found;
}

// Reconstructed from eboot.elf at 0x2FF4C0. Every object but the root must
// hold keys.
void PropAnimResource::ApplyFrame(float frame, Entity* entity) const {
    Entity* const own = mEntity;
    if (own == nullptr) {
        return;
    }
    for (GameObject* object = own->BeginObject(); object != nullptr; object = own->NextObject(object, Symbol())) {
        if (object != own->GetRoot()) {
            GetExistingKeysCom(object)->ApplyFrame(frame, entity);
        }
    }
}

// Reconstructed from eboot.elf at 0x2FF5B0.
bool PropAnimResource::HasKeys(GameObjectId object, Symbol com, const PropPath& path, bool* overlaps) const {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return false;
    }
    for (GameObject* candidate = entity->BeginObject(); candidate != nullptr;
         candidate = entity->NextObject(candidate, Symbol())) {
        if (candidate == entity->GetRoot()) {
            continue;
        }
        const PropKeysBaseCom* const keys = FindKeysCom(candidate);
        if (object.mId != kInvalidObjectId && keys->mProp.mObject.mId != object.mId) {
            continue;
        }
        if (com != Symbol() && keys->mProp.mCom != com) {
            continue;
        }
        if (path.mSize == 0 || path == keys->mProp.mPath) {
            return true;
        }
        if (overlaps != nullptr && (path.Contains(keys->mProp.mPath) || keys->mProp.mPath.Contains(path))) {
            *overlaps = true;
            return false;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2FF8F0. Every object but the root must
// hold keys.
void PropAnimResource::OffsetKeys(float offset, float threshold) {
    Entity* const entity = mEntity;
    if (entity == nullptr) {
        return;
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr; object = entity->NextObject(object, Symbol())) {
        if (object == entity->GetRoot()) {
            continue;
        }
        PropKeysBaseCom* const keys = GetExistingKeysCom(object);
        if (keys->NumKeys() == 0) {
            continue;
        }
        const float first = keys->GetKeyFrameAtIndex(0);
        keys->OffsetFrames(offset);
        if (!(first > threshold)) {
            keys->MoveKey(0, 0.0f, false);
        }
    }
    UpdateFrameRange();
}

// Reconstructed from eboot.elf at 0x2FFA50.
bool PropAnimResource::QueryValidPropTarget(const PropRef& prop, Entity* entity) const {
    GameObjectId keysObject = {kInvalidObjectId};
    const PropKeysBaseCom* const keys = FindKeys(prop, keysObject);
    return keys != nullptr && keys->QueryValidPropTarget(entity);
}

// Reconstructed from eboot.elf at 0x2FFAB0.
Entity* PropAnimResource::CreateEntity() {
    Entity* const entity = EntityResource::CreateEntity();
    entity->GetRoot()->CreateComponent(PropAnimCom::sClassName, false);
    return entity;
}

// Reconstructed from eboot.elf at 0x2FFAF0.
ResourceMetaData* PropAnimResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x2FFB00.
Symbol PropAnimResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2FFBA0.
bool PropAnimResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2FFBD0.
Resource* PropAnimResource::AsResource() {
    return this;
}

// Reconstructed from eboot.elf at 0x2FFBE0.
float PropAnimResource::GetTimeAlignment(TimeUnits units) const {
    static_cast<void>(units);
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x2B1DC0. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Resource* PropAnimResource::_Create() {
    return new PropAnimResource();
}

// Reconstructed from eboot.elf at 0x2A7B00. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropAnimResource::Init() {
    _Init(sMetaData);
    const Symbol id = Id();
    Resource::sFactory[id] = &_Create;
    sMetaData.Init(id, Symbol("EntityResource"), true);
}
