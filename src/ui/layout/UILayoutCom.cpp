// The "UILayout" component's object (0x8A9340 to 0x8AAA3F). Only the poll
// manager and group functions are reconstructed.
#include "ui/layout/UILayoutCom.h"

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "utl/files/FileUtl.h"
#include "utl/text/Str.h"
#include "utl/threading/PollMgr.h"

namespace {

// Removes the first element equal to the id, as the binary's inlined
// PropArray erase does: destroys it and moves the later elements down
// through the array's vtable. Name not in the reference map.
void EraseId(PropArray<GameObjectId>& array, GameObjectId id) {
    for (unsigned long i = 0; i < array.mSize; ++i) {
        if (array.data()[i].mId != id.mId) {
            continue;
        }
        array.Destruct(1, array.ElementAt(i));
        if (static_cast<unsigned int>(i) < array.mSize - 1) {
            array.Move(array.mSize - 1 - i, array.ElementAt(i), array.ElementAt(i + 1));
        }
        --array.mSize;
        return;
    }
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8AA970).
Symbol UILayoutCom::sId("UILayout");
Symbol UILayoutCom::sClassName("UILayout");

// Reconstructed from eboot.elf at 0x8A9830.
void UILayoutCom::_DestroyPollGroups() {
    if (mScenePollGroups != nullptr) {
        mScenePollGroups->Reset();
    }
    if (mPollGroup != nullptr) {
        mPollGroup->Reset();
    }
    delete mScenePollGroups;
    mScenePollGroups = nullptr;
    delete mPollGroup;
    mPollGroup = nullptr;
}

// Reconstructed from eboot.elf at 0x8A98D0.
void UILayoutCom::_ObtainPollMgr() {
    const String name(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    if (mPollMgr == nullptr) {
        mPollMgr = new PollMgr(name.c_str());
    }
}

// Reconstructed from eboot.elf at 0x8A9960.
void UILayoutCom::_CreatePollGroups() {
    const String name(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    mScenePollGroups = new RndSceneCom::PollGroups(name.c_str());
    mPollGroup = new PollGroup(name.c_str());
}

// Reconstructed from eboot.elf at 0x8A9B00.
void UILayoutCom::RemoveSceneObject(GameObjectId id) {
    EraseId(mPollOrder, id);
    EraseId(mDrawOrder, id);
    EraseId(mHmdMirrorOverlays, id);
}
