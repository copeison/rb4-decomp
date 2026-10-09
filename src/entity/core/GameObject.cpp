#include "entity/core/GameObject.h"

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "utl/text/MakeString.h"

// Reconstructed from eboot.elf at 0x115FC0. The object is named with the
// serial in the top 16 bits of its id.
const char* GameObject::MakeErrorName() const {
    const char* name;
    {
        FormatString format("%s (%u)");
        format << mName << (mId.mId >> 16);
        name = format.Str();
    }
    const char* const entity = mEntity->MakeErrorName();
    FormatString format("%s object in %s");
    format << name << entity;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1160F0.
void GameObject::SetName(Symbol name) {
    mName = name;
    mEntity->mResource->_OnObjectRenamed(this);
}

// Reconstructed from eboot.elf at 0x117700. A component table that changed
// is re-sorted now unless the caller defers it, and the entity is told.
Component* GameObject::CreateComponent(Symbol className, bool deferSort) {
    Component* const component = _CreateComponent(className, deferSort);
    if (mComsAdded) {
        mPollOrderDirty = true;
        mAllComsFlagged = false;
        if (!deferSort) {
            _SortComponents();
        }
        mEntity->mFlags |= 0x400;
    }
    return component;
}
