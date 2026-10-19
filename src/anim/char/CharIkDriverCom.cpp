// anim/CharIkDriverCom.o (0x2F5120 to 0x2F66FF). The struct type's
// registration (the map's PropArray<CharIkDriverCom::IkRef>::Init(Symbol),
// 0x2F5760) and the property handlers after the identity slots
// (0x2F5E50-0x2F663F, with the map's
// TagBase<eastl::vector<GameObjectId>>::_Set and the vector's
// DoInsertValue) are the compiler's. The object's static initializer is at
// 0x2F6640. The class's registration and factory are emitted in
// anim/AnimInit.o.
#include "anim/char/CharIkDriverCom.h"

#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "utl/containers/Vector.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x19F9358, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The component a tag belongs to. Name not in the reference map.
const CharIkDriverCom& Driver(const TagArgs& args) {
    return static_cast<const CharIkDriverCom&>(args.mComponent);
}

// The object an element of "ik" names, or null. Name not in the reference
// map.
GameObject* RefObject(const CharIkDriverCom& driver, const CharIkDriverCom::IkRef& ref) {
    if (ref.mObject.mId == gNullObjectId.mId) {
        return nullptr;
    }
    return driver.mObject->mEntity->GetObject(ref.mObject);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x2F6640).
Symbol CharIkDriverCom::sId("CharIkDriver");
Symbol CharIkDriverCom::sClassName("CharIkDriver");
PropRegistry CharIkDriverCom::sPropRegistry;
ComMetaData CharIkDriverCom::sMetaData;

// Reconstructed from eboot.elf at 0x2F5120.
void CharIkDriverCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<IkRef>(Symbol("IkRef"));
    _InitAsSuperclass(registry, metadata);
    CharIkDriverCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    PropInfo& iks = registry.RegisterPropArray("ik", offsetof(CharIkDriverCom, mIk), sizeof(PropArray<IkRef>));
    ArrayMetadata& iksMetadata = static_cast<ArrayMetadata&>(*iks.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(iks, kPropertyStruct, sizeof(IkRef));
    GroupMetadata& ik = static_cast<GroupMetadata&>(*iksMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const IkRef refPrototype;
    // "<object> : <component>", or the element's index without an object.
    ik.mDisplayName._Set([](const TagArgs& args) {
        const CharIkDriverCom& driver = Driver(args);
        const unsigned long index = args.mPath[-1].mValue;
        const IkRef& ref = driver.mIk[index];
        const GameObject* const object = RefObject(driver, ref);
        const char* text;
        if (object != nullptr) {
            FormatString format("%s : %s");
            format << object->mName << ref.mComponent;
            text = format.Str();
        } else {
            text = MakeString("%d", index);
        }
        return String(text);
    });
    PropRegistry& members = ik.mRegistry;

    GameObjectIdMetadata& object = TypeSpecificMetadata(
        members.RegisterProp("object", offsetof(IkRef, mObject), kPropertyGameObjectId, sizeof(GameObjectId)),
        refPrototype.mObject);
    object.mDefault = refPrototype.mObject;
    static_cast<void>(Symbol("prop"));
    object.mHelp = String("Object id of the game object that the component is on");
    // The objects that hold a CharIkCom.
    object.mAllowedObjects._Set([](const TagArgs& args) {
        eastl::vector<GameObjectId> objects;
        const Entity* const entity = args.mComponent.mObject->mEntity;
        for (GameObject* candidate = entity->BeginObject(); candidate != nullptr;
             candidate = entity->NextObject(candidate, Symbol())) {
            for (const GameObject::ComIndex& index : candidate->mComs) {
                if (index.mCom != nullptr && index.mCom->IsA(CharIkCom::sClassName)) {
                    objects.push_back(candidate->mId);
                    break;
                }
            }
        }
        return objects;
    });

    SymbolMetadata& component = TypeSpecificMetadata(
        members.RegisterProp("component", offsetof(IkRef, mComponent), kPropertySymbol, sizeof(Symbol)),
        refPrototype.mComponent);
    component.mDefault = refPrototype.mComponent;
    static_cast<void>(Symbol("prop"));
    component.mHelp = String("Component name of the world ik component");
    // The CharIkCom classes of the element's object.
    component.mAllowedValues._Set([](const TagArgs& args) {
        eastl::vector<AllowedValue<Symbol>> classes;
        const CharIkDriverCom& driver = Driver(args);
        const IkRef& ref = driver.mIk[args.mPath[-2].mValue];
        const GameObject* const target = RefObject(driver, ref);
        if (target != nullptr) {
            for (const GameObject::ComIndex& index : target->mComs) {
                Component* const ikCom = index.mCom;
                if (ikCom != nullptr && ikCom->IsA(CharIkCom::sClassName)) {
                    const Symbol id = ikCom->GetId();
                    classes.push_back(AllowedValue<Symbol>{id, String(ikCom->GetId().Str()), String("")});
                }
            }
        }
        return classes;
    });

    metadata.mDescription = "Put on the root object to allow char anim to include Inverse Kinematics (ik).";
    metadata.mAuthor = "James Fleming";
    metadata.mRootOnly = true;
}

// Reconstructed from eboot.elf at 0x2F5910. The deleting destructor is at
// 0x2F5B90; the list head unlinks itself.
CharIkDriverCom::~CharIkDriverCom() {}

// Reconstructed from eboot.elf at 0x2F59B0.
void CharIkDriverCom::Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) {
    for (CharIkCom& ik : mIks) {
        ik.Apply(pose, skeleton);
    }
}

// Reconstructed from eboot.elf at 0x2F5A10. The component is found by its
// class or interface symbol.
bool CharIkDriverCom::_OnResourcesLoaded() {
    while (!mIks.empty()) {
        mIks.remove(mIks.front());
    }
    const unsigned long count = mIk.size();
    for (unsigned long index = 0; index < count; ++index) {
        const IkRef& ref = mIk[index];
        const GameObject* const object = RefObject(*this, ref);
        if (object == nullptr) {
            continue;
        }
        Component* const component = object->FindCom(ref.mComponent);
        if (component != nullptr && component->IsA(CharIkCom::sClassName)) {
            mIks.push_back(*static_cast<CharIkCom*>(component));
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2F5C40.
Symbol CharIkDriverCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2F5C50.
Symbol CharIkDriverCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2F5C60.
int CharIkDriverCom::CurrentRev() const {
    return const_cast<CharIkDriverCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2F5C80.
bool CharIkDriverCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2F5CB0.
Component* CharIkDriverCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2F5CC0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CharIkDriverCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CharIkDriverCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CharIkDriverCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2F5E30.
PropRegistry& CharIkDriverCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2F5E40.
ComMetaData& CharIkDriverCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2B6280. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* CharIkDriverCom::_Create() {
    return new CharIkDriverCom();
}

// Reconstructed from eboot.elf at 0x2AEFC0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharIkDriverCom::Init() {
    InitComponentClass<CharIkDriverCom>();
}
