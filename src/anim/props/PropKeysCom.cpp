// anim/PropKeysCom.o (0x305350-0x305C8F): the key classes' registry and
// the base's out-of-line methods.
#include "anim/props/PropKeysCom.h"

#include "anim/props/PropAnimCom.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// A key class's registry key: the property type and, for structs, its name.
// Name not in the reference map.
struct KeysType {
    PropertyType mType;
    Symbol mSubType;

    bool operator<(const KeysType& other) const {
        if (mType != other.mType) {
            return mType < other.mType;
        }
        return mSubType < other.mSubType;
    }
};

// The key class of a type and the factory of its drivers. Name not in the
// reference map.
struct KeysFactory {
    Symbol mClassId;
    PropKeysBaseCom::PropDriverFactory mDriverFactory;
};

// The registered key classes, constructed by the static initializer
// (0x305BC0), which first sets a shared header's null object id to -1
// (0x19F9EB8). Name not in the reference map.
eastl::map<KeysType, KeysFactory> sKeysFactories;  // 0x19F9EC0

}  // namespace

TagRemapFunc PropKeysBaseCom::sRemapKeyValue = PropKeysBaseCom::_RemapKeyValue;

// Reconstructed from eboot.elf at 0x305350.
void PropKeysBaseCom::RegisterFactory(
    PropertyType type,
    const Symbol& subType,
    const Symbol& classId,
    PropDriverFactory factory) {
    sKeysFactories[KeysType{type, subType}].mClassId = classId;
    sKeysFactories[KeysType{type, subType}].mDriverFactory = factory;
}

// Reconstructed from eboot.elf at 0x3054E0.
Entity* PropKeysBaseCom::_GetContextEntityPrototype() const {
    return mObject->mEntity->mResource->mInlineOwner->mEntity;
}

// Reconstructed from eboot.elf at 0x305500. The root's PropAnimCom lookup
// is left over from a stripped check.
GameObjectId PropKeysBaseCom::NewKeysType(
    Entity* entity,
    PropertyType type,
    Symbol subType,
    const PropRef& prop,
    PropKeysBaseCom** keys,
    GameObject* object) {
    static_cast<void>(entity->GetRoot()->GetCom<PropAnimCom>());
    const auto found = sKeysFactories.find(KeysType{type, subType});
    if (found == sKeysFactories.end()) {
        return GameObjectId{0xFFFFFFFFU};
    }
    if (object == nullptr) {
        object = entity->CreateObject(0, 0);
    }
    const String name((FormatString("Keys type %d %s") << static_cast<int>(type) << subType).Str());
    object->SetName(Symbol(name.c_str()));
    *keys = static_cast<PropKeysBaseCom*>(object->CreateComponent(found->second.mClassId, false));
    (*keys)->_Setup(prop);
    return object->mId;
}

// Reconstructed from eboot.elf at 0x305750.
PropKeysBaseCom::~PropKeysBaseCom() {}

// Reconstructed from eboot.elf at 0x305780.
bool PropKeysBaseCom::QueryValidPropTarget(Entity* entity) const {
    return mProp.Exists(entity);
}

// Reconstructed from eboot.elf at 0x305800.
void PropKeysBaseCom::_RemapKeyValue(TagRemapArgs& args) {
    const PropKeysBaseCom& keys = static_cast<const PropKeysBaseCom&>(args.mComponent);
    Entity* const entity = keys._GetContextEntityPrototype();
    args.mEntity = entity;
    args.mTargetCom = keys.mProp.Com(entity);
    args.mTargetPath = keys.mProp.mPath;
    args.mTargetInfo = keys.mProp.Find(entity, nullptr);
}
