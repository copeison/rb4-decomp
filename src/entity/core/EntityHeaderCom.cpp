// The entity header component: an object the map does not name (0x1D1E90 to
// 0x1D8D37). The per-layer accessors (0x1D52E0-0x1D57CF) are not
// reconstructed.
#include "entity/core/EntityHeaderCom.h"

#include <cmath>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropMetadata.h"
#include "utl/text/MakeString.h"

namespace {

// The id of no object, at 0x19E4A78, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};

}  // namespace

// The object's statics, in the order of its static initializer (0x1D8C80).
Symbol EntityHeaderCom::sId("EntityHeader");
Symbol EntityHeaderCom::sClassName("EntityHeader");
PropRegistry EntityHeaderCom::sPropRegistry;
ComMetaData EntityHeaderCom::sMetaData;

namespace {

// The first poll-order position "poll_order" shows, shared by every entity
// (0x19E4CE8). Name not in the reference map.
unsigned long gPollOrderStartIndex;

// Whether the entity's resource is not a TransEntityResource, which hides
// the instancing, statistics and icon camera properties. The body of 16
// tag functions (0x1D6100 to 0x1D7C40). Name not in the reference map.
bool IsOutsideTransEntity(const TagArgs& args) {
    return !args.mComponent.mObject->mEntity->mResource->IsA(TransEntityResource::Id());
}

// The object at the poll-order position N after the start index, or the
// null id past the end. The binary emits one getter per position, from
// 0x1D5A80 (object_0) to 0x1D5CC0 (object_9). Name not in the reference
// map.
template <unsigned long N>
GameObjectId GetPollOrderObject(const PropAccessorArgs& args) {
    const PropArray<GameObjectId>& order = args.mComponent->mObject->mEntity->mPollOrder;
    const unsigned long index = gPollOrderStartIndex + N;
    return index < order.size() ? order[index] : gNullObjectId;
}

// Registers "object_N", the poll-order position N after the start index,
// named by its position. Inlined. Name not in the reference map.
template <unsigned long N>
void RegisterPollOrderObject(PropRegistry& registry, const char* name) {
    const GameObjectId type = {0xFFFFFFFFu};
    GameObjectIdMetadata& object =
        TypeSpecificMetadata(registry.RegisterProp(name, -1, kPropertyGameObjectId, 0), type);
    static_cast<void>(Symbol("prop"));
    object.mDisplayName._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return String(MakeString("object %d", gPollOrderStartIndex + N));
    });
    object.mReadOnly = true;
    object.mGetter = GetPollOrderObject<N>;
}

// Registers a three-float member of "icon_cam_xfm" with zero defaults.
// Inlined. Name not in the reference map.
void RegisterIconCamVector(PropRegistry& registry, const char* name, int offset) {
    PropRegistry& vector = static_cast<GroupMetadata&>(
        *registry.RegisterProp(name, offset, kPropertyStruct, sizeof(Vector3)).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    const Vector3 prototype = {0.0F, 0.0F, 0.0F};
    FloatMetadata& x =
        TypeSpecificMetadata(vector.RegisterProp("x", offsetof(Vector3, x), kPropertyFloat, 4), prototype.x);
    x.mDefault = prototype.x;
    FloatMetadata& y =
        TypeSpecificMetadata(vector.RegisterProp("y", offsetof(Vector3, y), kPropertyFloat, 4), prototype.y);
    y.mDefault = prototype.y;
    FloatMetadata& z =
        TypeSpecificMetadata(vector.RegisterProp("z", offsetof(Vector3, z), kPropertyFloat, 4), prototype.z);
    z.mDefault = prototype.z;
}

// Registers one of the read-only statistics, hidden outside a
// TransEntityResource; each offset has its own tag function. Inlined. Name
// not in the reference map.
template <int Offset>
void RegisterStatistic(PropRegistry& registry, const char* name, const int& value) {
    IntMetadata<int>& statistic = TypeSpecificMetadata(registry.RegisterProp(name, Offset, kPropertyInt, 4), value);
    statistic.mDefault = value;
    static_cast<void>(Symbol("prop"));
    statistic.mReadOnly = true;
    statistic.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });
}

}  // namespace

// Reconstructed from eboot.elf at 0x114E20, in entity/Utl.o. Every root
// must hold an EntityHeaderCom.
bool IsStaticEntity(const Entity* entity) {
    for (;;) {
        if (entity->GetRoot()->GetExistingCom<EntityHeaderCom>()->mStatic) {
            return true;
        }
        if (entity->mParentObject == nullptr) {
            return false;
        }
        entity = entity->GetParent();
    }
}

// Reconstructed from eboot.elf at 0x1D1E90.
EntityHeaderCom::EntityHeaderCom()
    : mNumInstances(0),
      mNumMeshes(0),
      mNumParticles(0),
      mNumPropanims(0),
      mNumLights(0),
      mNumVerts(0),
      mNumFaces(0),
      mChangelist(0),
      mIconCamInitialized(false),
      mIconCamXfm(Transform::sID),
      mIconCamNear(0.1F),
      mIconCamFar(1000.0F),
      mInstancePollingMode(0),
      mCopyOnInstance(true),
      mDrivesParent(false),
      mStatic(false),
      mCapabilities(15),
      mShowEntity(),
      mIconData(),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x1D1F60. Every layer starts visible,
// selectable and unmodified.
EntityHeaderCom::RuntimeData::RuntimeData() : mNumModifiedLayers(0), mPollRequested(false) {
    mLayerVisible.resize(kMaxLayers);
    for (bool& visible : mLayerVisible) {
        visible = true;
    }
    mLayerSelectable.resize(kMaxLayers);
    for (bool& selectable : mLayerSelectable) {
        selectable = true;
    }
    mLayerModified.resize(kMaxLayers);
    for (bool& modified : mLayerModified) {
        modified = false;
    }
    mLayerChanges.resize(kMaxLayers);
    for (LayerChanges& changes : mLayerChanges) {
        changes.mCount = 0;
        changes.mDirty = false;
    }
}

// Reconstructed from eboot.elf at 0x1D21D0. The instancing, statistics
// and icon camera properties are hidden outside a TransEntityResource,
// each through its own copy of the tag function; the statistics, the icon
// camera, "capabilities", "icon_data" and "poll_order" are read-only, and
// the icon camera, "capabilities" and "poll_order" are debug properties.
// Revision 1 clears an icon camera translation that is not finite. Every
// property registration builds a "prop" symbol it does not use, and the
// struct and array registrations "struct" and "array".
void EntityHeaderCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    EntityHeaderCom prototype;
    // At 0x1D5F90. The report is compiled out.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            EntityHeaderCom* const header = static_cast<EntityHeaderCom*>(args.mComponent);
            Vector3& translate = header->mIconCamXfm.v;
            if (!std::isfinite(translate.x) || !std::isfinite(translate.y) || !std::isfinite(translate.z)) {
                static_cast<void>(header->mObject->MakeErrorName());
                translate = {0.0F, 0.0F, 0.0F};
            }
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    BoolMetadata& copyOnInstance = TypeSpecificMetadata(
        registry.RegisterProp("copy_on_instance", offsetof(EntityHeaderCom, mCopyOnInstance), kPropertyBool, 1),
        prototype.mCopyOnInstance);
    copyOnInstance.mDefault = prototype.mCopyOnInstance;
    static_cast<void>(Symbol("prop"));
    copyOnInstance.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    BoolMetadata& drivesParent = TypeSpecificMetadata(
        registry.RegisterProp("drives_parent", offsetof(EntityHeaderCom, mDrivesParent), kPropertyBool, 1),
        prototype.mDrivesParent);
    drivesParent.mDefault = prototype.mDrivesParent;
    static_cast<void>(Symbol("prop"));
    drivesParent.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    BoolMetadata& isStatic = TypeSpecificMetadata(
        registry.RegisterProp("static", offsetof(EntityHeaderCom, mStatic), kPropertyBool, 1), prototype.mStatic);
    isStatic.mDefault = prototype.mStatic;
    static_cast<void>(Symbol("prop"));
    isStatic.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    // Read-only while the entity drives its instancing object.
    IntMetadata<int>& pollingMode = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "instance_polling_mode", offsetof(EntityHeaderCom, mInstancePollingMode), kPropertyInt, 4).mMetadata.Get());
    pollingMode.mDefault = prototype.mInstancePollingMode;
    static_cast<void>(Symbol("prop"));
    pollingMode.mHelp = String(
        "Defines the circumstances in which instances of this entity will be polled. Disabling polling when it "
        "is unnecessary improves CPU performance.");
    pollingMode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            0, String("Always poll"), String("Instances will always poll, even when hidden or off screen.")},
        AllowedValue<int>{
            1,
            String("Poll only when showing"),
            String(
                "Instances will not poll when hidden, but will always poll when showing (even when they are off "
                "screen).")},
        AllowedValue<int>{
            2,
            String("Poll only when on screen"),
            String(
                "Instances will poll only when on screen (i.e. in the camera frustum). If an instance is off "
                "screen, it will not poll. If an instance is not showing, it is by definition not 'on screen' and "
                "therefore will not poll.")}};
    pollingMode.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const EntityHeaderCom&>(args.mComponent).mDrivesParent;
    });
    pollingMode.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    PropRefMetadata& showEntity = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_entity", offsetof(EntityHeaderCom, mShowEntity), kPropertyPropRef, sizeof(PropRef)),
        prototype.mShowEntity);
    showEntity.mRefDefault = prototype.mShowEntity;
    static_cast<void>(Symbol("prop"));
    showEntity.mHelp = String(
        "If set, defines a property that is called by RndEntityCom during poll to show/hide the Entity.  That "
        "lets you optimize by not polling when not needed, based on criteria inside the entity");

    RegisterStatistic<offsetof(EntityHeaderCom, mNumInstances)>(registry, "num_instances", prototype.mNumInstances);
    RegisterStatistic<offsetof(EntityHeaderCom, mNumMeshes)>(registry, "num_meshes", prototype.mNumMeshes);
    RegisterStatistic<offsetof(EntityHeaderCom, mNumParticles)>(registry, "num_particles", prototype.mNumParticles);
    RegisterStatistic<offsetof(EntityHeaderCom, mNumPropanims)>(registry, "num_propanims", prototype.mNumPropanims);
    RegisterStatistic<offsetof(EntityHeaderCom, mNumLights)>(registry, "num_lights", prototype.mNumLights);
    RegisterStatistic<offsetof(EntityHeaderCom, mNumVerts)>(registry, "num_verts", prototype.mNumVerts);
    RegisterStatistic<offsetof(EntityHeaderCom, mNumFaces)>(registry, "num_faces", prototype.mNumFaces);
    RegisterStatistic<offsetof(EntityHeaderCom, mChangelist)>(registry, "changelist", prototype.mChangelist);

    BoolMetadata& iconCamInitialized = TypeSpecificMetadata(
        registry.RegisterProp(
            "icon_cam_initialized", offsetof(EntityHeaderCom, mIconCamInitialized), kPropertyBool, 1),
        prototype.mIconCamInitialized);
    iconCamInitialized.mDefault = prototype.mIconCamInitialized;
    static_cast<void>(Symbol("prop"));
    iconCamInitialized.mReadOnly = true;
    iconCamInitialized.mDebug = true;
    iconCamInitialized.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    FloatMetadata& iconCamNear = TypeSpecificMetadata(
        registry.RegisterProp("icon_cam_near", offsetof(EntityHeaderCom, mIconCamNear), kPropertyFloat, 4),
        prototype.mIconCamNear);
    iconCamNear.mDefault = prototype.mIconCamNear;
    static_cast<void>(Symbol("prop"));
    iconCamNear.mReadOnly = true;
    iconCamNear.mDebug = true;
    iconCamNear.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    FloatMetadata& iconCamFar = TypeSpecificMetadata(
        registry.RegisterProp("icon_cam_far", offsetof(EntityHeaderCom, mIconCamFar), kPropertyFloat, 4),
        prototype.mIconCamFar);
    iconCamFar.mDefault = prototype.mIconCamFar;
    static_cast<void>(Symbol("prop"));
    iconCamFar.mReadOnly = true;
    iconCamFar.mDebug = true;
    iconCamFar.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });

    GroupMetadata& iconCamXfm = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "icon_cam_xfm", offsetof(EntityHeaderCom, mIconCamXfm), kPropertyStruct, sizeof(Transform)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    iconCamXfm.mReadOnly = true;
    iconCamXfm.mDebug = true;
    iconCamXfm.mIsHidden._Set([](const TagArgs& args) { return IsOutsideTransEntity(args); });
    RegisterIconCamVector(iconCamXfm.mRegistry, "basis_x", offsetof(Transform, m.x));
    RegisterIconCamVector(iconCamXfm.mRegistry, "basis_y", offsetof(Transform, m.y));
    RegisterIconCamVector(iconCamXfm.mRegistry, "basis_z", offsetof(Transform, m.z));
    RegisterIconCamVector(iconCamXfm.mRegistry, "translate", offsetof(Transform, v));

    IntMetadata<unsigned int>& capabilities = TypeSpecificMetadata(
        registry.RegisterProp("capabilities", offsetof(EntityHeaderCom, mCapabilities), kPropertyUInt, 4),
        prototype.mCapabilities);
    capabilities.mDefault = prototype.mCapabilities;
    static_cast<void>(Symbol("prop"));
    capabilities.mReadOnly = true;
    capabilities.mDebug = true;
    capabilities.mAllowedValues = eastl::vector<AllowedValue<unsigned int>>{
        AllowedValue<unsigned int>{1, String("Manipulate Layers"), String("")},
        AllowedValue<unsigned int>{2, String("Create Objects"), String("")},
        AllowedValue<unsigned int>{4, String("Delete Objects"), String("")},
        AllowedValue<unsigned int>{8, String("Rename Objects"), String("")}};

    PropInfo& iconData = registry.RegisterPropArray(
        "icon_data", offsetof(EntityHeaderCom, mIconData), sizeof(PropArray<unsigned char>));
    ArrayMetadata& iconDataMetadata = static_cast<ArrayMetadata&>(*iconData.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(iconData, kPropertyUInt8, sizeof(unsigned char));
    iconDataMetadata.mReadOnly = true;
    iconDataMetadata.mIsHidden = true;

    // A view of the entity's poll order: its size and ten positions from a
    // start index the editor sets.
    GroupMetadata& pollOrder = static_cast<GroupMetadata&>(
        *registry.RegisterProp("poll_order", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    pollOrder.mDebug = true;
    pollOrder.mSaves = false;
    const unsigned long sizeType = 0;
    IntMetadata<unsigned long>& size =
        TypeSpecificMetadata(pollOrder.mRegistry.RegisterProp("size", -1, kPropertyUInt64, 0), sizeType);
    static_cast<void>(Symbol("prop"));
    size.mReadOnly = true;
    size.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<unsigned long>(args.mComponent->mObject->mEntity->mPollOrder.size());
    };
    const unsigned long startIndexType = 0;
    IntMetadata<unsigned long>& startIndex = TypeSpecificMetadata(
        pollOrder.mRegistry.RegisterProp("start_index", -1, kPropertyUInt64, 0), startIndexType);
    static_cast<void>(Symbol("prop"));
    startIndex.mGetter = [](const PropAccessorArgs& args) {
        static_cast<void>(args);
        return gPollOrderStartIndex;
    };
    startIndex.mSetter = [](const PropAccessorArgs& args) {
        gPollOrderStartIndex = *static_cast<const unsigned long*>(args.mValue);
    };
    RegisterPollOrderObject<0>(pollOrder.mRegistry, "object_0");
    RegisterPollOrderObject<1>(pollOrder.mRegistry, "object_1");
    RegisterPollOrderObject<2>(pollOrder.mRegistry, "object_2");
    RegisterPollOrderObject<3>(pollOrder.mRegistry, "object_3");
    RegisterPollOrderObject<4>(pollOrder.mRegistry, "object_4");
    RegisterPollOrderObject<5>(pollOrder.mRegistry, "object_5");
    RegisterPollOrderObject<6>(pollOrder.mRegistry, "object_6");
    RegisterPollOrderObject<7>(pollOrder.mRegistry, "object_7");
    RegisterPollOrderObject<8>(pollOrder.mRegistry, "object_8");
    RegisterPollOrderObject<9>(pollOrder.mRegistry, "object_9");

    metadata.mDescription = "Common specifications for Entities";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mAllowedResources.push_back(EntityResource::Id());
    metadata.mCategory = ComMetaData::kCategoryMiscellaneous;
    metadata.mEditorRestrictions = 0;
    metadata.mLightweight = true;
    metadata.mAliases.push_back(Symbol("EntityDefinitionCom"));
    metadata.mRootOnly = true;
}

// Inlined into _Imprint at 0x1D58C2: the properties up to "icon_data" are
// copied as bytes.
EntityHeaderCom::EntityHeaderCom(const EntityHeaderCom& other)
    : Component(other),
      mNumInstances(other.mNumInstances),
      mNumMeshes(other.mNumMeshes),
      mNumParticles(other.mNumParticles),
      mNumPropanims(other.mNumPropanims),
      mNumLights(other.mNumLights),
      mNumVerts(other.mNumVerts),
      mNumFaces(other.mNumFaces),
      mChangelist(other.mChangelist),
      mIconCamInitialized(other.mIconCamInitialized),
      mIconCamXfm(other.mIconCamXfm),
      mIconCamNear(other.mIconCamNear),
      mIconCamFar(other.mIconCamFar),
      mInstancePollingMode(other.mInstancePollingMode),
      mCopyOnInstance(other.mCopyOnInstance),
      mDrivesParent(other.mDrivesParent),
      mStatic(other.mStatic),
      mCapabilities(other.mCapabilities),
      mShowEntity(other.mShowEntity),
      mIconData(),
      mRuntime() {
    mIconData._Copy(other.mIconData);
}

// Reconstructed from eboot.elf at 0x1D51B0.
bool EntityHeaderCom::CopyOnInstance() const {
    return mCopyOnInstance;
}

// Reconstructed from eboot.elf at 0x1D51C0. A "static" entity's resource
// suppresses its poll and is asked to poll once.
bool EntityHeaderCom::_OnResourcesLoaded() {
    EntityResource* const resource = mObject->mEntity->mResource;
    if (resource == nullptr || !resource->IsA(TransEntityResource::Id())) {
        return true;
    }
    if (mIconData.mSize != 0) {
        resource->SetIconData(mIconData.mData, mIconData.mSize);
        mIconData.Resize(0);
    }
    static_cast<TransEntityResource*>(resource)->mSuppressPoll = mStatic;
    if (mStatic) {
        mRuntime.mPollRequested = true;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x1D5800.
Symbol EntityHeaderCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x1D5810.
Symbol EntityHeaderCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x1D5820.
int EntityHeaderCom::CurrentRev() const {
    return const_cast<EntityHeaderCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x1D5840.
bool EntityHeaderCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x1D5870.
Component* EntityHeaderCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x1D5880. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* EntityHeaderCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<EntityHeaderCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) EntityHeaderCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x1D5A20.
PropRegistry& EntityHeaderCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x1D5A30.
ComMetaData& EntityHeaderCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x1D5D00, the body slot 0 (0x1D50E0)
// jumps to. The members free the layer state and the icon data.
EntityHeaderCom::~EntityHeaderCom() {}

// Reconstructed from eboot.elf at 0x112DFC0, emitted with the class's
// Init.
Component* EntityHeaderCom::_Create() {
    return new EntityHeaderCom();
}

// Reconstructed from eboot.elf at 0x112A480.
void EntityHeaderCom::Init() {
    InitComponentClass<EntityHeaderCom>();
}
