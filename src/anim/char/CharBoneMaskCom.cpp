// anim/CharBoneMaskCom.o (0x2E1B00 to 0x2E4C0F). The struct types'
// registrations (the map's PropArray<CharBoneChannelMask>::Init(Symbol) and
// PropArray<CharBoneMaskCom::Constraint>::Init(Symbol), 0x2E3070 and
// 0x2E3220), the property handlers after the identity slots
// (0x2E3B50-0x2E49FF) and Resource::GetOrLoad<CharSkeletonResource>
// (0x2E4A00) are the compiler's. The empty function at 0x2E3870 has no
// caller and is not identified. The object's static initializer is at
// 0x2E4B50. The class's registration and factory are emitted in
// anim/AnimInit.o.
#include "anim/char/CharBoneMaskCom.h"

#include <functional>
#include <new>

#include "anim/char/CharBoneMaskResource.h"
#include "anim/core/AnimResource.h"
#include "anim/skeleton/CharSkeleton.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "os/memory/MemMgr.h"
#include "render/scene/CharResource.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The -1 of a shared header that the static initializer sets (0x19F87A8).
// Nothing in this object reads it. Name not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The index of the array element the tag's path ends in. Name not in the
// reference map.
unsigned long ElementIndex(const TagArgs& args) {
    return args.mPath[-1].mValue;
}

// The component a tag or an action belongs to. Name not in the reference
// map.
CharBoneMaskCom& BoneMask(const TagArgs& args) {
    return const_cast<CharBoneMaskCom&>(static_cast<const CharBoneMaskCom&>(args.mComponent));
}

// Reads a property of the component that holds the properties as they were
// saved into `value`; a missing or unconvertible property keeps the value.
// The release build keeps only the arguments of the error it reports.
// Inlined into the revision handler. Name not in the reference map.
template <typename T>
void ReadSavedProp(const Component& saved, const PropPath& path, T& value, PropertyType type) {
    void* storage = const_cast<Component*>(&saved);
    const PropInfo* const info = const_cast<Component&>(saved)._GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _GetProp(saved, path, *info, storage, &value, type)) {
        return;
    }
    static_cast<void>(saved.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

}  // namespace

// The object's statics, in the order of its static initializer (0x2E4B50).
Symbol CharBoneMaskCom::sId("BoneMask");
Symbol CharBoneMaskCom::sClassName("BoneMask");
PropRegistry CharBoneMaskCom::sPropRegistry;
ComMetaData CharBoneMaskCom::sMetaData;

// Reconstructed from eboot.elf at 0x2E1B00.
CharBoneMaskCom::CharBoneMaskCom()
    : mSkeletonFile(),
      mSubtreeIndex(0),
      mBlendMode(AnimResource::kBlendAdditive),
      mBlendFromRoot(false),
      mBoneChannelMasks(),
      mConstraints(),
      mPreviewChars(),
      mHarvestAnimsUnder(false),
      mSkeleton(),
      mHideInactive(false) {}

// Reconstructed from eboot.elf at 0x2E1C00. The deleting destructor is at
// 0x2E1D70.
CharBoneMaskCom::~CharBoneMaskCom() {}

// Reconstructed from eboot.elf at 0x2E1D90. The registry is at revision 1;
// revision 0 saved "subtree_index" as the float "blend_order".
void CharBoneMaskCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<CharBoneChannelMask>(Symbol("CharBoneChannelMask"));
    RegisterPropStruct<Constraint>(Symbol("Constraint"));
    CharBoneMaskCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    ActionMetadata& disableAll = static_cast<ActionMetadata&>(*registry.RegisterAction("disable_all").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    disableAll.mHelp = String("Disables all the bone channels.");
    disableAll.mAction = [](const TagArgs& args) { BoneMask(args)._DisableAll(); };

    ActionMetadata& enableAll = static_cast<ActionMetadata&>(*registry.RegisterAction("enable_all").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    enableAll.mHelp = String("Enables pos and full rotation on all the bone channels.");
    enableAll.mAction = [](const TagArgs& args) { BoneMask(args)._EnableAll(); };

    BoolMetadata& hideInactive = TypeSpecificMetadata(
        registry.RegisterProp("hide_inactive", offsetof(CharBoneMaskCom, mHideInactive), kPropertyBool, sizeof(bool)),
        prototype.mHideInactive);
    hideInactive.mDefault = prototype.mHideInactive;
    static_cast<void>(Symbol("prop"));
    hideInactive.mHelp = String("If true, hides the inactive bones, ones where no DOF are captured.");
    hideInactive.mSaves = false;
    hideInactive.mHistory = false;

    PropInfo& masks = registry.RegisterPropArray(
        "bone_channel_masks",
        offsetof(CharBoneMaskCom, mBoneChannelMasks),
        sizeof(PropArray<CharBoneChannelMask>));
    ArrayMetadata& masksMetadata = static_cast<ArrayMetadata&>(*masks.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(masks, kPropertyStruct, sizeof(CharBoneChannelMask));
    GroupMetadata& mask = static_cast<GroupMetadata&>(*masksMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    {
        const CharBoneChannelMask maskPrototype;
        mask.mKind = 1;
        // The help names the bone's parent in the skeleton.
        mask.mHelp._Set([](const TagArgs& args) {
            const CharSkeleton* const skeleton = BoneMask(args).mSkeleton->mSkeleton;
            const Symbol parent = skeleton->mNames[skeleton->mParentIndices[ElementIndex(args)]];
            return String(MakeString(
                "Name, Position, Rotation, Scale, and InverseParentScale.  Parent bone is %s.", parent));
        });
        // With "hide_inactive", the masks that keep no channel are hidden.
        mask.mIsHidden._Set([](const TagArgs& args) {
            const CharBoneMaskCom& component = BoneMask(args);
            if (!component.mHideInactive) {
                return false;
            }
            const CharBoneChannelMask& element = component.mBoneChannelMasks[ElementIndex(args)];
            return !element.mUsePosition && !element.mUseScale && !element.mUseInverseParentScale
                && element.mRotation == CharBoneChannelMask::kRotationNone;
        });
        mask.mDisplayName._Set([](const TagArgs& args) {
            return String(BoneMask(args).mBoneChannelMasks[ElementIndex(args)].mName.Str());
        });

        PropRegistry& members = mask.mRegistry;
        SymbolMetadata& name = TypeSpecificMetadata(
            members.RegisterProp("name", offsetof(CharBoneChannelMask, mName), kPropertySymbol, sizeof(Symbol)),
            maskPrototype.mName);
        name.mDefault = maskPrototype.mName;
        static_cast<void>(Symbol("prop"));
        name.mIsHidden = true;

        BoolMetadata& usePosition = TypeSpecificMetadata(
            members.RegisterProp(
                "use_position", offsetof(CharBoneChannelMask, mUsePosition), kPropertyBool, sizeof(bool)),
            maskPrototype.mUsePosition);
        usePosition.mDefault = maskPrototype.mUsePosition;

        IntMetadata<int>& rotation = static_cast<IntMetadata<int>&>(
            *members.RegisterProp("rotation", offsetof(CharBoneChannelMask, mRotation), kPropertyInt, sizeof(int))
                 .mMetadata.Get());
        rotation.mDefault = static_cast<int>(maskPrototype.mRotation);
        static_cast<void>(Symbol("prop"));
        rotation.mAllowedValues = CharBoneChannelMask::GetRotationAllowedValues();

        BoolMetadata& useScale = TypeSpecificMetadata(
            members.RegisterProp("use_scale", offsetof(CharBoneChannelMask, mUseScale), kPropertyBool, sizeof(bool)),
            maskPrototype.mUseScale);
        useScale.mDefault = maskPrototype.mUseScale;

        BoolMetadata& useInverseParentScale = TypeSpecificMetadata(
            members.RegisterProp(
                "use_inverse_parent_scale",
                offsetof(CharBoneChannelMask, mUseInverseParentScale),
                kPropertyBool,
                sizeof(bool)),
            maskPrototype.mUseInverseParentScale);
        useInverseParentScale.mDefault = maskPrototype.mUseInverseParentScale;
    }

    ResourcePathMetadata& skeletonFile = TypeSpecificMetadata(
        registry.RegisterProp(
            "skeleton_file", offsetof(CharBoneMaskCom, mSkeletonFile), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mSkeletonFile);
    skeletonFile.mDefault = prototype.mSkeletonFile;
    static_cast<void>(Symbol("prop"));
    skeletonFile.mResourceId = CharSkeletonResource::Id();

    IntMetadata<unsigned long>& subtreeIndex = TypeSpecificMetadata(
        registry.RegisterProp(
            "subtree_index", offsetof(CharBoneMaskCom, mSubtreeIndex), kPropertyUInt64, sizeof(unsigned long)),
        prototype.mSubtreeIndex);
    subtreeIndex.mDefault = prototype.mSubtreeIndex;
    static_cast<void>(Symbol("prop"));
    subtreeIndex.mHelp = String(
        "Identifies this as belonging to a subtree, which can have it sown weight.  Also higher subtree numbers are "
        "blended after lower ones");

    IntMetadata<int>& blendMode = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("blend_mode", offsetof(CharBoneMaskCom, mBlendMode), kPropertyInt, sizeof(int))
             .mMetadata.Get());
    blendMode.mDefault = prototype.mBlendMode;
    static_cast<void>(Symbol("prop"));
    blendMode.mAllowedValues = AnimResource::GetBlendModeAllowedValues();

    BoolMetadata& blendFromRoot = TypeSpecificMetadata(
        registry.RegisterProp(
            "blend_from_root", offsetof(CharBoneMaskCom, mBlendFromRoot), kPropertyBool, sizeof(bool)),
        prototype.mBlendFromRoot);
    blendFromRoot.mDefault = prototype.mBlendFromRoot;
    static_cast<void>(Symbol("prop"));
    blendFromRoot.mHelp = String("Do we blend relative to the char_root, rather than absolute");

    PropInfo& constraints = registry.RegisterPropArray(
        "constraints", offsetof(CharBoneMaskCom, mConstraints), sizeof(PropArray<Constraint>));
    ArrayMetadata& constraintsMetadata = static_cast<ArrayMetadata&>(*constraints.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(constraints, kPropertyStruct, sizeof(Constraint));
    GroupMetadata& constraint = static_cast<GroupMetadata&>(*constraintsMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const Constraint constraintPrototype = {};
    constraint.mKind = 1;
    constraint.mHelp = String(
        "The first bone will be exactly follow the second bone, allows you to do this here rather than in Max.  The "
        "source must be a skeleton bone, the target can just be a bone in the file");
    SymbolMetadata& source = TypeSpecificMetadata(
        constraint.mRegistry.RegisterProp("source", offsetof(Constraint, mSource), kPropertySymbol, sizeof(Symbol)),
        constraintPrototype.mSource);
    source.mDefault = constraintPrototype.mSource;
    static_cast<void>(Symbol("prop"));
    source.mAllowedValues._Set([](const TagArgs& args) {
        return BoneMask(args).mSkeleton->mSkeleton->GetAllowedValueBones(true);
    });
    SymbolMetadata& target = TypeSpecificMetadata(
        constraint.mRegistry.RegisterProp("target", offsetof(Constraint, mTarget), kPropertySymbol, sizeof(Symbol)),
        constraintPrototype.mTarget);
    target.mDefault = constraintPrototype.mTarget;
    static_cast<void>(Symbol("prop"));

    BoolMetadata& harvestAnimsUnder = TypeSpecificMetadata(
        registry.RegisterProp(
            "harvest_anims_under", offsetof(CharBoneMaskCom, mHarvestAnimsUnder), kPropertyBool, sizeof(bool)),
        prototype.mHarvestAnimsUnder);
    harvestAnimsUnder.mDefault = prototype.mHarvestAnimsUnder;
    static_cast<void>(Symbol("prop"));
    harvestAnimsUnder.mHelp = String("If true, only look for anims under our directory, by convention per project");

    PropInfo& previewChars = registry.RegisterPropArray(
        "preview_chars", offsetof(CharBoneMaskCom, mPreviewChars), sizeof(PropArray<ResourcePath>));
    ArrayMetadata& previewCharsMetadata = static_cast<ArrayMetadata&>(*previewChars.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    previewCharsMetadata.mHelp =
        String("Paths pointing to the chars we can use as preview characters, they must use this bonemask");
    PropRegistry::RegisterPropArrayItem(previewChars, kPropertyResourcePath, sizeof(ResourcePath));
    const ResourcePath previewCharType;
    ResourcePathMetadata& previewChar = TypeSpecificMetadata(previewCharsMetadata.mItemInfo, previewCharType);
    static_cast<void>(Symbol("item"));
    previewChar.mResourceId = CharResource::Id();
    previewChar.mAllowsInlined = false;

    ActionMetadata& harvestPreviewChars =
        static_cast<ActionMetadata&>(*registry.RegisterAction("harvest_preview_chars").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    harvestPreviewChars.mHelp = String("Harvests all viable preview chars.");
    harvestPreviewChars.mAction = [](const TagArgs& args) { BoneMask(args).HarvestPreviewChars(); };

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev > 0) {
            return;
        }
        float blendOrder = 0.0F;
        ReadSavedProp(*args.mSavedProps, PropPath(Symbol("blend_order")), blendOrder, kPropertyFloat);
        static_cast<CharBoneMaskCom*>(args.mComponent)->mSubtreeIndex = static_cast<unsigned long>(blendOrder);
    };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
    }

    metadata.mDescription = "Com that contains a static CharBoneMask and its meta data.";
    metadata.mAuthor = "James Fleming";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(CharBoneMaskResource::Id());
    metadata.mEditorRestrictions = 0;
    metadata.mAliases.push_back(Symbol("CharBoneMaskCom"));
}

// Reconstructed from eboot.elf at 0x2E33D0. A skeleton already inlined into
// the entity is preferred to loading the file. The masks stay as they are
// while their names follow the skeleton's bones; otherwise each bone gets
// its old mask or a disabled one, built in a temporary array.
bool CharBoneMaskCom::_OnResourcesLoaded() {
    if (mSkeletonFile.mPath != Symbol()) {
        ResourcePath path;
        path = mSkeletonFile.Str();
        ResourcePtr<CharSkeletonResource> skeleton =
            mObject->mEntity->mResource->TryGetInline<CharSkeletonResource>(path);
        if (skeleton == nullptr) {
            skeleton = Resource::GetOrLoad<CharSkeletonResource>(path, false);
        }
        mSkeleton = static_cast<ResourcePtr<CharSkeletonResource>&&>(skeleton);
        if (mSkeleton != nullptr && !mSkeleton->Fail() && mSkeleton->mSkeleton != nullptr) {
            const CharSkeleton* const bones = mSkeleton->mSkeleton;
            const unsigned long count = bones->mNames.size();
            const unsigned long oldCount = mBoneChannelMasks.size();
            if (count == oldCount) {
                unsigned long index = 0;
                while (index < count && mBoneChannelMasks[index].mName == bones->mNames[index]) {
                    ++index;
                }
                if (index == count) {
                    return true;
                }
            }
            CharBoneChannelMask disabled;
            PropArray<CharBoneChannelMask> masks;
            disabled.SetDisable();
            unsigned int temp;
            MemPushTemp(temp, true, true);
            masks.Resize(count);
            MemPopTemp(temp);
            for (unsigned long index = 0; index < count; ++index) {
                const Symbol name = bones->mNames[index];
                unsigned long old = 0;
                while (old < oldCount && mBoneChannelMasks[old].mName != name) {
                    ++old;
                }
                if (old < oldCount) {
                    masks[index] = mBoneChannelMasks[old];
                } else {
                    masks[index] = disabled;
                    masks[index].mName = name;
                }
            }
            mBoneChannelMasks.Resize(0);
            mBoneChannelMasks.Reserve(static_cast<unsigned int>(count));
            // The map's PropArray<CharBoneChannelMask>::PushBack.
            for (unsigned long index = 0; index < count; ++index) {
                mBoneChannelMasks._Insert(mBoneChannelMasks.size(), &masks[index]);
            }
            return true;
        }
    }
    mBoneChannelMasks.Resize(0);
    return true;
}

// Reconstructed from eboot.elf at 0x2E37A0.
CharSkeletonResource* CharBoneMaskCom::GetSkeletonResource() const {
    return mSkeleton;
}

// Reconstructed from eboot.elf at 0x2E37B0.
void CharBoneMaskCom::_Enter() {}

// Reconstructed from eboot.elf at 0x2E37C0.
void CharBoneMaskCom::_Poll() {}

// Reconstructed from eboot.elf at 0x2E37D0.
void CharBoneMaskCom::_DisableAll() {
    for (unsigned long index = 0; index < mBoneChannelMasks.size(); ++index) {
        mBoneChannelMasks[index].SetDisable();
    }
}

// Reconstructed from eboot.elf at 0x2E3820.
void CharBoneMaskCom::_EnableAll() {
    for (unsigned long index = 0; index < mBoneChannelMasks.size(); ++index) {
        mBoneChannelMasks[index].SetSRTIPS();
    }
}

// Reconstructed from eboot.elf at 0x2E3860.
void CharBoneMaskCom::HarvestPreviewChars() {}

// Reconstructed from eboot.elf at 0x2E3880.
Symbol CharBoneMaskCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2E3890.
Symbol CharBoneMaskCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2E38A0.
int CharBoneMaskCom::CurrentRev() const {
    return const_cast<CharBoneMaskCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2E38C0.
bool CharBoneMaskCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2E38F0.
Component* CharBoneMaskCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2E3900. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CharBoneMaskCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CharBoneMaskCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CharBoneMaskCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2E3B20.
PropRegistry& CharBoneMaskCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2E3B30.
ComMetaData& CharBoneMaskCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2E3B40.
void CharBoneMaskCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x2B5F50. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* CharBoneMaskCom::_Create() {
    return new CharBoneMaskCom();
}

// Reconstructed from eboot.elf at 0x2AE800. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharBoneMaskCom::Init() {
    InitComponentClass<CharBoneMaskCom>();
}
