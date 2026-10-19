// anim/CharIk.o (0x2F1360 to 0x2F157F): the class's builder and statics.
// The virtual methods, the registration and the factory are emitted in
// anim/AnimInit.o; _InitAsSuperclass in anim/CharIkBoneInterpCom.o.
#include "anim/char/CharIkCom.h"

#include <new>

#include "entity/core/GameObject.h"
#include "os/memory/MemMgr.h"
#include "render/scene/CharResource.h"
#include "utl/text/Str.h"

namespace {

// The -1 of a shared header that the static initializer sets (0x19F8E20).
// Nothing in this object reads it. Name not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

}  // namespace

// The object's statics, in the order of its static initializer (0x2F14C0).
Symbol CharIkCom::sId("CharIk");
Symbol CharIkCom::sClassName("CharIk");
PropRegistry CharIkCom::sPropRegistry;
ComMetaData CharIkCom::sMetaData;

// Reconstructed from eboot.elf at 0x2F1360. The class registers no
// properties and inherits no registry.
void CharIkCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    static_cast<void>(registry);
    metadata.mCategory = ComMetaData::kCategoryCharacter;
    metadata.mDescription = "Base class for all CharIk components";
    metadata.mAuthor = "James Fleming";
    metadata.mAllowedResources.push_back(CharResource::Id());
    metadata.mEditorRestrictions = 0;
}

// Reconstructed from eboot.elf at 0x2F3C20. The heap index is looked up
// once.
void CharIkCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("CharIkCom", sMetaData);
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

// Reconstructed from eboot.elf at 0x2B6020. The deleting destructor is at
// 0x2B6050; the link unlinks itself.
CharIkCom::~CharIkCom() {}

// Reconstructed from eboot.elf at 0x2B6090.
Symbol CharIkCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2B60A0.
Symbol CharIkCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2B60B0.
int CharIkCom::CurrentRev() const {
    return const_cast<CharIkCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2B60D0.
bool CharIkCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2B6100.
Component* CharIkCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2B6110. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CharIkCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CharIkCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CharIkCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2B6240.
PropRegistry& CharIkCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2B6250.
ComMetaData& CharIkCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2B6260.
void CharIkCom::Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) {
    static_cast<void>(pose);
    static_cast<void>(skeleton);
}

// Reconstructed from eboot.elf at 0x2B6270.
float CharIkCom::GetPriority() const {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x2B5FA0. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* CharIkCom::_Create() {
    return new CharIkCom();
}

// Reconstructed from eboot.elf at 0x2AEBE0. The binary emits the
// registration in anim/AnimInit.o, which calls it; the class is not
// created itself.
void CharIkCom::Init() {
    InitComponentClass<CharIkCom>(false, false);
}
