// The "UILayout" component's object (0x8A9340 to 0x8AAA3F). Only the poll
// manager functions are reconstructed.
#include "ui/layout/UILayoutCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "os/memory/PoolAlloc.h"
#include "utl/files/FileUtl.h"
#include "utl/text/Str.h"
#include "utl/threading/PollMgr.h"

namespace {

// The size of the small-pool block a nested poll manager lives in. Name not
// in the reference map.
constexpr unsigned long kPollMgrBlockSize = 544;

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
void UILayoutCom::_DestroyPollMgrs() {
    if (mScenePollGroups != nullptr) {
        mScenePollGroups->Reset();
    }
    if (mPollMgr != nullptr) {
        mPollMgr->Reset();
    }
    delete mScenePollGroups;
    mScenePollGroups = nullptr;
    if (mPollMgr != nullptr) {
        mPollMgr->~PollMgr();
        PoolFree(kPollMgrBlockSize, mPollMgr);
    }
    mPollMgr = nullptr;
}

// Reconstructed from eboot.elf at 0x8A98D0.
void UILayoutCom::_ObtainPollGroup() {
    const String name(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    if (mPollGroup == nullptr) {
        mPollGroup = new PollGroup(name.c_str());
    }
}

// Reconstructed from eboot.elf at 0x8A9960.
void UILayoutCom::_CreatePollMgrs() {
    const String name(FileGetName(mObject->mEntity->mResource->mPath.Str()));
    mScenePollGroups = new RndSceneCom::PollGroups(name.c_str());
    PollMgr* const pollMgr = static_cast<PollMgr*>(PoolAlloc(kPollMgrBlockSize, nullptr));
    new (pollMgr) PollMgr(name.c_str());
    mPollMgr = pollMgr;
}

// Reconstructed from eboot.elf at 0x8A9B00.
void UILayoutCom::RemoveSceneObject(GameObjectId id) {
    EraseId(mPollOrder, id);
    EraseId(mDrawOrder, id);
    EraseId(mHmdMirrorOverlays, id);
}
