// render/RndLookAtCameraCom.o (0x4066D0 to 0x407F2B). The property
// callbacks at 0x407C10-0x407E50 are not reconstructed.
#include "render/context/RndLookAtCameraCom.h"

#include <cmath>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropArray.h"
#include "math/matrix/Matrix3.h"
#include "math/rotation/Rot.h"
#include "math/transform/Transform.h"
#include "render/context/RndCameraCom.h"
#include "render/drawing/RndBillboard.h"
#include "render/scene/RndSceneCom.h"

namespace {

// The id of no object. The binary reads it from a shared header's global
// (0x1A71FB8). Name not in the reference map.
constexpr unsigned int kNoObject = 0xFFFFFFFFU;

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// A zero vector stays zero.
Vector3 Normalize(const Vector3& v) {
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    const float scale = length != 0.0F ? 1.0F / length : 0.0F;
    return {scale * v.x, scale * v.y, scale * v.z};
}

// The rows of `a` times `b`. Name not in the reference map.
Hmx::Matrix3 MultiplyRows(const Hmx::Matrix3& a, const Hmx::Matrix3& b) {
    const auto row = [&b](const Vector3& r) -> Vector3 {
        return {r.x * b.x.x + r.y * b.y.x + r.z * b.z.x,
                r.x * b.x.y + r.y * b.y.y + r.z * b.z.y,
                r.x * b.x.z + r.y * b.y.z + r.z * b.z.z};
    };
    return {row(a.x), row(a.y), row(a.z)};
}

// The scene component of the nearest entity, from `entity` up through the
// entities that instance it, or null. Inlined into _Poll. Name not in the
// reference map.
RndSceneCom* FindScene(Entity* entity) {
    for (; entity != nullptr; entity = entity->GetParent()) {
        RndSceneCom* scene = entity->GetRoot()->GetCom<RndSceneCom>();
        if (scene != nullptr) {
            return scene;
        }
    }
    return nullptr;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x407E50).
Symbol RndLookAtCameraCom::sId("LookAtCamera");
Symbol RndLookAtCameraCom::sClassName("LookAtCamera");
PropRegistry RndLookAtCameraCom::sPropRegistry;
ComMetaData RndLookAtCameraCom::sMetaData;

// Reconstructed from eboot.elf at 0x4066D0.
RndLookAtCameraCom::RndLookAtCameraCom()
    : mFacingType(kBillboardCamera), mFacingAxis(kBillboardAxisNegY) {}

// Reconstructed from eboot.elf at 0x406F30. The deleting destructor is at
// 0x407A20.
RndLookAtCameraCom::~RndLookAtCameraCom() {}

// Reconstructed from eboot.elf at 0x406F40.
void RndLookAtCameraCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(TransCom::sClassName);
}

// Reconstructed from eboot.elf at 0x407000. The facing rotation is taken
// into the parent's frame and stored as the transform's Euler angles. With
// kBillboardCameraKeepZ the Z axis comes from the current local rotation;
// otherwise the facing Y axis is kept and the others rebuilt around it.
void RndLookAtCameraCom::_Poll() {
    if (mFacingType == kBillboardNone) {
        return;
    }
    TransCom* trans = mObject->GetCom<TransCom>();
    Entity* entity = mObject->mEntity;
    RndSceneCom* scene = FindScene(entity);
    if (scene == nullptr) {
        return;
    }
    RndCameraCom* camera = scene->GetCamera(0);
    if (camera == nullptr) {
        return;
    }
    TransCom* cameraTrans = camera->mObject->GetCom<TransCom>();
    if (cameraTrans == nullptr || trans == nullptr) {
        return;
    }

    const Transform world = trans->mWorldXfm;
    const Transform cameraXfm = cameraTrans->mWorldXfm;
    const Transform facing =
        ComputeBillboardXfm(world, cameraXfm, mFacingType, mFacingAxis);

    Hmx::Matrix3 toParent = Hmx::Matrix3::sID;
    const TransCom* parentTrans = nullptr;
    if (trans->mTransParent.mId != kNoObject) {
        GameObject* parent = entity->GetObject(trans->mTransParent);
        if (parent != nullptr) {
            parentTrans = parent->GetCom<TransCom>();
        }
    }
    if (parentTrans != nullptr) {
        Invert(parentTrans->mWorldXfm.m, toParent, nullptr);
    }
    Hmx::Matrix3 local = MultiplyRows(facing.m, toParent);

    if (mFacingType == kBillboardCameraKeepZ) {
        // The binary builds the current rotation through the wrapper at
        // 0x2186A0.
        Hmx::Matrix3 current = Hmx::Matrix3::sID;
        MakeRotMatrix(trans->mLocalXfm.mEuler, current, true);
        local.z = Normalize(current.z);
        local.x = Normalize(Cross(local.y, local.z));
        local.y = Cross(local.z, local.x);
    } else {
        local.y = Normalize(local.y);
        local.x = Normalize(Cross(local.y, local.z));
        local.z = Cross(local.x, local.y);
    }
    // The binary calls MakeEuler through the thunk at 0x218610.
    MakeEuler(local, trans->mLocalXfm.mEuler);
    trans->mDirtyFlags |= 1;
    trans->_Poll();
}

// Reconstructed from eboot.elf at 0x4078E0.
void RndLookAtCameraCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x4078F0.
void RndLookAtCameraCom::_GetPollDeps(
    eastl::vector<GameObjectId>& before,
    eastl::vector<GameObjectId>& after) {
    static_cast<void>(after);
    RndSceneCom* scene = mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    if (scene == nullptr) {
        return;
    }
    RndCameraCom* camera = scene->GetCamera(0);
    if (camera == nullptr) {
        return;
    }
    before.push_back(camera->mObject->mId);
}

// Reconstructed from eboot.elf at 0x407A40.
Symbol RndLookAtCameraCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x407A50.
Symbol RndLookAtCameraCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x407A60.
int RndLookAtCameraCom::CurrentRev() const {
    return const_cast<RndLookAtCameraCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x407A80.
bool RndLookAtCameraCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x407AB0.
Component* RndLookAtCameraCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x407AC0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLookAtCameraCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLookAtCameraCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLookAtCameraCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x407BF0.
PropRegistry& RndLookAtCameraCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x407C00.
ComMetaData& RndLookAtCameraCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4043E0. The binary emits the factory
// with the other render factories.
Component* RndLookAtCameraCom::_Create() {
    return new RndLookAtCameraCom();
}
