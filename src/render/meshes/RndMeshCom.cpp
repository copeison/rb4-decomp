// render/RndMeshCom.o (0x5C3020 to 0x5CE6AF). The object's static
// initializer is at 0x5CE180.
#include "render/meshes/RndMeshCom.h"

#include <functional>
#include <new>
#include <utility>

#include "anim/skeleton/CharSkeleton.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "math/geometry/Geo.h"
#include "math/geometry/Sphere.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialReferenceCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/meshes/RndDynamicGpuData.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshResource.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/meshes/RndVertex.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/skeletons/RndSkeletonPoseCom.h"
#include "render/splines/RndSplineCom.h"
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

// The degree conversions of the sweep angle accessors.
constexpr float kRadiansToDegrees = 57.295776F;
constexpr float kDegreesToRadians = 0.017453292F;

// The mesh resource while it has not failed, or null. Inlined into the
// resource accessors. Name not in the reference map.
RndMeshResource* LoadedMeshResource(const PropAccessorArgs& args) {
    RndMeshResource* const resource = static_cast<RndMeshCom*>(args.mComponent)->mMeshResource.Get();
    if (resource == nullptr || resource->Fail()) {
        return nullptr;
    }
    return resource;
}

// The property accessors _Init installs (0x5CA9E0 to 0x5CB7BF). Names not
// in the reference map.
void SetResourceUsage(const PropAccessorArgs& args);
bool GetCompressVertices(const PropAccessorArgs& args);
void SetCompressVertices(const PropAccessorArgs& args);
bool GetSmoothTangents(const PropAccessorArgs& args);
void SetSmoothTangents(const PropAccessorArgs& args);
bool GetCalcUVScale(const PropAccessorArgs& args);
void SetCalcUVScale(const PropAccessorArgs& args);
bool GetCalcCurvature(const PropAccessorArgs& args);
void SetCalcCurvature(const PropAccessorArgs& args);
bool GetKeepMeshData(const PropAccessorArgs& args);
void SetKeepMeshData(const PropAccessorArgs& args);
unsigned int GetVertexUsage(const PropAccessorArgs& args);
void SetVertexUsage(const PropAccessorArgs& args);
unsigned int GetFaceUsage(const PropAccessorArgs& args);
void SetFaceUsage(const PropAccessorArgs& args);
int GetVertexType(const PropAccessorArgs& args);
unsigned long GetNumFaces(const PropAccessorArgs& args);
unsigned long GetNumVertices(const PropAccessorArgs& args);
unsigned long GetVerticesSize(const PropAccessorArgs& args);
Symbol GetRootBone(const PropAccessorArgs& args);
long GetRootBoneIndex(const PropAccessorArgs& args);
float GetExtentsWidth(const PropAccessorArgs& args);
float GetExtentsLength(const PropAccessorArgs& args);
float GetExtentsHeight(const PropAccessorArgs& args);
float GetTriangleWidth(const PropAccessorArgs& args);
void SetTriangleWidth(const PropAccessorArgs& args);
float GetCubeWidth(const PropAccessorArgs& args);
void SetCubeWidth(const PropAccessorArgs& args);
int GetCubeSubdivisions(const PropAccessorArgs& args);
void SetCubeSubdivisions(const PropAccessorArgs& args);
int GetBakeXfmMode(const PropAccessorArgs& args);
void SetBakeXfmMode(const PropAccessorArgs& args);
GameObjectId GetCombineWith(const PropAccessorArgs& args);
void SetCombineWith(const PropAccessorArgs& args);
bool GetCombineDeleteOriginal(const PropAccessorArgs& args);
void SetCombineDeleteOriginal(const PropAccessorArgs& args);
float GetTriangleHeight(const PropAccessorArgs& args);
void SetTriangleHeight(const PropAccessorArgs& args);
int GetQuadFacing(const PropAccessorArgs& args);
void SetQuadFacing(const PropAccessorArgs& args);
float GetQuadWidth(const PropAccessorArgs& args);
void SetQuadWidth(const PropAccessorArgs& args);
float GetQuadHeight(const PropAccessorArgs& args);
void SetQuadHeight(const PropAccessorArgs& args);
int GetQuadWidthSubdivisions(const PropAccessorArgs& args);
void SetQuadWidthSubdivisions(const PropAccessorArgs& args);
int GetQuadHeightSubdivisions(const PropAccessorArgs& args);
void SetQuadHeightSubdivisions(const PropAccessorArgs& args);
int GetTriangleFanFacing(const PropAccessorArgs& args);
void SetTriangleFanFacing(const PropAccessorArgs& args);
float GetTriangleFanRadius(const PropAccessorArgs& args);
void SetTriangleFanRadius(const PropAccessorArgs& args);
int GetTriangleFanNumSides(const PropAccessorArgs& args);
void SetTriangleFanNumSides(const PropAccessorArgs& args);
float GetBoxWidth(const PropAccessorArgs& args);
void SetBoxWidth(const PropAccessorArgs& args);
float GetBoxLength(const PropAccessorArgs& args);
void SetBoxLength(const PropAccessorArgs& args);
float GetBoxHeight(const PropAccessorArgs& args);
void SetBoxHeight(const PropAccessorArgs& args);
int GetBoxWidthSubdivisions(const PropAccessorArgs& args);
void SetBoxWidthSubdivisions(const PropAccessorArgs& args);
int GetBoxLengthSubdivisions(const PropAccessorArgs& args);
void SetBoxLengthSubdivisions(const PropAccessorArgs& args);
int GetBoxHeightSubdivisions(const PropAccessorArgs& args);
void SetBoxHeightSubdivisions(const PropAccessorArgs& args);
float GetSphereRadius(const PropAccessorArgs& args);
void SetSphereRadius(const PropAccessorArgs& args);
float GetSphereSweepAngle(const PropAccessorArgs& args);
void SetSphereSweepAngle(const PropAccessorArgs& args);
unsigned long GetSphereRadialSubdivisions(const PropAccessorArgs& args);
void SetSphereRadialSubdivisions(const PropAccessorArgs& args);
unsigned long GetSphereLatitudeSubdivisions(const PropAccessorArgs& args);
void SetSphereLatitudeSubdivisions(const PropAccessorArgs& args);
float GetCylinderRadius(const PropAccessorArgs& args);
void SetCylinderRadius(const PropAccessorArgs& args);
float GetCylinderHeight(const PropAccessorArgs& args);
void SetCylinderHeight(const PropAccessorArgs& args);
float GetCylinderSweepAngle(const PropAccessorArgs& args);
void SetCylinderSweepAngle(const PropAccessorArgs& args);
unsigned long GetCylinderRadialSubdivisions(const PropAccessorArgs& args);
void SetCylinderRadialSubdivisions(const PropAccessorArgs& args);
unsigned long GetCylinderHeightSubdivisions(const PropAccessorArgs& args);
void SetCylinderHeightSubdivisions(const PropAccessorArgs& args);
float GetCapsuleRadius(const PropAccessorArgs& args);
void SetCapsuleRadius(const PropAccessorArgs& args);
float GetCapsuleHeight(const PropAccessorArgs& args);
void SetCapsuleHeight(const PropAccessorArgs& args);
float GetCapsuleSweepAngle(const PropAccessorArgs& args);
void SetCapsuleSweepAngle(const PropAccessorArgs& args);
unsigned long GetCapsuleRadialSubdivisions(const PropAccessorArgs& args);
void SetCapsuleRadialSubdivisions(const PropAccessorArgs& args);
unsigned long GetCapsuleLatitudeSubdivisions(const PropAccessorArgs& args);
void SetCapsuleLatitudeSubdivisions(const PropAccessorArgs& args);
unsigned long GetCapsuleHeightSubdivisions(const PropAccessorArgs& args);
void SetCapsuleHeightSubdivisions(const PropAccessorArgs& args);
float GetConeRadius(const PropAccessorArgs& args);
void SetConeRadius(const PropAccessorArgs& args);
float GetConeHeight(const PropAccessorArgs& args);
void SetConeHeight(const PropAccessorArgs& args);
float GetConeSweepAngle(const PropAccessorArgs& args);
void SetConeSweepAngle(const PropAccessorArgs& args);
unsigned long GetConeRadialSubdivisions(const PropAccessorArgs& args);
void SetConeRadialSubdivisions(const PropAccessorArgs& args);
unsigned long GetConeHeightSubdivisions(const PropAccessorArgs& args);
void SetConeHeightSubdivisions(const PropAccessorArgs& args);
float GetTorusPrimaryRadius(const PropAccessorArgs& args);
void SetTorusPrimaryRadius(const PropAccessorArgs& args);
float GetTorusSecondaryRadius(const PropAccessorArgs& args);
void SetTorusSecondaryRadius(const PropAccessorArgs& args);
float GetTorusSweepAngle(const PropAccessorArgs& args);
void SetTorusSweepAngle(const PropAccessorArgs& args);
unsigned long GetTorusRadialSubdivisions(const PropAccessorArgs& args);
void SetTorusRadialSubdivisions(const PropAccessorArgs& args);
unsigned long GetTorusLatitudeSubdivisions(const PropAccessorArgs& args);
void SetTorusLatitudeSubdivisions(const PropAccessorArgs& args);
float GetIcosahedronRadius(const PropAccessorArgs& args);
void SetIcosahedronRadius(const PropAccessorArgs& args);
GameObjectId GetSplineFanSpline(const PropAccessorArgs& args);
void SetSplineFanSpline(const PropAccessorArgs& args);
unsigned long GetSplineFanSectionsPerKnotSpan(const PropAccessorArgs& args);
void SetSplineFanSectionsPerKnotSpan(const PropAccessorArgs& args);
Symbol GetExportFilepath(const PropAccessorArgs& args);
void SetExportFilepath(const PropAccessorArgs& args);

}  // namespace

// The object's statics, in the order of its static initializer. The
// remaining ones are the values the "utilities" properties edit and their
// actions build from: one parameter block per mesh builder.
Symbol RndMeshCom::sId("Mesh");
Symbol RndMeshCom::sClassName("Mesh");
PropRegistry RndMeshCom::sPropRegistry;
ComMetaData RndMeshCom::sMetaData;
Symbol RndMeshCom::kSubObjectResource("mesh");

namespace {

// The utilities' values. Names not in the reference map.
// "combine": the object to take the geometry from, and whether to delete
// it.
GameObjectId gCombineWith = {0xFFFFFFFFU};  // 0x1AA6040
RndMeshUtl::CreateTriangleParams gMakeTriangleParams;  // 0x1AA6048
RndMeshUtl::CreateFacingQuadParams gMakeQuadParams = [] {  // 0x1AA60C0
    RndMeshUtl::CreateFacingQuadParams params;
    params.mFacing = 2;
    params.mWidth = 1.0F;
    params.mHeight = 1.0F;
    params.mNumSegmentsU = 1;
    params.mNumSegmentsV = 1;
    return params;
}();
RndMeshUtl::CreateTriangleFanParams gMakeTriangleFanParams = [] {  // 0x1AA60F8
    RndMeshUtl::CreateTriangleFanParams params;
    params.mFacing = 2;
    params.mNumSegments = 5;
    params.mRadius = 1.0F;
    return params;
}();
// A unit box, for "make_cube" and "make_box". Inlined into the static
// initializer. Name not in the reference map.
RndMeshUtl::CreateBoxParams MakeUnitBoxParams() {
    RndMeshUtl::CreateBoxParams params;
    params.mAxisX = {1.0F, 0.0F, 0.0F};
    params.mAxisY = {0.0F, 1.0F, 0.0F};
    params.mAxisZ = {0.0F, 0.0F, 1.0F};
    params.mNumSegmentsX = 1;
    params.mNumSegmentsY = 1;
    params.mNumSegmentsZ = 1;
    return params;
}
RndMeshUtl::CreateBoxParams gMakeCubeParams = MakeUnitBoxParams();  // 0x1AA6130
RndMeshUtl::CreateBoxParams gMakeBoxParams = MakeUnitBoxParams();   // 0x1AA6188
RndMeshUtl::CreateSphereParams gMakeSphereParams = [] {  // 0x1AA61E0
    RndMeshUtl::CreateSphereParams params;
    params.mRadius = 1.0F;
    params.mNumRings = 16;
    return params;
}();
RndMeshUtl::CreateCylinderParams gMakeCylinderParams = [] {  // 0x1AA6238
    RndMeshUtl::CreateCylinderParams params;
    params.mRadius = 1.0F;
    params.mHeight = 2.0F;
    params.mNumHeightSegments = 1;
    return params;
}();
RndMeshUtl::CreateCapsuleParams gMakeCapsuleParams = [] {  // 0x1AA6290
    RndMeshUtl::CreateCapsuleParams params;
    params.mRadius = 1.0F;
    params.mLength = 2.0F;
    params.mNumCapSegments = 8;
    params.mNumSideSegments = 1;
    return params;
}();
RndMeshUtl::CreateConeParams gMakeConeParams = [] {  // 0x1AA62F0
    RndMeshUtl::CreateConeParams params;
    params.mRadius = 1.0F;
    params.mHeight = 2.0F;
    params.mNumHeightSegments = 1;
    return params;
}();
RndMeshUtl::CreateTorusParams gMakeTorusParams = [] {  // 0x1AA6348
    RndMeshUtl::CreateTorusParams params;
    params.mPrimaryRadius = 1.0F;
    params.mSecondaryRadius = 0.25F;
    params.mNumLatitudeSegments = 32;
    return params;
}();
RndMeshUtl::CreateIcosahedronParams gMakeIcosahedronParams = [] {  // 0x1AA63A0
    RndMeshUtl::CreateIcosahedronParams params;
    params.mRadius = 1.0F;
    return params;
}();
RndMeshUtl::CreateSplineFanParams gMakeSplineFanParams = [] {  // 0x1AA63C8
    RndMeshUtl::CreateSplineFanParams params;
    params.mSectionsPerKnotSpan = 1;
    return params;
}();
// "make_spline_fan"'s spline object.
GameObjectId gMakeSplineFanSpline = {0xFFFFFFFFU};  // 0x1AA63F8
// "export_to_fbx"'s file.
Symbol gExportFilepath;  // 0x1AA6400
// "combine"'s "delete_original".
bool gCombineDeleteOriginal;  // 0x1AA6408
// "bake_xfm"'s mode, in the data section.
RndMeshUtl::BakeXfmMode gBakeXfmMode = RndMeshUtl::kBakeXfmFull;  // 0x19BDCC8

}  // namespace

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

// Reconstructed from eboot.elf at 0x5C3660. Revision 1 fixes saved mesh
// paths into sub-object paths. The resource's build options are read and
// written through the resource, the mesh's statistics through the mesh, and
// the "utilities" edit the parameter blocks above; each utility's action
// builds an inline mesh and is read-only while the component is driven.
// Every property registration builds a "prop" symbol it does not use, the
// struct and action registrations "struct" and "action". The lambdas'
// bodies are at 0x5CB840 to 0x5CE0E0.
void RndMeshCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndDrawInstanceCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "An instance of a rendered triangle mesh";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mLightweight = true;
    metadata.mAliases.push_back(Symbol("RndMeshCom"));
    RndMeshCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev <= 0) {
                static_cast<RndMeshCom*>(args.mComponent)->mMeshResourcePath.FixSubObjectPath(
                    kSubObjectResource);
            }
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    ResourcePathMetadata& path = TypeSpecificMetadata(
        registry.RegisterProp(
            "mesh_resource_path", offsetof(RndMeshCom, mMeshResourcePath), kPropertyResourcePath, 8),
        prototype.mMeshResourcePath);
    path.mDefault = prototype.mMeshResourcePath;
    static_cast<void>(Symbol("prop"));
    path.mResourceId = RndMeshResource::Id();
    path.mNoTest = true;
    path.mHelp = String(
        "The mesh resource that this component should use.\n\nNOTE: You can only remap inline mesh "
        "resources! FBX files cannot be selected or remapped this way. You need to have file-based mesh "
        "edits in the FBX file or re-import FBX.");

    IntMetadata<int>& type = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "mesh_resource_type", offsetof(RndMeshCom, mResourceType), kPropertyInt, 4).mMetadata.Get());
    type.mDefault = prototype.mResourceType;
    static_cast<void>(Symbol("prop"));
    type.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kResourceFile, String("File"), String("Mesh is a file-based resource")},
        AllowedValue<int>{kResourceInline, String("Inline"), String("Mesh is an inline resource")},
        AllowedValue<int>{
            kResourceUnique, String("Unique"), String("Mesh is not a resource (I manage its lifetime)")},
        AllowedValue<int>{
            kResourceUniqueExternal,
            String("Unique (External)"),
            String("Mesh is not a resource (someone external manages its lifetime)")}};
    type.mReadOnly = true;

    IntMetadata<int>& usage = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "mesh_resource_usage", offsetof(RndMeshCom, mResourceUsage), kPropertyInt, 4).mMetadata.Get());
    usage.mDefault = prototype.mResourceUsage;
    static_cast<void>(Symbol("prop"));
    usage.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kUsageShare, String("Share"), String("Shares the same mesh geometry with other instances")},
        AllowedValue<int>{
            kUsageUnique,
            String("Copy"),
            String(
                "Makes a full copy of the mesh geometry (eg. useful for mesh deformation, but means you "
                "can't batch it)")}};
    usage.mNoTest = true;
    usage.mSetter = SetResourceUsage;
    usage.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshCom& com = static_cast<const RndMeshCom&>(args.mComponent);
        return com.mResourceType == kResourceUnique || com.mResourceType == kResourceUniqueExternal;
    });

    GroupMetadata& resourceProperties = static_cast<GroupMetadata&>(
        *registry.RegisterProp("mesh_resource_properties", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& resourceRegistry = resourceProperties.mRegistry;
    resourceProperties.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshResource* const resource =
            static_cast<const RndMeshCom&>(args.mComponent).mMeshResource.Get();
        return resource == nullptr || resource->Fail();
    });
    resourceProperties.mHelp = String(
        "Properties which live on the mesh resource.  History does not work for these properties.  For "
        "file-based resoures (eg. FBX) the settings live in a separate DTA file.");

    const bool compressType = false;
    BoolMetadata& compress = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("compress_vertices", -1, kPropertyBool, 0), compressType);
    static_cast<void>(Symbol("prop"));
    compress.mHelp = String("Should mesh vertices be compressed?");
    compress.mGetter = GetCompressVertices;
    compress.mSetter = SetCompressVertices;
    compress.mDefault = true;
    compress.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshResource* const resource =
            static_cast<const RndMeshCom&>(args.mComponent).mMeshResource.Get();
        if (resource == nullptr || resource->Fail()) {
            return true;
        }
        const RndVertexType vertexType = resource->mMesh->_GetVertexTypeImpl();
        if (RndVertex::IsTypeCompressed(vertexType)) {
            return false;
        }
        return !RndVertex::CanCompressType(vertexType);
    });

    const bool smoothType = false;
    BoolMetadata& smooth = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("smooth_tangents", -1, kPropertyBool, 0), smoothType);
    static_cast<void>(Symbol("prop"));
    smooth.mHelp = String("Should we smooth mesh tangents/bitangents?");
    smooth.mGetter = GetSmoothTangents;
    smooth.mSetter = SetSmoothTangents;
    smooth.mDefault = true;
    smooth.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshResource* const resource =
            static_cast<const RndMeshCom&>(args.mComponent).mMeshResource.Get();
        return resource == nullptr || resource->Fail();
    });

    const bool uvScaleType = false;
    BoolMetadata& uvScale = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("calc_uv_scale", -1, kPropertyBool, 0), uvScaleType);
    static_cast<void>(Symbol("prop"));
    uvScale.mHelp = String(
        "Should we compute vertex UV scale? (stored in vertex color red channel; eg. needed for skin "
        "shader)");
    uvScale.mGetter = GetCalcUVScale;
    uvScale.mSetter = SetCalcUVScale;
    uvScale.mDefault = true;
    uvScale.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshResource* const resource =
            static_cast<const RndMeshCom&>(args.mComponent).mMeshResource.Get();
        return resource == nullptr || resource->Fail();
    });

    const bool curvatureType = false;
    BoolMetadata& curvature = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("calc_curvature", -1, kPropertyBool, 0), curvatureType);
    static_cast<void>(Symbol("prop"));
    curvature.mHelp = String(
        "Should we compute vertex curvature? (stored in vertex color green channel; eg. needed for skin "
        "shader)");
    curvature.mGetter = GetCalcCurvature;
    curvature.mSetter = SetCalcCurvature;
    curvature.mDefault = true;
    curvature.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshResource* const resource =
            static_cast<const RndMeshCom&>(args.mComponent).mMeshResource.Get();
        return resource == nullptr || resource->Fail();
    });

    const bool keepType = false;
    BoolMetadata& keep = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("keep_mesh_data", -1, kPropertyBool, 0), keepType);
    static_cast<void>(Symbol("prop"));
    keep.mHelp = String(
        "Should mesh vertex/face data be kept after syncing its Gpu counterpart? (this requires more "
        "memory, but allows access to the mesh data at game runtime).");
    keep.mGetter = GetKeepMeshData;
    keep.mSetter = SetKeepMeshData;

    const unsigned int vertexUsageType = 0;
    IntMetadata<unsigned int>& vertexUsage = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("vertex_usage", -1, kPropertyUInt, 0), vertexUsageType);
    static_cast<void>(Symbol("prop"));
    vertexUsage.mHelp = String("Additional mesh vertex usage hints");
    vertexUsage.mBitfield = true;
    vertexUsage.mAllowedValues = eastl::vector<AllowedValue<unsigned int>>{
        AllowedValue<unsigned int>{
            1, String("Dynamic (Cpu)"), String("Allows dynamic updating of vertex data on the Cpu")},
        AllowedValue<unsigned int>{
            2, String("Dynamic (Gpu)"), String("Allows dynamic updating of vertex data on the Gpu")}};
    vertexUsage.mGetter = GetVertexUsage;
    vertexUsage.mSetter = SetVertexUsage;

    const unsigned int faceUsageType = 0;
    IntMetadata<unsigned int>& faceUsage = TypeSpecificMetadata(
        resourceRegistry.RegisterProp("face_usage", -1, kPropertyUInt, 0), faceUsageType);
    static_cast<void>(Symbol("prop"));
    faceUsage.mHelp = String("Additional mesh face usage hints");
    faceUsage.mReadOnly = true;
    faceUsage.mBitfield = true;
    faceUsage.mAllowedValues = eastl::vector<AllowedValue<unsigned int>>{
        AllowedValue<unsigned int>{
            1, String("Dynamic (Cpu)"), String("Allows dynamic updating of vertex data on the Cpu")},
        AllowedValue<unsigned int>{
            2, String("Dynamic (Gpu)"), String("Allows dynamic updating of vertex data on the Gpu")}};
    faceUsage.mGetter = GetFaceUsage;
    faceUsage.mSetter = SetFaceUsage;

    BoolMetadata& driven = TypeSpecificMetadata(
        registry.RegisterProp("driven", offsetof(RndMeshCom, mDriven), kPropertyBool, 1), prototype.mDriven);
    driven.mDefault = prototype.mDriven;
    static_cast<void>(Symbol("prop"));
    driven.mReadOnly = true;

    BoolMetadata& staticSphere = TypeSpecificMetadata(
        registry.RegisterProp(
            "static_bounding_sphere", offsetof(RndMeshCom, mStaticBoundingSphere), kPropertyBool, 1),
        prototype.mStaticBoundingSphere);
    staticSphere.mDefault = prototype.mStaticBoundingSphere;
    static_cast<void>(Symbol("prop"));
    staticSphere.mHelp = String("Don't update the bounding sphere, even if skinned");

    const int vertexTypeType = 0;
    IntMetadata<int>& vertexType =
        TypeSpecificMetadata(registry.RegisterProp("vertex_type", -1, kPropertyInt, 0), vertexTypeType);
    static_cast<void>(Symbol("prop"));
    vertexType.mReadOnly = true;
    vertexType.mDebug = true;
    eastl::vector<AllowedValue<int>> vertexTypes;
    vertexTypes.reserve(9);
    for (int value = kVertexInvalid; value != kVertexSkinnedCompressed + 1; ++value) {
        vertexTypes.push_back(AllowedValue<int>{
            value, String(RndVertex::GetTypeName(static_cast<RndVertexType>(value))), String("")});
    }
    vertexType.mAllowedValues = vertexTypes;
    vertexType.mGetter = GetVertexType;

    const unsigned long numFacesType = 0;
    IntMetadata<unsigned long>& numFaces =
        TypeSpecificMetadata(registry.RegisterProp("num_faces", -1, kPropertyUInt64, 0), numFacesType);
    static_cast<void>(Symbol("prop"));
    numFaces.mReadOnly = true;
    numFaces.mDebug = true;
    numFaces.mGetter = GetNumFaces;

    const unsigned long numVerticesType = 0;
    IntMetadata<unsigned long>& numVertices = TypeSpecificMetadata(
        registry.RegisterProp("num_vertices", -1, kPropertyUInt64, 0), numVerticesType);
    static_cast<void>(Symbol("prop"));
    numVertices.mReadOnly = true;
    numVertices.mDebug = true;
    numVertices.mGetter = GetNumVertices;

    const unsigned long verticesSizeType = 0;
    IntMetadata<unsigned long>& verticesSize = TypeSpecificMetadata(
        registry.RegisterProp("vertices_size", -1, kPropertyUInt64, 0), verticesSizeType);
    static_cast<void>(Symbol("prop"));
    verticesSize.mReadOnly = true;
    verticesSize.mDebug = true;
    verticesSize.mUnits = Symbol("KB");
    verticesSize.mGetter = GetVerticesSize;

    const Symbol rootBoneType;
    SymbolMetadata& rootBone =
        TypeSpecificMetadata(registry.RegisterProp("root_bone", -1, kPropertySymbol, 0), rootBoneType);
    static_cast<void>(Symbol("prop"));
    rootBone.mReadOnly = true;
    rootBone.mDebug = true;
    rootBone.mGetter = GetRootBone;

    const long rootBoneIndexType = 0;
    IntMetadata<long>& rootBoneIndex = TypeSpecificMetadata(
        registry.RegisterProp("root_bone_index", -1, kPropertyInt64, 0), rootBoneIndexType);
    static_cast<void>(Symbol("prop"));
    rootBoneIndex.mReadOnly = true;
    rootBoneIndex.mDebug = true;
    rootBoneIndex.mGetter = GetRootBoneIndex;

    GroupMetadata& extents = static_cast<GroupMetadata&>(
        *registry.RegisterProp("extents", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& extentsRegistry = extents.mRegistry;
    extents.mKind = 1;
    extents.mReadOnly = true;
    extents.mDebug = true;
    const float widthType = 0.0F;
    FloatMetadata& width =
        TypeSpecificMetadata(extentsRegistry.RegisterProp("width", -1, kPropertyFloat, 0), widthType);
    static_cast<void>(Symbol("prop"));
    width.mGetter = GetExtentsWidth;
    const float lengthType = 0.0F;
    FloatMetadata& length =
        TypeSpecificMetadata(extentsRegistry.RegisterProp("length", -1, kPropertyFloat, 0), lengthType);
    static_cast<void>(Symbol("prop"));
    length.mGetter = GetExtentsLength;
    const float heightType = 0.0F;
    FloatMetadata& height =
        TypeSpecificMetadata(extentsRegistry.RegisterProp("height", -1, kPropertyFloat, 0), heightType);
    static_cast<void>(Symbol("prop"));
    height.mGetter = GetExtentsHeight;

    gMakeQuadParams.mFacing = 4;
    PropRegistry& utilities = static_cast<GroupMetadata&>(
        *registry.RegisterProp("utilities", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));

    GroupMetadata& bakeXfm = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("bake_xfm", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    bakeXfm.mHelp = String("Form the mesh into a triangle");
    PropRegistry& bakeXfmRegistry = bakeXfm.mRegistry;
    IntMetadata<int>& bakeMode = static_cast<IntMetadata<int>&>(
        *bakeXfmRegistry.RegisterProp("mode", -1, kPropertyInt, 0).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    bakeMode.mGetter = GetBakeXfmMode;
    bakeMode.mSetter = SetBakeXfmMode;
    bakeMode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{RndMeshUtl::kBakeXfmScale, String("Scale"), String("Only bake scale")},
        AllowedValue<int>{
            RndMeshUtl::kBakeXfmRotationAndScale,
            String("Rotation and Scale"),
            String("Bake rotation and scale")},
        AllowedValue<int>{
            RndMeshUtl::kBakeXfmFull,
            String("Full Xfm"),
            String("Bake the full transform (including position)")}};
    ActionMetadata& bakeXfmAction =
        static_cast<ActionMetadata&>(*bakeXfmRegistry.RegisterAction("bake_xfm").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    bakeXfmAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        RndMeshUtl::BakeXfm(com, gBakeXfmMode);
    };
    bakeXfm.mReadOnly._Set([](const TagArgs& args) {
        const RndMeshCom& com = static_cast<const RndMeshCom&>(args.mComponent);
        if (com.mMesh != nullptr && com.mResourceType != kResourceFile) {
            return com.mDriven;
        }
        return true;
    });

    GroupMetadata& combine = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("combine", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    combine.mHelp = String("Combine with geometry from another mesh");
    PropRegistry& combineRegistry = combine.mRegistry;
    const GameObjectId withType = {0xFFFFFFFFU};
    GameObjectIdMetadata& with =
        TypeSpecificMetadata(combineRegistry.RegisterProp("with", -1, kPropertyGameObjectId, 0), withType);
    static_cast<void>(Symbol("prop"));
    with.mRequiredComs = {sId};
    with.mGetter = GetCombineWith;
    with.mSetter = SetCombineWith;
    const bool deleteOriginalType = false;
    BoolMetadata& deleteOriginal = TypeSpecificMetadata(
        combineRegistry.RegisterProp("delete_original", -1, kPropertyBool, 0), deleteOriginalType);
    static_cast<void>(Symbol("prop"));
    deleteOriginal.mHelp = String("Should the mesh we're combining geometry from be destroyed?");
    deleteOriginal.mGetter = GetCombineDeleteOriginal;
    deleteOriginal.mSetter = SetCombineDeleteOriginal;
    ActionMetadata& combineAction =
        static_cast<ActionMetadata&>(*combineRegistry.RegisterAction("combine").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    combineAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        if (gCombineWith.mId == gNullObjectId.mId) {
            return;
        }
        const GameObject* const object = com.mObject->mEntity->GetObject(gCombineWith);
        if (object == nullptr) {
            return;
        }
        if (RndMeshCom* const other = object->GetCom<RndMeshCom>()) {
            RndMeshUtl::Combine(com, *other, gCombineDeleteOriginal, true);
        }
    };
    combineAction.mReadOnly._Set([](const TagArgs& args) {
        if (gCombineWith.mId != gNullObjectId.mId) {
            const RndMeshCom& com = static_cast<const RndMeshCom&>(args.mComponent);
            if (com.mMesh != nullptr && com.mResourceType != kResourceFile) {
                return com.mDriven;
            }
        }
        return true;
    });

    GroupMetadata& makeTriangle = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_triangle", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeTriangle.mHelp = String("Form the mesh into a triangle");
    PropRegistry& makeTriangleRegistry = makeTriangle.mRegistry;
    const float triangleWidthType = 0.0F;
    FloatMetadata& triangleWidth = TypeSpecificMetadata(
        makeTriangleRegistry.RegisterProp("width", -1, kPropertyFloat, 0), triangleWidthType);
    static_cast<void>(Symbol("prop"));
    triangleWidth.mGetter = GetTriangleWidth;
    triangleWidth.mSetter = SetTriangleWidth;
    triangleWidth.mMin = 0.0F;
    triangleWidth.mDefault = RndMeshUtl::CreateTriangleParams().mPoints[0].x;
    triangleWidth.mHelp = String("Width of the resulting triangle (in X axis)");
    const float triangleHeightType = 0.0F;
    FloatMetadata& triangleHeight = TypeSpecificMetadata(
        makeTriangleRegistry.RegisterProp("height", -1, kPropertyFloat, 0), triangleHeightType);
    static_cast<void>(Symbol("prop"));
    triangleHeight.mGetter = GetTriangleHeight;
    triangleHeight.mSetter = SetTriangleHeight;
    triangleHeight.mMin = 0.0F;
    triangleHeight.mDefault = RndMeshUtl::CreateTriangleParams().mPoints[1].y;
    triangleHeight.mHelp = String("Width of the resulting triangle (in Y axis)");
    ActionMetadata& makeTriangleAction =
        static_cast<ActionMetadata&>(*makeTriangleRegistry.RegisterAction("make_triangle").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeTriangleAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateTriangle(gMakeTriangleParams));
    };
    makeTriangle.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeQuad = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_quad", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeQuad.mHelp = String("Form the mesh into a quad");
    PropRegistry& makeQuadRegistry = makeQuad.mRegistry;
    IntMetadata<int>& quadFacing = static_cast<IntMetadata<int>&>(
        *makeQuadRegistry.RegisterProp("facing", -1, kPropertyInt, 0).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    quadFacing.mGetter = GetQuadFacing;
    quadFacing.mSetter = SetQuadFacing;
    quadFacing.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("+X"), String("")},
        AllowedValue<int>{1, String("+Y"), String("")},
        AllowedValue<int>{2, String("+Z"), String("")},
        AllowedValue<int>{3, String("-X"), String("")},
        AllowedValue<int>{4, String("-Y (use for billboarding)"), String("")},
        AllowedValue<int>{5, String("-Z"), String("")}};
    quadFacing.mDefault = 4;
    quadFacing.mHelp = String("Facing direction of the resulting quad");
    const float quadWidthType = 0.0F;
    FloatMetadata& quadWidth =
        TypeSpecificMetadata(makeQuadRegistry.RegisterProp("width", -1, kPropertyFloat, 0), quadWidthType);
    static_cast<void>(Symbol("prop"));
    quadWidth.mGetter = GetQuadWidth;
    quadWidth.mSetter = SetQuadWidth;
    quadWidth.mMin = 0.0F;
    quadWidth.mDefault = 1.0F;
    quadWidth.mHelp = String("Width of the resulting quad");
    const float quadHeightType = 0.0F;
    FloatMetadata& quadHeight =
        TypeSpecificMetadata(makeQuadRegistry.RegisterProp("height", -1, kPropertyFloat, 0), quadHeightType);
    static_cast<void>(Symbol("prop"));
    quadHeight.mGetter = GetQuadHeight;
    quadHeight.mSetter = SetQuadHeight;
    quadHeight.mMin = 0.0F;
    quadHeight.mDefault = 1.0F;
    quadHeight.mHelp = String("height of the resulting quad");
    const int quadWidthSubdivisionsType = 0;
    IntMetadata<int>& quadWidthSubdivisions =
        TypeSpecificMetadata(makeQuadRegistry.RegisterProp("width_subdivisions", -1, kPropertyInt, 0), quadWidthSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    quadWidthSubdivisions.mGetter = GetQuadWidthSubdivisions;
    quadWidthSubdivisions.mSetter = SetQuadWidthSubdivisions;
    quadWidthSubdivisions.mMin = 1;
    quadWidthSubdivisions.mDefault = 1;
    quadWidthSubdivisions.mHelp = String("Number of width subdivsions");
    const int quadHeightSubdivisionsType = 0;
    IntMetadata<int>& quadHeightSubdivisions =
        TypeSpecificMetadata(makeQuadRegistry.RegisterProp("height_subdivisions", -1, kPropertyInt, 0), quadHeightSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    quadHeightSubdivisions.mGetter = GetQuadHeightSubdivisions;
    quadHeightSubdivisions.mSetter = SetQuadHeightSubdivisions;
    quadHeightSubdivisions.mMin = 1;
    quadHeightSubdivisions.mDefault = 1;
    quadHeightSubdivisions.mHelp = String("Number of height subdivsions");
    ActionMetadata& makeQuadAction =
        static_cast<ActionMetadata&>(*makeQuadRegistry.RegisterAction("make_quad").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeQuadAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateFacingQuad(gMakeQuadParams));
    };
    makeQuad.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeTriangleFan = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_triangle_fan", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeTriangleFan.mHelp = String("Form the mesh into a triangle fan");
    PropRegistry& makeTriangleFanRegistry = makeTriangleFan.mRegistry;
    IntMetadata<int>& fanFacing = static_cast<IntMetadata<int>&>(
        *makeTriangleFanRegistry.RegisterProp("facing", -1, kPropertyInt, 0).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    fanFacing.mGetter = GetTriangleFanFacing;
    fanFacing.mSetter = SetTriangleFanFacing;
    fanFacing.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("+X"), String("")},
        AllowedValue<int>{1, String("+Y"), String("")},
        AllowedValue<int>{2, String("+Z"), String("")},
        AllowedValue<int>{3, String("-X"), String("")},
        AllowedValue<int>{4, String("-Y (use for billboarding)"), String("")},
        AllowedValue<int>{5, String("-Z"), String("")}};
    fanFacing.mDefault = 4;
    fanFacing.mHelp = String("Facing direction of the resulting triangle fan");
    const float fanRadiusType = 0.0F;
    FloatMetadata& fanRadius =
        TypeSpecificMetadata(makeTriangleFanRegistry.RegisterProp("radius", -1, kPropertyFloat, 0), fanRadiusType);
    static_cast<void>(Symbol("prop"));
    fanRadius.mGetter = GetTriangleFanRadius;
    fanRadius.mSetter = SetTriangleFanRadius;
    fanRadius.mMin = 0.0F;
    fanRadius.mDefault = 1.0F;
    fanRadius.mHelp = String("Radius of the resulting triangle fan");
    const int fanNumSidesType = 0;
    IntMetadata<int>& fanNumSides =
        TypeSpecificMetadata(makeTriangleFanRegistry.RegisterProp("num_sides", -1, kPropertyInt, 0), fanNumSidesType);
    static_cast<void>(Symbol("prop"));
    fanNumSides.mGetter = GetTriangleFanNumSides;
    fanNumSides.mSetter = SetTriangleFanNumSides;
    fanNumSides.mMin = 3;
    fanNumSides.mDefault = 5;
    fanNumSides.mHelp = String("Number of sides for the resulting polygon");
    ActionMetadata& makeTriangleFanAction =
        static_cast<ActionMetadata&>(*makeTriangleFanRegistry.RegisterAction("make_triangle_fan").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeTriangleFanAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateTriangleFan(gMakeTriangleFanParams));
    };
    makeTriangleFan.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeCube = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_cube", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeCube.mHelp = String("Form the mesh into a cube");
    PropRegistry& makeCubeRegistry = makeCube.mRegistry;
    const float cubeWidthType = 0.0F;
    FloatMetadata& cubeWidth =
        TypeSpecificMetadata(makeCubeRegistry.RegisterProp("width", -1, kPropertyFloat, 0), cubeWidthType);
    static_cast<void>(Symbol("prop"));
    cubeWidth.mGetter = GetCubeWidth;
    cubeWidth.mSetter = SetCubeWidth;
    cubeWidth.mMin = 0.0F;
    cubeWidth.mDefault = 1.0F;
    cubeWidth.mHelp = String("Width of the resulting cube");
    const int cubeSubdivisionsType = 0;
    IntMetadata<int>& cubeSubdivisions =
        TypeSpecificMetadata(makeCubeRegistry.RegisterProp("subdivisions", -1, kPropertyInt, 0), cubeSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    cubeSubdivisions.mGetter = GetCubeSubdivisions;
    cubeSubdivisions.mSetter = SetCubeSubdivisions;
    cubeSubdivisions.mMin = 1;
    cubeSubdivisions.mDefault = 1;
    cubeSubdivisions.mHelp = String("Number of subdivsions (along each edge)");
    ActionMetadata& makeCubeAction =
        static_cast<ActionMetadata&>(*makeCubeRegistry.RegisterAction("make_cube").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeCubeAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateBox(gMakeCubeParams));
    };
    makeCube.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeBox = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_box", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeBox.mHelp = String("Form the mesh into a box");
    PropRegistry& makeBoxRegistry = makeBox.mRegistry;
    const float boxWidthType = 0.0F;
    FloatMetadata& boxWidth =
        TypeSpecificMetadata(makeBoxRegistry.RegisterProp("width", -1, kPropertyFloat, 0), boxWidthType);
    static_cast<void>(Symbol("prop"));
    boxWidth.mGetter = GetBoxWidth;
    boxWidth.mSetter = SetBoxWidth;
    boxWidth.mMin = 0.0F;
    boxWidth.mDefault = 1.0F;
    boxWidth.mHelp = String("Width of the resulting box (in X axis)");
    const float boxLengthType = 0.0F;
    FloatMetadata& boxLength =
        TypeSpecificMetadata(makeBoxRegistry.RegisterProp("length", -1, kPropertyFloat, 0), boxLengthType);
    static_cast<void>(Symbol("prop"));
    boxLength.mGetter = GetBoxLength;
    boxLength.mSetter = SetBoxLength;
    boxLength.mMin = 0.0F;
    boxLength.mDefault = 0.0F;
    boxLength.mHelp = String("length of the resulting box (in Y axis)");
    const float boxHeightType = 0.0F;
    FloatMetadata& boxHeight =
        TypeSpecificMetadata(makeBoxRegistry.RegisterProp("height", -1, kPropertyFloat, 0), boxHeightType);
    static_cast<void>(Symbol("prop"));
    boxHeight.mGetter = GetBoxHeight;
    boxHeight.mSetter = SetBoxHeight;
    boxHeight.mMin = 0.0F;
    boxHeight.mDefault = 1.0F;
    boxHeight.mHelp = String("height of the resulting box (in Z axis)");
    const int boxWidthSubdivisionsType = 0;
    IntMetadata<int>& boxWidthSubdivisions =
        TypeSpecificMetadata(makeBoxRegistry.RegisterProp("width_subdivisions", -1, kPropertyInt, 0), boxWidthSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    boxWidthSubdivisions.mGetter = GetBoxWidthSubdivisions;
    boxWidthSubdivisions.mSetter = SetBoxWidthSubdivisions;
    boxWidthSubdivisions.mMin = 1;
    boxWidthSubdivisions.mDefault = 1;
    boxWidthSubdivisions.mHelp = String("Number of width subdivsions (along X axis)");
    const int boxLengthSubdivisionsType = 0;
    IntMetadata<int>& boxLengthSubdivisions =
        TypeSpecificMetadata(makeBoxRegistry.RegisterProp("length_subdivisions", -1, kPropertyInt, 0), boxLengthSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    boxLengthSubdivisions.mGetter = GetBoxLengthSubdivisions;
    boxLengthSubdivisions.mSetter = SetBoxLengthSubdivisions;
    boxLengthSubdivisions.mMin = 1;
    boxLengthSubdivisions.mDefault = 1;
    boxLengthSubdivisions.mHelp = String("Number of length subdivsions (along Y axis)");
    const int boxHeightSubdivisionsType = 0;
    IntMetadata<int>& boxHeightSubdivisions =
        TypeSpecificMetadata(makeBoxRegistry.RegisterProp("height_subdivisions", -1, kPropertyInt, 0), boxHeightSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    boxHeightSubdivisions.mGetter = GetBoxHeightSubdivisions;
    boxHeightSubdivisions.mSetter = SetBoxHeightSubdivisions;
    boxHeightSubdivisions.mMin = 1;
    boxHeightSubdivisions.mDefault = 1;
    boxHeightSubdivisions.mHelp = String("Number of height subdivsions (along Z axis)");
    ActionMetadata& makeBoxAction =
        static_cast<ActionMetadata&>(*makeBoxRegistry.RegisterAction("make_box").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeBoxAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateBox(gMakeBoxParams));
    };
    makeBox.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeSphere = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_sphere", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeSphere.mHelp = String("Form the mesh into a sphere");
    PropRegistry& makeSphereRegistry = makeSphere.mRegistry;
    const float sphereRadiusType = 0.0F;
    FloatMetadata& sphereRadius =
        TypeSpecificMetadata(makeSphereRegistry.RegisterProp("radius", -1, kPropertyFloat, 0), sphereRadiusType);
    static_cast<void>(Symbol("prop"));
    sphereRadius.mGetter = GetSphereRadius;
    sphereRadius.mSetter = SetSphereRadius;
    sphereRadius.mMin = 0.0F;
    sphereRadius.mDefault = 1.0F;
    sphereRadius.mHelp = String("Radius of the resulting sphere");
    const float sphereSweepAngleType = 0.0F;
    FloatMetadata& sphereSweepAngle =
        TypeSpecificMetadata(makeSphereRegistry.RegisterProp("sweep_angle", -1, kPropertyFloat, 0), sphereSweepAngleType);
    static_cast<void>(Symbol("prop"));
    sphereSweepAngle.mGetter = GetSphereSweepAngle;
    sphereSweepAngle.mSetter = SetSphereSweepAngle;
    sphereSweepAngle.mMin = 0.0F;
    sphereSweepAngle.mMax = 360.0F;
    sphereSweepAngle.mSlider = true;
    sphereSweepAngle.mDefault = 360.0F;
    sphereSweepAngle.mHelp = String("Sweep angle of the resulting sphere (eg. 360 for a full sphere, 180 for a hemisphere)");
    const unsigned long sphereRadialSubdivisionsType = 0;
    IntMetadata<unsigned long>& sphereRadialSubdivisions =
        TypeSpecificMetadata(makeSphereRegistry.RegisterProp("radial_subdivisions", -1, kPropertyUInt64, 0), sphereRadialSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    sphereRadialSubdivisions.mGetter = GetSphereRadialSubdivisions;
    sphereRadialSubdivisions.mSetter = SetSphereRadialSubdivisions;
    sphereRadialSubdivisions.mMin = 3UL;
    sphereRadialSubdivisions.mDefault = 32UL;
    sphereRadialSubdivisions.mHelp = String("Number of radial subdivsions");
    const unsigned long sphereLatitudeSubdivisionsType = 0;
    IntMetadata<unsigned long>& sphereLatitudeSubdivisions =
        TypeSpecificMetadata(makeSphereRegistry.RegisterProp("latitude_subdivisions", -1, kPropertyUInt64, 0), sphereLatitudeSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    sphereLatitudeSubdivisions.mGetter = GetSphereLatitudeSubdivisions;
    sphereLatitudeSubdivisions.mSetter = SetSphereLatitudeSubdivisions;
    sphereLatitudeSubdivisions.mMin = 2UL;
    sphereLatitudeSubdivisions.mDefault = 16UL;
    sphereLatitudeSubdivisions.mHelp = String("Number of latitude subdivsions");
    ActionMetadata& makeSphereAction =
        static_cast<ActionMetadata&>(*makeSphereRegistry.RegisterAction("make_sphere").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeSphereAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateSphere(gMakeSphereParams));
    };
    makeSphere.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeCylinder = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_cylinder", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeCylinder.mHelp = String("Form the mesh into a cylinder");
    PropRegistry& makeCylinderRegistry = makeCylinder.mRegistry;
    const float cylinderRadiusType = 0.0F;
    FloatMetadata& cylinderRadius =
        TypeSpecificMetadata(makeCylinderRegistry.RegisterProp("radius", -1, kPropertyFloat, 0), cylinderRadiusType);
    static_cast<void>(Symbol("prop"));
    cylinderRadius.mGetter = GetCylinderRadius;
    cylinderRadius.mSetter = SetCylinderRadius;
    cylinderRadius.mMin = 0.0F;
    cylinderRadius.mDefault = 1.0F;
    cylinderRadius.mHelp = String("Radius of the resulting cylinder");
    const float cylinderHeightType = 0.0F;
    FloatMetadata& cylinderHeight =
        TypeSpecificMetadata(makeCylinderRegistry.RegisterProp("height", -1, kPropertyFloat, 0), cylinderHeightType);
    static_cast<void>(Symbol("prop"));
    cylinderHeight.mGetter = GetCylinderHeight;
    cylinderHeight.mSetter = SetCylinderHeight;
    cylinderHeight.mMin = 0.0F;
    cylinderHeight.mDefault = 2.0F;
    cylinderHeight.mHelp = String("Height of the resulting cylinder");
    const float cylinderSweepAngleType = 0.0F;
    FloatMetadata& cylinderSweepAngle =
        TypeSpecificMetadata(makeCylinderRegistry.RegisterProp("sweep_angle", -1, kPropertyFloat, 0), cylinderSweepAngleType);
    static_cast<void>(Symbol("prop"));
    cylinderSweepAngle.mGetter = GetCylinderSweepAngle;
    cylinderSweepAngle.mSetter = SetCylinderSweepAngle;
    cylinderSweepAngle.mMin = 0.0F;
    cylinderSweepAngle.mMax = 360.0F;
    cylinderSweepAngle.mSlider = true;
    cylinderSweepAngle.mDefault = 360.0F;
    cylinderSweepAngle.mHelp = String("Sweep angle of the resulting cylinder");
    const unsigned long cylinderRadialSubdivisionsType = 0;
    IntMetadata<unsigned long>& cylinderRadialSubdivisions =
        TypeSpecificMetadata(makeCylinderRegistry.RegisterProp("radial_subdivisions", -1, kPropertyUInt64, 0), cylinderRadialSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    cylinderRadialSubdivisions.mGetter = GetCylinderRadialSubdivisions;
    cylinderRadialSubdivisions.mSetter = SetCylinderRadialSubdivisions;
    cylinderRadialSubdivisions.mMin = 3UL;
    cylinderRadialSubdivisions.mDefault = 32UL;
    cylinderRadialSubdivisions.mHelp = String("Number of radial subdivsions");
    const unsigned long cylinderHeightSubdivisionsType = 0;
    IntMetadata<unsigned long>& cylinderHeightSubdivisions =
        TypeSpecificMetadata(makeCylinderRegistry.RegisterProp("height_subdivisions", -1, kPropertyUInt64, 0), cylinderHeightSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    cylinderHeightSubdivisions.mGetter = GetCylinderHeightSubdivisions;
    cylinderHeightSubdivisions.mSetter = SetCylinderHeightSubdivisions;
    cylinderHeightSubdivisions.mMin = 1UL;
    cylinderHeightSubdivisions.mDefault = 1UL;
    cylinderHeightSubdivisions.mHelp = String("Number of height subdivsions");
    ActionMetadata& makeCylinderAction =
        static_cast<ActionMetadata&>(*makeCylinderRegistry.RegisterAction("make_cylinder").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeCylinderAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateCylinder(gMakeCylinderParams));
    };
    makeCylinder.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeCapsule = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_capsule", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeCapsule.mHelp = String("Form the mesh into a capsule");
    PropRegistry& makeCapsuleRegistry = makeCapsule.mRegistry;
    const float capsuleRadiusType = 0.0F;
    FloatMetadata& capsuleRadius =
        TypeSpecificMetadata(makeCapsuleRegistry.RegisterProp("radius", -1, kPropertyFloat, 0), capsuleRadiusType);
    static_cast<void>(Symbol("prop"));
    capsuleRadius.mGetter = GetCapsuleRadius;
    capsuleRadius.mSetter = SetCapsuleRadius;
    capsuleRadius.mMin = 0.0F;
    capsuleRadius.mDefault = 1.0F;
    capsuleRadius.mHelp = String("Radius of the resulting capsule");
    const float capsuleHeightType = 0.0F;
    FloatMetadata& capsuleHeight =
        TypeSpecificMetadata(makeCapsuleRegistry.RegisterProp("height", -1, kPropertyFloat, 0), capsuleHeightType);
    static_cast<void>(Symbol("prop"));
    capsuleHeight.mGetter = GetCapsuleHeight;
    capsuleHeight.mSetter = SetCapsuleHeight;
    capsuleHeight.mMin = 0.0F;
    capsuleHeight.mDefault = 2.0F;
    capsuleHeight.mHelp = String("Height of the resulting capsule");
    const float capsuleSweepAngleType = 0.0F;
    FloatMetadata& capsuleSweepAngle =
        TypeSpecificMetadata(makeCapsuleRegistry.RegisterProp("sweep_angle", -1, kPropertyFloat, 0), capsuleSweepAngleType);
    static_cast<void>(Symbol("prop"));
    capsuleSweepAngle.mGetter = GetCapsuleSweepAngle;
    capsuleSweepAngle.mSetter = SetCapsuleSweepAngle;
    capsuleSweepAngle.mMin = 0.0F;
    capsuleSweepAngle.mMax = 360.0F;
    capsuleSweepAngle.mSlider = true;
    capsuleSweepAngle.mDefault = 360.0F;
    capsuleSweepAngle.mHelp = String("Sweep angle of the resulting capsule");
    const unsigned long capsuleRadialSubdivisionsType = 0;
    IntMetadata<unsigned long>& capsuleRadialSubdivisions =
        TypeSpecificMetadata(makeCapsuleRegistry.RegisterProp("radial_subdivisions", -1, kPropertyUInt64, 0), capsuleRadialSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    capsuleRadialSubdivisions.mGetter = GetCapsuleRadialSubdivisions;
    capsuleRadialSubdivisions.mSetter = SetCapsuleRadialSubdivisions;
    capsuleRadialSubdivisions.mMin = 3UL;
    capsuleRadialSubdivisions.mDefault = 32UL;
    capsuleRadialSubdivisions.mHelp = String("Number of radial subdivsions");
    const unsigned long capsuleLatitudeSubdivisionsType = 0;
    IntMetadata<unsigned long>& capsuleLatitudeSubdivisions =
        TypeSpecificMetadata(makeCapsuleRegistry.RegisterProp("latitude_subdivisions", -1, kPropertyUInt64, 0), capsuleLatitudeSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    capsuleLatitudeSubdivisions.mGetter = GetCapsuleLatitudeSubdivisions;
    capsuleLatitudeSubdivisions.mSetter = SetCapsuleLatitudeSubdivisions;
    capsuleLatitudeSubdivisions.mMin = 2UL;
    capsuleLatitudeSubdivisions.mDefault = 8UL;
    capsuleLatitudeSubdivisions.mHelp = String("Number of latitude subdivsions per hemisphere");
    const unsigned long capsuleHeightSubdivisionsType = 0;
    IntMetadata<unsigned long>& capsuleHeightSubdivisions =
        TypeSpecificMetadata(makeCapsuleRegistry.RegisterProp("height_subdivisions", -1, kPropertyUInt64, 0), capsuleHeightSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    capsuleHeightSubdivisions.mGetter = GetCapsuleHeightSubdivisions;
    capsuleHeightSubdivisions.mSetter = SetCapsuleHeightSubdivisions;
    capsuleHeightSubdivisions.mMin = 1UL;
    capsuleHeightSubdivisions.mDefault = 1UL;
    capsuleHeightSubdivisions.mHelp = String("Number of height subdivsions");
    ActionMetadata& makeCapsuleAction =
        static_cast<ActionMetadata&>(*makeCapsuleRegistry.RegisterAction("make_capsule").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeCapsuleAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateCapsule(gMakeCapsuleParams));
    };
    makeCapsule.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeCone = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_cone", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeCone.mHelp = String("Form the mesh into a cone");
    PropRegistry& makeConeRegistry = makeCone.mRegistry;
    const float coneRadiusType = 0.0F;
    FloatMetadata& coneRadius =
        TypeSpecificMetadata(makeConeRegistry.RegisterProp("radius", -1, kPropertyFloat, 0), coneRadiusType);
    static_cast<void>(Symbol("prop"));
    coneRadius.mGetter = GetConeRadius;
    coneRadius.mSetter = SetConeRadius;
    coneRadius.mMin = 0.0F;
    coneRadius.mDefault = 1.0F;
    coneRadius.mHelp = String("Radius of the resulting cone");
    const float coneHeightType = 0.0F;
    FloatMetadata& coneHeight =
        TypeSpecificMetadata(makeConeRegistry.RegisterProp("height", -1, kPropertyFloat, 0), coneHeightType);
    static_cast<void>(Symbol("prop"));
    coneHeight.mGetter = GetConeHeight;
    coneHeight.mSetter = SetConeHeight;
    coneHeight.mMin = 0.0F;
    coneHeight.mDefault = 2.0F;
    coneHeight.mHelp = String("Height of the resulting cone");
    const float coneSweepAngleType = 0.0F;
    FloatMetadata& coneSweepAngle =
        TypeSpecificMetadata(makeConeRegistry.RegisterProp("sweep_angle", -1, kPropertyFloat, 0), coneSweepAngleType);
    static_cast<void>(Symbol("prop"));
    coneSweepAngle.mGetter = GetConeSweepAngle;
    coneSweepAngle.mSetter = SetConeSweepAngle;
    coneSweepAngle.mMin = 0.0F;
    coneSweepAngle.mMax = 360.0F;
    coneSweepAngle.mSlider = true;
    coneSweepAngle.mDefault = 360.0F;
    coneSweepAngle.mHelp = String("Sweep angle of the resulting cone");
    const unsigned long coneRadialSubdivisionsType = 0;
    IntMetadata<unsigned long>& coneRadialSubdivisions =
        TypeSpecificMetadata(makeConeRegistry.RegisterProp("radial_subdivisions", -1, kPropertyUInt64, 0), coneRadialSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    coneRadialSubdivisions.mGetter = GetConeRadialSubdivisions;
    coneRadialSubdivisions.mSetter = SetConeRadialSubdivisions;
    coneRadialSubdivisions.mMin = 3UL;
    coneRadialSubdivisions.mDefault = 32UL;
    coneRadialSubdivisions.mHelp = String("Number of radial subdivsions");
    const unsigned long coneHeightSubdivisionsType = 0;
    IntMetadata<unsigned long>& coneHeightSubdivisions =
        TypeSpecificMetadata(makeConeRegistry.RegisterProp("height_subdivisions", -1, kPropertyUInt64, 0), coneHeightSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    coneHeightSubdivisions.mGetter = GetConeHeightSubdivisions;
    coneHeightSubdivisions.mSetter = SetConeHeightSubdivisions;
    coneHeightSubdivisions.mMin = 1UL;
    coneHeightSubdivisions.mDefault = 1UL;
    coneHeightSubdivisions.mHelp = String("Number of height subdivsions");
    ActionMetadata& makeConeAction =
        static_cast<ActionMetadata&>(*makeConeRegistry.RegisterAction("make_cone").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeConeAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateCone(gMakeConeParams));
    };
    makeCone.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeTorus = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_torus", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeTorus.mHelp = String("Form the mesh into a torus");
    PropRegistry& makeTorusRegistry = makeTorus.mRegistry;
    const float torusPrimaryRadiusType = 0.0F;
    FloatMetadata& torusPrimaryRadius =
        TypeSpecificMetadata(makeTorusRegistry.RegisterProp("primary_radius", -1, kPropertyFloat, 0), torusPrimaryRadiusType);
    static_cast<void>(Symbol("prop"));
    torusPrimaryRadius.mGetter = GetTorusPrimaryRadius;
    torusPrimaryRadius.mSetter = SetTorusPrimaryRadius;
    torusPrimaryRadius.mMin = 0.0F;
    torusPrimaryRadius.mDefault = 1.0F;
    torusPrimaryRadius.mHelp = String("Primary radius of the resulting torus");
    const float torusSecondaryRadiusType = 0.0F;
    FloatMetadata& torusSecondaryRadius =
        TypeSpecificMetadata(makeTorusRegistry.RegisterProp("secondary_radius", -1, kPropertyFloat, 0), torusSecondaryRadiusType);
    static_cast<void>(Symbol("prop"));
    torusSecondaryRadius.mGetter = GetTorusSecondaryRadius;
    torusSecondaryRadius.mSetter = SetTorusSecondaryRadius;
    torusSecondaryRadius.mMin = 0.0F;
    torusSecondaryRadius.mDefault = 0.25F;
    torusSecondaryRadius.mHelp = String("Tube radius of the resulting torus");
    const float torusSweepAngleType = 0.0F;
    FloatMetadata& torusSweepAngle =
        TypeSpecificMetadata(makeTorusRegistry.RegisterProp("sweep_angle", -1, kPropertyFloat, 0), torusSweepAngleType);
    static_cast<void>(Symbol("prop"));
    torusSweepAngle.mGetter = GetTorusSweepAngle;
    torusSweepAngle.mSetter = SetTorusSweepAngle;
    torusSweepAngle.mMin = 0.0F;
    torusSweepAngle.mMax = 360.0F;
    torusSweepAngle.mSlider = true;
    torusSweepAngle.mDefault = 360.0F;
    torusSweepAngle.mHelp = String("Sweep angle of the resulting torus");
    const unsigned long torusRadialSubdivisionsType = 0;
    IntMetadata<unsigned long>& torusRadialSubdivisions =
        TypeSpecificMetadata(makeTorusRegistry.RegisterProp("radial_subdivisions", -1, kPropertyUInt64, 0), torusRadialSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    torusRadialSubdivisions.mGetter = GetTorusRadialSubdivisions;
    torusRadialSubdivisions.mSetter = SetTorusRadialSubdivisions;
    torusRadialSubdivisions.mMin = 3UL;
    torusRadialSubdivisions.mDefault = 32UL;
    torusRadialSubdivisions.mHelp = String("Number of radial subdivsions");
    const unsigned long torusLatitudeSubdivisionsType = 0;
    IntMetadata<unsigned long>& torusLatitudeSubdivisions =
        TypeSpecificMetadata(makeTorusRegistry.RegisterProp("latitude_subdivisions", -1, kPropertyUInt64, 0), torusLatitudeSubdivisionsType);
    static_cast<void>(Symbol("prop"));
    torusLatitudeSubdivisions.mGetter = GetTorusLatitudeSubdivisions;
    torusLatitudeSubdivisions.mSetter = SetTorusLatitudeSubdivisions;
    torusLatitudeSubdivisions.mMin = 3UL;
    torusLatitudeSubdivisions.mDefault = 32UL;
    torusLatitudeSubdivisions.mHelp = String("Number of latitude subdivsions");
    ActionMetadata& makeTorusAction =
        static_cast<ActionMetadata&>(*makeTorusRegistry.RegisterAction("make_torus").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeTorusAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateTorus(gMakeTorusParams));
    };
    makeTorus.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeIcosahedron = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_icosahedron", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeIcosahedron.mHelp = String("Form the mesh into a icosahedron");
    PropRegistry& makeIcosahedronRegistry = makeIcosahedron.mRegistry;
    const float icosahedronRadiusType = 0.0F;
    FloatMetadata& icosahedronRadius =
        TypeSpecificMetadata(makeIcosahedronRegistry.RegisterProp("radius", -1, kPropertyFloat, 0), icosahedronRadiusType);
    static_cast<void>(Symbol("prop"));
    icosahedronRadius.mGetter = GetIcosahedronRadius;
    icosahedronRadius.mSetter = SetIcosahedronRadius;
    icosahedronRadius.mMin = 0.0F;
    icosahedronRadius.mDefault = 1.0F;
    icosahedronRadius.mHelp = String("Radius of the resulting icosahedron");
    ActionMetadata& makeIcosahedronAction =
        static_cast<ActionMetadata&>(*makeIcosahedronRegistry.RegisterAction("make_icosahedron").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeIcosahedronAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        com.SetInlineMesh(RndMeshUtl::CreateIcosahedron(gMakeIcosahedronParams));
    };
    makeIcosahedron.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& makeSplineFan = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("make_spline_fan", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    makeSplineFan.mHelp = String("Form the mesh into a fan following the contour of a spline");
    PropRegistry& makeSplineFanRegistry = makeSplineFan.mRegistry;
    const GameObjectId splineType = {0xFFFFFFFFU};
    GameObjectIdMetadata& spline = TypeSpecificMetadata(
        makeSplineFanRegistry.RegisterProp("spline", -1, kPropertyGameObjectId, 0), splineType);
    static_cast<void>(Symbol("prop"));
    spline.mGetter = GetSplineFanSpline;
    spline.mSetter = SetSplineFanSpline;
    spline.mRequiredComs = {RndSplineCom::sId};
    const unsigned long sectionsType = 0;
    IntMetadata<unsigned long>& sections = TypeSpecificMetadata(
        makeSplineFanRegistry.RegisterProp("sections_per_knot_span", -1, kPropertyUInt64, 0), sectionsType);
    static_cast<void>(Symbol("prop"));
    sections.mGetter = GetSplineFanSectionsPerKnotSpan;
    sections.mSetter = SetSplineFanSectionsPerKnotSpan;
    sections.mMin = 1UL;
    ActionMetadata& makeSplineFanAction =
        static_cast<ActionMetadata&>(*makeSplineFanRegistry.RegisterAction("make_spline_fan").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    makeSplineFanAction.mAction = [](const TagArgs& args) {
        RndMeshCom& com = const_cast<RndMeshCom&>(static_cast<const RndMeshCom&>(args.mComponent));
        if (gMakeSplineFanSpline.mId == gNullObjectId.mId) {
            return;
        }
        const GameObject* const object = com.mObject->mEntity->GetObject(gMakeSplineFanSpline);
        if (object == nullptr) {
            return;
        }
        if (const RndSplineCom* const splineCom = object->GetCom<RndSplineCom>()) {
            com.SetInlineMesh(RndMeshUtl::CreateSplineFan(*splineCom, gMakeSplineFanParams));
        }
    };
    makeSplineFanAction.mReadOnly._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return gMakeSplineFanSpline.mId == gNullObjectId.mId;
    });
    makeSplineFan.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndMeshCom&>(args.mComponent).mDriven;
    });

    GroupMetadata& exportToFbx = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("export_to_fbx", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    exportToFbx.mHelp = String("Export this mesh to an FBX file");
    PropRegistry& exportToFbxRegistry = exportToFbx.mRegistry;
    const Symbol filepathType;
    SymbolMetadata& filepath = TypeSpecificMetadata(
        exportToFbxRegistry.RegisterProp("filepath", -1, kPropertySymbol, 0), filepathType);
    static_cast<void>(Symbol("prop"));
    filepath.mGetter = GetExportFilepath;
    filepath.mSetter = SetExportFilepath;
    ActionMetadata& exportToFbxAction =
        static_cast<ActionMetadata&>(*exportToFbxRegistry.RegisterAction("export_to_fbx").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    exportToFbxAction.mAction = [](const TagArgs& args) {
        static_cast<void>(args);
    };
    exportToFbxAction.mReadOnly._Set([](const TagArgs& args) {
        return gExportFilepath == Symbol() || static_cast<const RndMeshCom&>(args.mComponent).mMesh == nullptr;
    });
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

namespace {

// Reconstructed from eboot.elf at 0x5CA9E0. The edit poll applies it.
void SetResourceUsage(const PropAccessorArgs& args) {
    static_cast<RndMeshCom*>(args.mComponent)->mPendingUsage = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CA9F0.
bool GetCompressVertices(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr ? resource->AreVerticesCompressed() : false;
}

// Reconstructed from eboot.elf at 0x5CAA30.
void SetCompressVertices(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetCompressVertices(*static_cast<const bool*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CAA80.
bool GetSmoothTangents(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr && resource->mSmoothTangents;
}

// Reconstructed from eboot.elf at 0x5CAAC0.
void SetSmoothTangents(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetSmoothTangents(*static_cast<const bool*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CAB10.
bool GetCalcUVScale(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr && resource->mCalcUVScale;
}

// Reconstructed from eboot.elf at 0x5CAB50.
void SetCalcUVScale(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetCalcUVScale(*static_cast<const bool*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CABA0.
bool GetCalcCurvature(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr && resource->mCalcCurvature;
}

// Reconstructed from eboot.elf at 0x5CABE0.
void SetCalcCurvature(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetCalcCurvature(*static_cast<const bool*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CAC30.
bool GetKeepMeshData(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr ? resource->IsKeepMeshData() : false;
}

// Reconstructed from eboot.elf at 0x5CAC70.
void SetKeepMeshData(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetKeepMeshData(*static_cast<const bool*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CACC0.
unsigned int GetVertexUsage(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr ? resource->GetVertexUsageFlags() : 0;
}

// Reconstructed from eboot.elf at 0x5CAD00.
void SetVertexUsage(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetVertexUsageFlags(*static_cast<const unsigned int*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CAD50.
unsigned int GetFaceUsage(const PropAccessorArgs& args) {
    const RndMeshResource* const resource = LoadedMeshResource(args);
    return resource != nullptr ? resource->GetFaceUsageFlags() : 0;
}

// Reconstructed from eboot.elf at 0x5CAD90.
void SetFaceUsage(const PropAccessorArgs& args) {
    if (RndMeshResource* const resource = LoadedMeshResource(args)) {
        resource->SetFaceUsageFlags(*static_cast<const unsigned int*>(args.mValue));
    }
}

// Reconstructed from eboot.elf at 0x5CADE0.
int GetVertexType(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    return mesh != nullptr ? static_cast<int>(mesh->_GetVertexTypeImpl()) : -1;
}

// Reconstructed from eboot.elf at 0x5CAE00.
unsigned long GetNumFaces(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    return mesh != nullptr ? mesh->mFaces.size() : 0;
}

// Reconstructed from eboot.elf at 0x5CAE30.
unsigned long GetNumVertices(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    return mesh != nullptr ? mesh->_GetNumVerticesImpl() : 0;
}

// Reconstructed from eboot.elf at 0x5CAE50. In kilobytes.
unsigned long GetVerticesSize(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    if (mesh == nullptr) {
        return 0;
    }
    const unsigned long count = mesh->_GetNumVerticesImpl();
    const RndVertexInterpreter* const layout =
        RndVertexInterpreter::GetInstance(mesh->_GetVertexTypeImpl());
    return (layout->mStride * count) >> 10;
}

// Reconstructed from eboot.elf at 0x5CAEA0.
Symbol GetRootBone(const PropAccessorArgs& args) {
    const RndMeshCom* const com = static_cast<RndMeshCom*>(args.mComponent);
    const RndMesh* const mesh = com->mMesh;
    if (mesh == nullptr || mesh->mGeometryFrame == -1) {
        return Symbol("<none>");
    }
    const long bone = mesh->mGeometryFrame;
    if (com->mSkeleton == nullptr) {
        return Symbol("<unknown (no skeleton)>");
    }
    const CharSkeleton* const skeleton = com->mSkeleton->GetSkeleton();
    if (skeleton == nullptr) {
        return Symbol("<invalid>");
    }
    return skeleton->mNames[bone];
}

// Reconstructed from eboot.elf at 0x5CAF40.
long GetRootBoneIndex(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    return mesh != nullptr ? mesh->mGeometryFrame : -1;
}

// Reconstructed from eboot.elf at 0x5CAF60.
float GetExtentsWidth(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    if (mesh == nullptr) {
        return 0.0F;
    }
    const Box box = RndMeshUtl::ComputeAABB(*mesh);
    return box.mMax.x - box.mMin.x;
}

// Reconstructed from eboot.elf at 0x5CAFC0.
float GetExtentsLength(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    if (mesh == nullptr) {
        return 0.0F;
    }
    const Box box = RndMeshUtl::ComputeAABB(*mesh);
    return box.mMax.y - box.mMin.y;
}

// Reconstructed from eboot.elf at 0x5CB020.
float GetExtentsHeight(const PropAccessorArgs& args) {
    const RndMesh* const mesh = static_cast<RndMeshCom*>(args.mComponent)->mMesh;
    if (mesh == nullptr) {
        return 0.0F;
    }
    const Box box = RndMeshUtl::ComputeAABB(*mesh);
    return box.mMax.z - box.mMin.z;
}

// Reconstructed from eboot.elf at 0x5CB0E0. The triangle spans its first and
// third points.
float GetTriangleWidth(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTriangleParams.mPoints[0].x + gMakeTriangleParams.mPoints[0].x;
}

// Reconstructed from eboot.elf at 0x5CB0F0.
void SetTriangleWidth(const PropAccessorArgs& args) {
    gMakeTriangleParams.mPoints[0].x = *static_cast<const float*>(args.mValue) * 0.5F;
    gMakeTriangleParams.mPoints[2].x = *static_cast<const float*>(args.mValue) * -0.5F;
}

// Reconstructed from eboot.elf at 0x5CB240.
float GetCubeWidth(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCubeParams.mAxisX.x;
}

// Reconstructed from eboot.elf at 0x5CB250. The cube's three axes.
void SetCubeWidth(const PropAccessorArgs& args) {
    gMakeCubeParams.mAxisX.x = *static_cast<const float*>(args.mValue);
    gMakeCubeParams.mAxisY.y = *static_cast<const float*>(args.mValue);
    gMakeCubeParams.mAxisZ.z = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB280.
int GetCubeSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCubeParams.mNumSegmentsX;
}

// Reconstructed from eboot.elf at 0x5CB290.
void SetCubeSubdivisions(const PropAccessorArgs& args) {
    gMakeCubeParams.mNumSegmentsX = *static_cast<const int*>(args.mValue);
    gMakeCubeParams.mNumSegmentsY = *static_cast<const int*>(args.mValue);
    gMakeCubeParams.mNumSegmentsZ = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB080.
int GetBakeXfmMode(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gBakeXfmMode;
}

// Reconstructed from eboot.elf at 0x5CB090.
void SetBakeXfmMode(const PropAccessorArgs& args) {
    gBakeXfmMode = static_cast<RndMeshUtl::BakeXfmMode>(*static_cast<const int*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x5CB0A0.
GameObjectId GetCombineWith(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gCombineWith;
}

// Reconstructed from eboot.elf at 0x5CB0B0.
void SetCombineWith(const PropAccessorArgs& args) {
    gCombineWith = *static_cast<const GameObjectId*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB0C0.
bool GetCombineDeleteOriginal(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gCombineDeleteOriginal;
}

// Reconstructed from eboot.elf at 0x5CB0D0.
void SetCombineDeleteOriginal(const PropAccessorArgs& args) {
    gCombineDeleteOriginal = *static_cast<const bool*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB120.
float GetTriangleHeight(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTriangleParams.mPoints[1].y;
}

// Reconstructed from eboot.elf at 0x5CB130.
void SetTriangleHeight(const PropAccessorArgs& args) {
    gMakeTriangleParams.mPoints[1].y = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB140.
int GetQuadFacing(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return static_cast<int>(gMakeQuadParams.mFacing);
}

// Reconstructed from eboot.elf at 0x5CB150.
void SetQuadFacing(const PropAccessorArgs& args) {
    gMakeQuadParams.mFacing = static_cast<unsigned int>(*static_cast<const int*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x5CB160.
float GetQuadWidth(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeQuadParams.mWidth;
}

// Reconstructed from eboot.elf at 0x5CB170.
void SetQuadWidth(const PropAccessorArgs& args) {
    gMakeQuadParams.mWidth = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB180.
float GetQuadHeight(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeQuadParams.mHeight;
}

// Reconstructed from eboot.elf at 0x5CB190.
void SetQuadHeight(const PropAccessorArgs& args) {
    gMakeQuadParams.mHeight = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB1A0.
int GetQuadWidthSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeQuadParams.mNumSegmentsU;
}

// Reconstructed from eboot.elf at 0x5CB1B0.
void SetQuadWidthSubdivisions(const PropAccessorArgs& args) {
    gMakeQuadParams.mNumSegmentsU = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB1C0.
int GetQuadHeightSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeQuadParams.mNumSegmentsV;
}

// Reconstructed from eboot.elf at 0x5CB1D0.
void SetQuadHeightSubdivisions(const PropAccessorArgs& args) {
    gMakeQuadParams.mNumSegmentsV = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB1E0.
int GetTriangleFanFacing(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return static_cast<int>(gMakeTriangleFanParams.mFacing);
}

// Reconstructed from eboot.elf at 0x5CB1F0.
void SetTriangleFanFacing(const PropAccessorArgs& args) {
    gMakeTriangleFanParams.mFacing = static_cast<unsigned int>(*static_cast<const int*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x5CB200.
float GetTriangleFanRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTriangleFanParams.mRadius;
}

// Reconstructed from eboot.elf at 0x5CB210.
void SetTriangleFanRadius(const PropAccessorArgs& args) {
    gMakeTriangleFanParams.mRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB220.
int GetTriangleFanNumSides(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return static_cast<int>(gMakeTriangleFanParams.mNumSegments);
}

// Reconstructed from eboot.elf at 0x5CB230.
void SetTriangleFanNumSides(const PropAccessorArgs& args) {
    gMakeTriangleFanParams.mNumSegments = static_cast<unsigned long>(*static_cast<const int*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x5CB2C0.
float GetBoxWidth(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeBoxParams.mAxisX.x;
}

// Reconstructed from eboot.elf at 0x5CB2D0.
void SetBoxWidth(const PropAccessorArgs& args) {
    gMakeBoxParams.mAxisX.x = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB2E0.
float GetBoxLength(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeBoxParams.mAxisY.y;
}

// Reconstructed from eboot.elf at 0x5CB2F0.
void SetBoxLength(const PropAccessorArgs& args) {
    gMakeBoxParams.mAxisY.y = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB300.
float GetBoxHeight(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeBoxParams.mAxisZ.z;
}

// Reconstructed from eboot.elf at 0x5CB310.
void SetBoxHeight(const PropAccessorArgs& args) {
    gMakeBoxParams.mAxisZ.z = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB320.
int GetBoxWidthSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeBoxParams.mNumSegmentsX;
}

// Reconstructed from eboot.elf at 0x5CB330.
void SetBoxWidthSubdivisions(const PropAccessorArgs& args) {
    gMakeBoxParams.mNumSegmentsX = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB340.
int GetBoxLengthSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeBoxParams.mNumSegmentsY;
}

// Reconstructed from eboot.elf at 0x5CB350.
void SetBoxLengthSubdivisions(const PropAccessorArgs& args) {
    gMakeBoxParams.mNumSegmentsY = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB360.
int GetBoxHeightSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeBoxParams.mNumSegmentsZ;
}

// Reconstructed from eboot.elf at 0x5CB370.
void SetBoxHeightSubdivisions(const PropAccessorArgs& args) {
    gMakeBoxParams.mNumSegmentsZ = *static_cast<const int*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB380.
float GetSphereRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeSphereParams.mRadius;
}

// Reconstructed from eboot.elf at 0x5CB390.
void SetSphereRadius(const PropAccessorArgs& args) {
    gMakeSphereParams.mRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB3A0.
float GetSphereSweepAngle(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeSphereParams.mSweepAngle * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x5CB3C0.
void SetSphereSweepAngle(const PropAccessorArgs& args) {
    gMakeSphereParams.mSweepAngle = *static_cast<const float*>(args.mValue) * kDegreesToRadians;
}

// Reconstructed from eboot.elf at 0x5CB3E0.
unsigned long GetSphereRadialSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeSphereParams.mNumSegments;
}

// Reconstructed from eboot.elf at 0x5CB3F0.
void SetSphereRadialSubdivisions(const PropAccessorArgs& args) {
    gMakeSphereParams.mNumSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB400.
unsigned long GetSphereLatitudeSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeSphereParams.mNumRings;
}

// Reconstructed from eboot.elf at 0x5CB410.
void SetSphereLatitudeSubdivisions(const PropAccessorArgs& args) {
    gMakeSphereParams.mNumRings = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB420.
float GetCylinderRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCylinderParams.mRadius;
}

// Reconstructed from eboot.elf at 0x5CB430.
void SetCylinderRadius(const PropAccessorArgs& args) {
    gMakeCylinderParams.mRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB440.
float GetCylinderHeight(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCylinderParams.mHeight;
}

// Reconstructed from eboot.elf at 0x5CB450.
void SetCylinderHeight(const PropAccessorArgs& args) {
    gMakeCylinderParams.mHeight = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB460.
float GetCylinderSweepAngle(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCylinderParams.mSweepAngle * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x5CB480.
void SetCylinderSweepAngle(const PropAccessorArgs& args) {
    gMakeCylinderParams.mSweepAngle = *static_cast<const float*>(args.mValue) * kDegreesToRadians;
}

// Reconstructed from eboot.elf at 0x5CB4A0.
unsigned long GetCylinderRadialSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCylinderParams.mNumSegments;
}

// Reconstructed from eboot.elf at 0x5CB4B0.
void SetCylinderRadialSubdivisions(const PropAccessorArgs& args) {
    gMakeCylinderParams.mNumSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB4C0.
unsigned long GetCylinderHeightSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCylinderParams.mNumHeightSegments;
}

// Reconstructed from eboot.elf at 0x5CB4D0.
void SetCylinderHeightSubdivisions(const PropAccessorArgs& args) {
    gMakeCylinderParams.mNumHeightSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB4E0.
float GetCapsuleRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCapsuleParams.mRadius;
}

// Reconstructed from eboot.elf at 0x5CB4F0.
void SetCapsuleRadius(const PropAccessorArgs& args) {
    gMakeCapsuleParams.mRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB500.
float GetCapsuleHeight(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCapsuleParams.mLength;
}

// Reconstructed from eboot.elf at 0x5CB510.
void SetCapsuleHeight(const PropAccessorArgs& args) {
    gMakeCapsuleParams.mLength = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB520.
float GetCapsuleSweepAngle(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCapsuleParams.mSweepAngle * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x5CB540.
void SetCapsuleSweepAngle(const PropAccessorArgs& args) {
    gMakeCapsuleParams.mSweepAngle = *static_cast<const float*>(args.mValue) * kDegreesToRadians;
}

// Reconstructed from eboot.elf at 0x5CB560.
unsigned long GetCapsuleRadialSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCapsuleParams.mNumSegments;
}

// Reconstructed from eboot.elf at 0x5CB570.
void SetCapsuleRadialSubdivisions(const PropAccessorArgs& args) {
    gMakeCapsuleParams.mNumSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB580.
unsigned long GetCapsuleLatitudeSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCapsuleParams.mNumCapSegments;
}

// Reconstructed from eboot.elf at 0x5CB590.
void SetCapsuleLatitudeSubdivisions(const PropAccessorArgs& args) {
    gMakeCapsuleParams.mNumCapSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB5A0.
unsigned long GetCapsuleHeightSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeCapsuleParams.mNumSideSegments;
}

// Reconstructed from eboot.elf at 0x5CB5B0.
void SetCapsuleHeightSubdivisions(const PropAccessorArgs& args) {
    gMakeCapsuleParams.mNumSideSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB5C0.
float GetConeRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeConeParams.mRadius;
}

// Reconstructed from eboot.elf at 0x5CB5D0.
void SetConeRadius(const PropAccessorArgs& args) {
    gMakeConeParams.mRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB5E0.
float GetConeHeight(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeConeParams.mHeight;
}

// Reconstructed from eboot.elf at 0x5CB5F0.
void SetConeHeight(const PropAccessorArgs& args) {
    gMakeConeParams.mHeight = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB600.
float GetConeSweepAngle(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeConeParams.mSweepAngle * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x5CB620.
void SetConeSweepAngle(const PropAccessorArgs& args) {
    gMakeConeParams.mSweepAngle = *static_cast<const float*>(args.mValue) * kDegreesToRadians;
}

// Reconstructed from eboot.elf at 0x5CB640.
unsigned long GetConeRadialSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeConeParams.mNumSegments;
}

// Reconstructed from eboot.elf at 0x5CB650.
void SetConeRadialSubdivisions(const PropAccessorArgs& args) {
    gMakeConeParams.mNumSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB660.
unsigned long GetConeHeightSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeConeParams.mNumHeightSegments;
}

// Reconstructed from eboot.elf at 0x5CB670.
void SetConeHeightSubdivisions(const PropAccessorArgs& args) {
    gMakeConeParams.mNumHeightSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB680.
float GetTorusPrimaryRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTorusParams.mPrimaryRadius;
}

// Reconstructed from eboot.elf at 0x5CB690.
void SetTorusPrimaryRadius(const PropAccessorArgs& args) {
    gMakeTorusParams.mPrimaryRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB6A0.
float GetTorusSecondaryRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTorusParams.mSecondaryRadius;
}

// Reconstructed from eboot.elf at 0x5CB6B0.
void SetTorusSecondaryRadius(const PropAccessorArgs& args) {
    gMakeTorusParams.mSecondaryRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB6C0.
float GetTorusSweepAngle(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTorusParams.mSweepAngle * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x5CB6E0.
void SetTorusSweepAngle(const PropAccessorArgs& args) {
    gMakeTorusParams.mSweepAngle = *static_cast<const float*>(args.mValue) * kDegreesToRadians;
}

// Reconstructed from eboot.elf at 0x5CB700.
unsigned long GetTorusRadialSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTorusParams.mNumSegments;
}

// Reconstructed from eboot.elf at 0x5CB710.
void SetTorusRadialSubdivisions(const PropAccessorArgs& args) {
    gMakeTorusParams.mNumSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB720.
unsigned long GetTorusLatitudeSubdivisions(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeTorusParams.mNumLatitudeSegments;
}

// Reconstructed from eboot.elf at 0x5CB730.
void SetTorusLatitudeSubdivisions(const PropAccessorArgs& args) {
    gMakeTorusParams.mNumLatitudeSegments = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB740.
float GetIcosahedronRadius(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeIcosahedronParams.mRadius;
}

// Reconstructed from eboot.elf at 0x5CB750.
void SetIcosahedronRadius(const PropAccessorArgs& args) {
    gMakeIcosahedronParams.mRadius = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB760.
GameObjectId GetSplineFanSpline(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeSplineFanSpline;
}

// Reconstructed from eboot.elf at 0x5CB770.
void SetSplineFanSpline(const PropAccessorArgs& args) {
    gMakeSplineFanSpline = *static_cast<const GameObjectId*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB780.
unsigned long GetSplineFanSectionsPerKnotSpan(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gMakeSplineFanParams.mSectionsPerKnotSpan;
}

// Reconstructed from eboot.elf at 0x5CB790.
void SetSplineFanSectionsPerKnotSpan(const PropAccessorArgs& args) {
    gMakeSplineFanParams.mSectionsPerKnotSpan = *static_cast<const unsigned long*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x5CB7A0.
Symbol GetExportFilepath(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return gExportFilepath;
}

// Reconstructed from eboot.elf at 0x5CB7B0.
void SetExportFilepath(const PropAccessorArgs& args) {
    gExportFilepath = *static_cast<const Symbol*>(args.mValue);
}

}  // namespace

// Reconstructed from eboot.elf at 0x405690. The binary emits the factory
// with the renderer's component registration.
Component* RndMeshCom::_Create() {
    return new RndMeshCom();
}

// Reconstructed from eboot.elf at 0x3F7420. The binary emits the
// registration in render/RndInit.o, with the placement factory at 0x4056C0.
void RndMeshCom::Init() {
    InitComponentClass<RndMeshCom>();
}
