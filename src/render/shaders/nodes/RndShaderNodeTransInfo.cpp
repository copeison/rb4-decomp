// render/RndShaderNodeTransInfo.o: only the scene globals are
// reconstructed.
#include "render/shaders/nodes/RndShaderNodeTransInfo.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/transform/Transform.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/scene/RndSceneCom.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "utl/containers/FixedVector.h"

namespace {

// The id of no object, set by the object's static initializer (0x5A82E0)
// with the shared header's thread-group widths. Names not in the reference
// map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};  // 0x1AA2588
[[maybe_unused]] int gTransInfoGroupSize2D = 8;  // 0x1AA258C
[[maybe_unused]] int gTransInfoGroupSize3D = 4;  // 0x1AA2590

// The scene's shader-graph transforms, "trans_0" to "trans_3".
constexpr unsigned long kNumTransInfos = 4;
// Floats in one transform's three 16-byte registers.
constexpr unsigned long kFloatsPerTransInfo = 12;

}  // namespace

// Reconstructed from eboot.elf at 0x5A6980. Each transform is written as
// the three rows of a 3x4 matrix; objects that are unset or missing, and a
// scene entity without its scene component, give the identity.
void RndShaderNodeTransInfo::SetGlobalConstants(
    RndShaderCBuffer& cbuffer,
    Entity* entity) {
    RndDevice* const device = TheRndDevice();
    const RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
    FixedVector<Transform, kNumTransInfos> xfms;
    for (unsigned long i = 0; i < kNumTransInfos; ++i) {
        Transform xfm = Transform::sID;
        if (scene != nullptr) {
            const GameObjectId id = scene->mShaderGraphTrans[i];
            if (id.mId != gNullObjectId.mId) {
                const GameObject* const object = entity->GetObject(id);
                if (object != nullptr) {
                    xfm = object->GetExistingCom<TransCom>()->mWorldXfm;
                }
            }
        }
        xfms.push_back(xfm);
    }

    auto* out = static_cast<float*>(cbuffer.mData)
        + device->mShaderMgr.mSGraphTransInfos * 4;
    for (const Transform& xfm : xfms) {
        const Vector3* const axes[] = {&xfm.m.x, &xfm.m.y, &xfm.m.z, &xfm.v};
        for (int column = 0; column < 4; ++column) {
            out[column] = axes[column]->x;
            out[4 + column] = axes[column]->y;
            out[8 + column] = axes[column]->z;
        }
        out += kFloatsPerTransInfo;
    }
    cbuffer.mSyncPending = true;
}
