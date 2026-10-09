// render/RndMeshCom.o (0x5C3020 to 0x5CE6AF). The object's static
// initializer is at 0x5CE180.
#include "render/meshes/RndMeshCom.h"

#include <new>
#include <utility>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/geometry/Sphere.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialReferenceCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/meshes/RndDynamicGpuData.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/skeletons/RndSkeletonPoseCom.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "utl/text/MakeString.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x5CE1AC; see RndTypesetter.cpp). This object
// reads the -1 as the null object id of a material reference. Names not in
// the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};    // 0x1AA5DD4
[[maybe_unused]] int gMeshGroupSize2D = 8;  // 0x1AA5DD8
[[maybe_unused]] int gMeshGroupSize3D = 4;  // 0x1AA5DDC

// Whether the mesh is the component's to delete. Inlined everywhere. Name
// not in the reference map.
bool OwnsMesh(const RndMeshCom& com) {
    return com.mResourceType == RndMeshCom::kResourceUnique
        || com.mResourceUsage == RndMeshCom::kUsageUnique;
}

}  // namespace

// The object's statics, in the order of its static initializer. The
// initializer then sets an int at 0x1AA6040 to -1 and builds the default
// parameter blocks of the mesh utilities (0x1AA6048 to 0x1AA6407, starting
// with RndMeshUtl::CreateTriangleParams, constructor 0x5CA890); they are
// not modelled.
Symbol RndMeshCom::sId("Mesh");
Symbol RndMeshCom::sClassName("Mesh");
PropRegistry RndMeshCom::sPropRegistry;
ComMetaData RndMeshCom::sMetaData;
Symbol RndMeshCom::kSubObjectResource("mesh");

// Reconstructed from eboot.elf at 0x5C3020.
RndMeshCom::RndMeshCom()
    : mMeshResourcePath(),
      mResourceType(kResourceFile),
      mResourceUsage(kUsageShare),
      mDriven(false),
      mStaticBoundingSphere(false),
      mMeshResource(),
      mMesh(nullptr),
      mSkeleton(nullptr),
      mSkinned(false),
      mPendingMeshSync(0),
      mPendingUsage(-1) {}

// Reconstructed from eboot.elf at 0x5C30B0.
RndMeshCom::~RndMeshCom() {
    if (OwnsMesh(*this)) {
        delete mMesh;
        mMesh = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x5C31B0. The entity's inline mesh is
// looked up and dropped again only to decide on the reload.
void RndMeshCom::SetResourceType(ResourceType type) {
    if (mResourceType == type) {
        return;
    }
    if (mMesh != nullptr) {
        if (OwnsMesh(*this)) {
            delete mMesh;
            mMesh = nullptr;
        }
        mMesh = nullptr;
        mSkinned = false;
    }
    mResourceType = type;
    if (type == kResourceInline) {
        const GameObjectId id = mObject->mId;
        const char* extension = RndMeshResource::sMetaData.mExtensions.back().Str();
        FormatString format("mesh_inline_%d_%d.%s");
        format << id.Layer() << (id.mId >> 16) << extension;
        mMeshResourcePath = format.Str();
        ResourcePtr<RndMeshResource> inlined =
            mObject->mEntity->mResource->TryGetInline<RndMeshResource>(mMeshResourcePath);
        if (inlined != nullptr) {
            LoadResources(false);
            return;
        }
    } else {
        mMeshResourcePath = "";
    }
    mMeshResource = nullptr;
}

// Reconstructed from eboot.elf at 0x5C33A0.
void RndMeshCom::_SetMesh(RndMesh* mesh) {
    if (mMesh == mesh) {
        return;
    }
    if (OwnsMesh(*this)) {
        delete mMesh;
        mMesh = nullptr;
    }
    mMesh = mesh;
    if (mesh == nullptr) {
        mSkinned = false;
        return;
    }
    mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(mesh->mBoundingSphere);
    const RndVertexInterpreter* layout = RndVertexInterpreter::GetInstance(mesh->_GetVertexTypeImpl());
    mSkinned = layout->mAttributes[RndVertexInterpreter::kWeightsAttribute].mOffset != -1
        && layout->mAttributes[RndVertexInterpreter::kBonesAttribute].mOffset != -1;
}

// Reconstructed from eboot.elf at 0x5C34C0. A different mesh already
// inlined under the path is removed first; the path survives the removal.
void RndMeshCom::SetInlineMesh(RndMesh* mesh) {
    SetResourceType(kResourceInline);
    EntityResource* entityResource = mObject->mEntity->mResource;
    ResourcePtr<RndMeshResource> inlined =
        entityResource->TryGetInline<RndMeshResource>(mMeshResourcePath);
    if (inlined != nullptr) {
        if (inlined->mMesh == mesh) {
            LoadResources(false);
            return;
        }
        const ResourcePath path = mMeshResourcePath;
        entityResource->UninlineResource(inlined);
        mMeshResourcePath = path;
    }
    if (mesh != nullptr) {
        RndMeshResource* resource = new RndMeshResource(mesh);
        resource->SetFile(mMeshResourcePath, false);
        entityResource->InlineResource(resource, mObject->mId.Layer());
    }
    LoadResources(false);
}

// Reconstructed from eboot.elf at 0x5C3630.
void RndMeshCom::SetUniqueMesh(RndMesh* mesh, ResourceType type) {
    SetResourceType(type);
    _SetMesh(mesh);
}

// Reconstructed from eboot.elf at 0x5C9BB0. The inline lookup comes first;
// a mesh resource held inline in its entity switches the type to inline.
bool RndMeshCom::_OnResourcesLoaded() {
    if (!RndDrawInstanceCom::_OnResourcesLoaded()) {
        return false;
    }
    if (mResourceType != kResourceUnique && mResourceType != kResourceUniqueExternal) {
        ResourcePtr<RndMeshResource> resource =
            mObject->mEntity->mResource->TryGetInline<RndMeshResource>(mMeshResourcePath);
        if (resource == nullptr) {
            resource = Resource::GetOrLoad<RndMeshResource>(mMeshResourcePath, false);
        }
        mMeshResource = std::move(resource);
        if (mMeshResource == nullptr || mMeshResource->Fail()) {
            if (mMesh != nullptr) {
                if (OwnsMesh(*this)) {
                    delete mMesh;
                    mMesh = nullptr;
                }
                mMesh = nullptr;
                mSkinned = false;
            }
        } else {
            RndMesh* mesh = mMeshResource->mMesh;
            if (mResourceUsage == kUsageUnique) {
                mesh = RndMeshUtl::Copy(*mesh, kVertexInvalid, true);
            }
            if (mMeshResource->mInlined) {
                mResourceType = kResourceInline;
            }
            _SetMesh(mesh);
        }
    }
    mSkeleton = mObject->mEntity->GetRoot()->GetCom<RndSkeletonPoseCom>();
    return true;
}

// Reconstructed from eboot.elf at 0x5C9E00.
void RndMeshCom::_Poll() {
    if (mSkinned && !mStaticBoundingSphere) {
        _PollSkinnedSphere();
    }
    RndDrawInstanceCom::_Poll();
    if (mPendingMeshSync == 0) {
        return;
    }
    if (mMesh != nullptr) {
        mMesh->mPendingSync.fetch_or(mPendingMeshSync);
        RndSceneDrawer* drawer = RndSceneDrawer::FindForEntity(mObject->mEntity);
        if (drawer != nullptr && mMesh->mMgr == nullptr) {
            drawer->mDynamicGpuDataMgr->Enqueue(*mMesh);
        }
        if ((mPendingMeshSync & 1) != 0) {
            mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(mMesh->mBoundingSphere);
        }
    }
    mPendingMeshSync = 0;
}

// Reconstructed from eboot.elf at 0x5CA020. A root bone index past the
// skeleton's transforms only builds the error name of a compiled-out warning.
void RndMeshCom::_PollSkinnedSphere() {
    if (mMesh == nullptr) {
        return;
    }
    Sphere sphere = mMesh->mBoundingSphere;
    if (mSkeleton != nullptr) {
        const unsigned long rootBone = static_cast<unsigned long>(mMesh->mGeometryFrame);
        if (rootBone < mSkeleton->mRenderXfms.size()) {
            Multiply(sphere, mSkeleton->mRenderXfms[rootBone], sphere);
        } else {
            MakeErrorName();
        }
    }
    mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(sphere);
}

// Reconstructed from eboot.elf at 0x5CA150.
void RndMeshCom::_EditPoll() {
    if (mPendingUsage != -1) {
        if (mResourceType != kResourceUnique && mResourceType != kResourceUniqueExternal
            && mMesh != nullptr) {
            if (mResourceUsage == kUsageUnique) {
                delete mMesh;
                mMesh = nullptr;
            }
            mMesh = nullptr;
            mSkinned = false;
        }
        mResourceUsage = static_cast<ResourceUsage>(mPendingUsage);
        mPendingUsage = -1;
        LoadResources(false);
    }
    RndMeshResource* resource = mMeshResource;
    if (resource != nullptr && (resource->mFileChangedOnDisk || resource->NeedsReload())) {
        LoadResources(false);
    }
    mSkeleton = mObject->mEntity->GetRoot()->GetCom<RndSkeletonPoseCom>();
    RndDrawInstanceCom::_EditPoll();
}

// Reconstructed from eboot.elf at 0x5CA260.
RndMaterialCom* RndMeshCom::_GetDefaultMaterial() const {
    return gRndDevice->mDefaults.mLitMaterial;
}

// Reconstructed from eboot.elf at 0x5CA280. The mesh's one instance draws
// the mesh with the skeleton's bone buffer.
void RndMeshCom::_InitDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) {
    RndDrawInstance& instance = *const_cast<RndDrawInstance*>(instances.mData);
    instance.mDrawable = mMesh;
    instance.mInstanceCBuffer =
        mSkinned && mSkeleton != nullptr ? mSkeleton->mBonesCBuffer : nullptr;
    _SyncInstanceStateFlags(instances);
}

// Reconstructed from eboot.elf at 0x5CA2D0. A hidden draw node only passes
// its show/hide flags on. The material is resolved as in
// _SyncInstanceStateFlags, and the instances take the object's world
// transform (transposed, with the translation in the last column), the draw
// node's inverse world rotation as their normal transform, its sphere, clip
// planes and flags, and the material's render state.
void RndMeshCom::_SyncDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) {
    RndDrawInstance* list = const_cast<RndDrawInstance*>(instances.mData);
    list[0].mDrawable = mMesh;
    const RndDrawNodeCom* drawNode = mRuntime.mDrawNode;
    const unsigned int hiddenFlags = drawNode->mRuntime.mWorldShowHideFlags;
    if ((hiddenFlags & 1) != 0) {
        for (unsigned long i = 0; i < instances.mSize; ++i) {
            list[i].mWorldShowHideFlags = hiddenFlags;
        }
        return;
    }
    RndDevice* device = gRndDevice;
    const int quality = static_cast<int>(device->mSettings->mQualityLevel);
    RndMaterialCom* material = mRuntime.mMaterial;
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
    const Transform& xfm = mRuntime.mTrans->mWorldXfm;
    const Vector3* axes[3] = {&xfm.m.x, &xfm.m.y, &xfm.m.z};
    for (unsigned long i = 0; i < instances.mSize; ++i) {
        RndDrawInstance& instance = list[i];
        float(&rows)[3][4] = instance.mInstanceData.mXfm;
        for (int axis = 0; axis < 3; ++axis) {
            rows[0][axis] = axes[axis]->x;
            rows[1][axis] = axes[axis]->y;
            rows[2][axis] = axes[axis]->z;
        }
        rows[0][3] = xfm.v.x;
        rows[1][3] = xfm.v.y;
        rows[2][3] = xfm.v.z;
        const Hmx::Matrix3& normal = drawNode->mRuntime.mInvWorldXfm.m;
        const Vector3* normalAxes[3] = {&normal.x, &normal.y, &normal.z};
        for (int row = 0; row < 3; ++row) {
            instance.mInstanceData.mNormalXfm[row][0] = normalAxes[row]->x;
            instance.mInstanceData.mNormalXfm[row][1] = normalAxes[row]->y;
            instance.mInstanceData.mNormalXfm[row][2] = normalAxes[row]->z;
        }
        instance.mSortBy = mSortBy;
        instance.mSortingHint =
            material->mBlendMode == RndBlendMode::kSource ? static_cast<signed char>(mSortingHint) : 0;
        instance.mDebugTag = mRuntime.mActiveSortKey;
        instance.mInstanceData.mPackedState = static_cast<unsigned int>(mBillboarding);
        const float* params[2] = {&mExtraData.x, &mExtraData1.x};
        for (int set = 0; set < 2; ++set) {
            for (int component = 0; component < 4; ++component) {
                instance.mInstanceData.mParams[set][component] = params[set][component];
            }
        }
        RndMaterialRuntimeData* data = material->mRuntimeData;
        data->mFrameStamp = device->mFrameCount;
        instance.mMaterial = data;
        instance.mUsesSceneTex = data->mRootFlags[1];
        instance.mUsesSceneDepth = data->mRootFlags[2];
        instance.mCullMode = material->mCullMode;
        instance.mReceiveAtmosphere = material->mReceiveAtmosphere;
        instance.mReceiveDecals = material->mReceiveDecals;
        const RndDrawNodeCom* node = mRuntime.mDrawNode;
        const unsigned int showHideFlags = node->mRuntime.mWorldShowHideFlags;
        instance.mStateFlags = data->mUsageHints[quality] | ((showHideFlags << 9) & 0x40000);
        instance.mWorldShowHideFlags = showHideFlags;
        instance.mCounterClockwise = node->mRuntime.mCounterClockwise;
        instance.mBounds = node->mRuntime.mWorldSphere;
        instance.mEnvironIndex = node->mRuntime.mEnvironIndex;
        instance.mClipPlanes[0] = node->mRuntime.mWorldClipPlanes[0];
        instance.mClipPlanes[1] = node->mRuntime.mWorldClipPlanes[1];
    }
}

// Reconstructed from eboot.elf at 0x5CA620.
Symbol RndMeshCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x5CA630.
Symbol RndMeshCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x5CA640.
int RndMeshCom::CurrentRev() const {
    return const_cast<RndMeshCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x5CA660.
bool RndMeshCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5CA690.
Component* RndMeshCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x5CA6A0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndMeshCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndMeshCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndMeshCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x5CA870.
PropRegistry& RndMeshCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x5CA880.
ComMetaData& RndMeshCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x405690. The binary emits the factory
// with the renderer's component registration.
Component* RndMeshCom::_Create() {
    return new RndMeshCom();
}
