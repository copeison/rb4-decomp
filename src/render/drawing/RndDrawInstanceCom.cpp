// render/RndDrawInstanceCom.o (0x6C1500 to 0x6C3D3F). The object's static
// initializer is at 0x6C3C70.
#include "render/drawing/RndDrawInstanceCom.h"

#include <cstring>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialReferenceCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

// Enables the draw instances' sort keys (RndDrawInstanceCom::_Poll). It
// lies in the data after gTimerThresholdMs and has no other reader or writer
// in the binary, so the name is weak. Name not in the reference map.
extern bool gDrawInstanceSortKeys;  // 0x19B03C1

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x6C3C9C; see RndTypesetter.cpp). This object
// reads the -1 as the null object id of a material reference. Names not in
// the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};          // 0x1AB0360
[[maybe_unused]] int gDrawInstanceGroupSize2D = 8;  // 0x1AB0364
[[maybe_unused]] int gDrawInstanceGroupSize3D = 4;  // 0x1AB0368

// The instances of a list, which the subclass owns. VectorAdapter is a
// read-only view; the components write the instances in place. Name not in
// the reference map.
RndDrawInstance* Instances(const VectorAdapter<RndDrawInstance>& list) {
    return const_cast<RndDrawInstance*>(list.mData);
}

}  // namespace

// The object's statics, in the order of its static initializer.
Symbol RndDrawInstanceCom::sId("DrawInstance");
Symbol RndDrawInstanceCom::sClassName("DrawInstance");
PropRegistry RndDrawInstanceCom::sPropRegistry;
ComMetaData RndDrawInstanceCom::sMetaData;

unsigned long RndDrawInstanceCom::NumSceneLods() {
    return gRndDevice->mSettings->mUseLod ? 3 : 1;
}

// Reconstructed from eboot.elf at 0x6C1500.
RndDrawInstanceCom::RndDrawInstanceCom()
    : mBillboarding(0),
      mSortBy(2),
      mSortingHint(false),
      mLods(7),
      mExtraData(Vector4::sZero),
      mExtraData1(Vector4::sZero),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x6C1590. The link unlinks itself.
RndDrawInstanceCom::~RndDrawInstanceCom() {}

// Reconstructed from eboot.elf at 0x6C1610.
void RndDrawInstanceCom::InitDrawInstances(VectorAdapter<RndDrawInstance>* instances) {
    mRuntime.mInstancesInited = true;
    const Entity* entity = mObject->mEntity;
    const unsigned long lods = NumSceneLods();
    for (unsigned long lod = 0; lod < lods; ++lod) {
        mRuntime.mDrawInstances[lod] = instances[lod];
        RndDrawInstance* list = Instances(instances[lod]);
        for (unsigned long i = 0; i < instances[lod].mSize; ++i) {
            list[i].mShowing = (entity->mFlags & 0x100) != 0;
        }
        if (instances[lod].mSize != 0) {
            _InitDrawInstancesImpl(instances[lod]);
        }
    }
    _SyncDrawInstances();
}

// Reconstructed from eboot.elf at 0x6C1720.
void RndDrawInstanceCom::_SyncDrawInstances() {
    const unsigned long lods = NumSceneLods();
    for (unsigned long lod = 0; lod < lods; ++lod) {
        if (mRuntime.mDrawInstances[lod].mSize != 0) {
            _SyncDrawInstancesImpl(mRuntime.mDrawInstances[lod]);
        }
    }
}

// Reconstructed from eboot.elf at 0x6C1780.
void RndDrawInstanceCom::SetInstanceClipPlane(unsigned long index, const Vector4& plane) {
    const unsigned long lods = NumSceneLods();
    for (unsigned long lod = 0; lod < lods; ++lod) {
        RndDrawInstance* list = Instances(mRuntime.mDrawInstances[lod]);
        for (unsigned long i = 0; i < mRuntime.mDrawInstances[lod].mSize; ++i) {
            list[i].mClipPlanes[index] = plane;
        }
    }
}

// Reconstructed from eboot.elf at 0x6C2B30.
void RndDrawInstanceCom::_PreDestroy(DestroyType type) {
    if (type != kDestroyComponent) {
        return;
    }
    RndDrawNodeCom* drawNode = mObject->GetCom<RndDrawNodeCom>();
    if (drawNode == nullptr) {
        return;
    }
    const TransCom* trans = mObject->GetExistingCom<TransCom>();
    Sphere sphere;
    sphere.center = trans->mWorldXfm.v;
    sphere.radius = 0.0F;
    drawNode->SetLocalSphere(sphere);
}

// Reconstructed from eboot.elf at 0x6C2C00.
bool RndDrawInstanceCom::_OnResourcesLoaded() {
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    mRuntime.mMaterial = mObject->GetCom<RndMaterialCom>();
    mRuntime.mMaterialReference =
        mRuntime.mMaterial == nullptr ? mObject->GetCom<RndMaterialReferenceCom>() : nullptr;
    mRuntime.mDefaultMaterial = _GetDefaultMaterial();
    return true;
}

// Reconstructed from eboot.elf at 0x6C2D70.
void RndDrawInstanceCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(RndDrawNodeCom::sClassName);
    follows.push_back(RndMaterialCom::sClassName);
    follows.push_back(RndMaterialReferenceCom::sClassName);
}

// Reconstructed from eboot.elf at 0x6C2F80. The body of
// _RegisterWithSceneDrawer is inlined.
void RndDrawInstanceCom::_Enter() {
    RndSceneDrawer* drawer = RndSceneDrawer::FindForEntity(mObject->mEntity);
    if (drawer != nullptr) {
        _RegisterWithSceneDrawer(drawer);
    }
}

// Reconstructed from eboot.elf at 0x6C3070.
void RndDrawInstanceCom::_RegisterWithSceneDrawer(RndSceneDrawer* drawer) {
    if (drawer != nullptr && mRuntime.mRegistered) {
        drawer->DeRegisterDrawInstanceCom(*this);
    }
    mRuntime.mRegistered = false;
    mRuntime.mInstancesInited = false;
    std::memset(mRuntime.mDrawInstances, 0, NumSceneLods() * sizeof(mRuntime.mDrawInstances[0]));
    if (drawer != nullptr && !mRuntime.mRegistrationSuppressed) {
        const unsigned long lods = NumSceneLods();
        bool hasInstances = false;
        for (unsigned long lod = 0; lod < lods; ++lod) {
            if (_GetNumDrawInstancesImpl(static_cast<RndSceneLod>(lod)) != 0) {
                hasInstances = true;
                break;
            }
        }
        if (hasInstances) {
            drawer->RegisterDrawInstanceCom(*this);
        }
    }
    mRuntime.mRegistered = true;
}

// Reconstructed from eboot.elf at 0x6C3150. When the whole entity goes,
// the component forgets its drawer without leaving it.
void RndDrawInstanceCom::_Exit(DestroyType type) {
    if (!mRuntime.mRegistered) {
        return;
    }
    if (type < kDestroyInstance) {
        mRuntime.mRegistered = false;
        mRuntime.mInstancesInited = false;
        std::memset(mRuntime.mDrawInstances, 0, sizeof(mRuntime.mDrawInstances));
    } else {
        _DeRegisterWithSceneDrawer(RndSceneDrawer::FindForEntity(mObject->mEntity));
    }
}

// Reconstructed from eboot.elf at 0x6C31F0.
void RndDrawInstanceCom::_DeRegisterWithSceneDrawer(RndSceneDrawer* drawer) {
    if (drawer != nullptr && mRuntime.mRegistered) {
        drawer->DeRegisterDrawInstanceCom(*this);
    }
    mRuntime.mRegistered = false;
    mRuntime.mInstancesInited = false;
    const unsigned long lods = NumSceneLods();
    for (unsigned long lod = 0; lod < lods; ++lod) {
        mRuntime.mDrawInstances[lod] = VectorAdapter<RndDrawInstance>();
    }
}

// Reconstructed from eboot.elf at 0x6C3260.
void RndDrawInstanceCom::_Poll() {
    mRuntime.mActiveSortKey = gDrawInstanceSortKeys ? mRuntime.mSortKey : 0;
    if (mRuntime.mInstancesInited) {
        _SyncDrawInstances();
    }
}

// Reconstructed from eboot.elf at 0x6C32E0.
void RndDrawInstanceCom::_OnDeactivate() {
    for (const VectorAdapter<RndDrawInstance>& list : mRuntime.mDrawInstances) {
        for (unsigned long i = 0; i < list.mSize; ++i) {
            Instances(list)[i].mShowing = false;
        }
    }
}

// Reconstructed from eboot.elf at 0x6C3380.
void RndDrawInstanceCom::_OnActivate() {
    for (const VectorAdapter<RndDrawInstance>& list : mRuntime.mDrawInstances) {
        for (unsigned long i = 0; i < list.mSize; ++i) {
            Instances(list)[i].mShowing = true;
        }
    }
}

// Reconstructed from eboot.elf at 0x6C3420.
void RndDrawInstanceCom::_EditEnter() {
    _Enter();
}

// Reconstructed from eboot.elf at 0x6C3430. The same as _Exit.
void RndDrawInstanceCom::_EditExit(DestroyType type) {
    if (!mRuntime.mRegistered) {
        return;
    }
    if (type < kDestroyInstance) {
        mRuntime.mRegistered = false;
        mRuntime.mInstancesInited = false;
        std::memset(mRuntime.mDrawInstances, 0, sizeof(mRuntime.mDrawInstances));
    } else {
        _DeRegisterWithSceneDrawer(RndSceneDrawer::FindForEntity(mObject->mEntity));
    }
}

// Reconstructed from eboot.elf at 0x6C34C0.
void RndDrawInstanceCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x6C34D0.
unsigned long RndDrawInstanceCom::_GetNumDrawInstancesImpl(RndSceneLod lod) const {
    return (mLods >> lod) & 1;
}

// Reconstructed from eboot.elf at 0x6C34E0.
RndMaterialCom* RndDrawInstanceCom::_GetDefaultMaterial() const {
    return gRndDevice->mDefaults.mUnlitMaterial;
}

// Reconstructed from eboot.elf at 0x6C3500.
void RndDrawInstanceCom::_SyncInstanceStateFlags(VectorAdapter<RndDrawInstance>& instances) {
    RndMaterialCom* material = mRuntime.mMaterial;
    const RndDrawNodeCom* drawNode = mRuntime.mDrawNode;
    const RndQualityLevel quality = gRndDevice->mSettings->mQualityLevel;
    if (material == nullptr) {
        const RndMaterialReferenceCom* reference = mRuntime.mMaterialReference;
        if (reference != nullptr && reference->mMaterial.mId != gNullObjectId.mId) {
            const GameObject* object = reference->mObject->mEntity->GetObject(reference->mMaterial);
            if (object != nullptr) {
                material = object->GetCom<RndMaterialCom>();
            }
        }
        if (material == nullptr) {
            material = mRuntime.mDefaultMaterial;
        }
    }
    const unsigned int hiddenFlag = (drawNode->mRuntime.mWorldShowHideFlags << 9) & 0x40000;
    RndDrawInstance* list = Instances(instances);
    for (unsigned long i = 0; i < instances.mSize; ++i) {
        list[i].mStateFlags =
            material->mRuntimeData->mUsageHints[static_cast<unsigned int>(quality)] | hiddenFlag;
    }
}

// Reconstructed from eboot.elf at 0x6C3620.
void RndDrawInstanceCom::_ReRegisterWithSceneDrawer() {
    if (mRuntime.mRegistered) {
        _RegisterWithSceneDrawer(RndSceneDrawer::FindForEntity(mObject->mEntity));
    }
}

// Reconstructed from eboot.elf at 0x6C3710.
Symbol RndDrawInstanceCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6C3720.
Symbol RndDrawInstanceCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6C3730.
int RndDrawInstanceCom::CurrentRev() const {
    return const_cast<RndDrawInstanceCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6C3750.
bool RndDrawInstanceCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6C3780.
Component* RndDrawInstanceCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6C3790. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndDrawInstanceCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndDrawInstanceCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndDrawInstanceCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6C3900.
PropRegistry& RndDrawInstanceCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6C3910.
ComMetaData& RndDrawInstanceCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x44ADA0. The function is shared with
// the subclasses' identical bodies.
bool RndDrawInstanceCom::_DrawsGeometry() const {
    return true;
}
