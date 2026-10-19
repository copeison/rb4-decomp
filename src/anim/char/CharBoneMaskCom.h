#pragma once

#include <cstddef>

#include "anim/char/CharBoneChannelMask.h"
#include "anim/skeleton/CharSkeletonResource.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// The component on a bone mask's root object (anim/CharBoneMaskCom.o,
// 0x2E1B00-0x2E4C0F; class id "BoneMask", alias "CharBoneMaskCom"): "Com
// that contains a static CharBoneMask and its meta data". It keeps one
// CharBoneChannelMask per bone of its skeleton file, in the skeleton's
// order, with the subtree and blend settings a bone mask applies and the
// bones that copy other bones ("constraints"). The vtable at 0x18F5508 has
// 41 slots. The object is 192 bytes. The map's methods take the object
// (ObjPtr const&) first; this build's do not. Field names follow the
// registry.
class CharBoneMaskCom : public Component {
public:
    // A bone that follows another exactly: "The first bone will be exactly
    // follow the second bone, allows you to do this here rather than in
    // Max. The source must be a skeleton bone, the target can just be a
    // bone in the file". 16 bytes.
    struct Constraint {
        // "source", one of the skeleton's bones.
        Symbol mSource;
        // "target".
        Symbol mTarget;
    };

    CharBoneMaskCom();  // 0x2E1B00
    // Copies the component for an imprint; the skeleton and
    // "hide_inactive" are not copied. Inlined into _Imprint (0x2E3900).
    CharBoneMaskCom(const CharBoneMaskCom& other)
        : Component(other),
          mSkeletonFile(other.mSkeletonFile),
          mSubtreeIndex(other.mSubtreeIndex),
          mBlendMode(other.mBlendMode),
          mBlendFromRoot(other.mBlendFromRoot),
          mBoneChannelMasks(other.mBoneChannelMasks),
          mConstraints(other.mConstraints),
          mPreviewChars(other.mPreviewChars),
          mHarvestAnimsUnder(other.mHarvestAnimsUnder),
          mSkeleton(),
          mHideInactive(false) {}
    // Slots 0-1: 0x2E1C00, 0x2E1D70.
    ~CharBoneMaskCom() override;

    Symbol GetId() const override;          // slot 4: 0x2E3880
    Symbol GetClassName() const override;   // slot 5: 0x2E3890
    int CurrentRev() const override;        // slot 7: 0x2E38A0
    bool IsA(Symbol type) const override;   // slot 8: 0x2E38C0
    Component* AsComponent() override;      // slot 9: 0x2E38F0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2E3900
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2E3B20
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2E3B30
    // Slot 29. Loads the skeleton file (inlined into the entity or from
    // disk) and rebuilds the masks to match its bones, keeping the masks of
    // the bones that remain; without a skeleton the masks are cleared. The
    // map's _LoadResources(ObjPtr const&) has this body.
    bool _OnResourcesLoaded() override;  // 0x2E33D0
    // Slots 31 and 33: empty. The map's _Enter and _Poll.
    void _Enter() override;  // 0x2E37B0
    void _Poll() override;   // 0x2E37C0
    // Slot 38: polls as in the game mode (slot 33). The map's
    // _EditPoll(ObjPtr const&).
    void _EditPoll() override;  // 0x2E3B40

    // The loaded skeleton resource; the bone mask builders read it (callers
    // at 0x2E0594 and 0x32D470). Name not in the reference map.
    CharSkeletonResource* GetSkeletonResource() const;  // 0x2E37A0
    // The "disable_all" and "enable_all" actions: mask every channel of
    // every bone, or keep each bone's position, full rotation, scale and
    // inverse parent scale. The map's signatures take an ObjPtr const&.
    void _DisableAll();  // 0x2E37D0
    void _EnableAll();   // 0x2E3820
    // The "harvest_preview_chars" action; empty in this build. The map's
    // HarvestPreviewChars(ObjPtr const&).
    void HarvestPreviewChars();  // 0x2E3860

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2AE800.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B5F50; the
    // placement factory beside it is at 0x2B5F80.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2E1D90

    // "BoneMask", constructed by the static initializer at 0x2E4B50, which
    // first sets a shared header's null object id to -1 (0x19F87A8).
    static Symbol sId;  // 0x19F87B0
    // Also "BoneMask". Name not in the reference map.
    static Symbol sClassName;           // 0x19F87B8
    static PropRegistry sPropRegistry;  // 0x19F87C0
    static ComMetaData sMetaData;       // 0x19F8860

    // "skeleton_file", a CharSkeletonResource.
    ResourcePath mSkeletonFile;
    // "subtree_index": "Identifies this as belonging to a subtree, which can
    // have it sown weight. Also higher subtree numbers are blended after
    // lower ones". Revision 0 saved it as the float "blend_order".
    unsigned long mSubtreeIndex;
    // "blend_mode", an AnimResource::BlendMode.
    int mBlendMode;
    // "blend_from_root": "Do we blend relative to the char_root, rather
    // than absolute".
    bool mBlendFromRoot;
    // "bone_channel_masks", one per skeleton bone.
    PropArray<CharBoneChannelMask> mBoneChannelMasks;
    // "constraints".
    PropArray<Constraint> mConstraints;
    // "preview_chars": "Paths pointing to the chars we can use as preview
    // characters, they must use this bonemask" (CharResource files).
    PropArray<ResourcePath> mPreviewChars;
    // "harvest_anims_under": "If true, only look for anims under our
    // directory, by convention per project".
    bool mHarvestAnimsUnder;
    // The loaded "skeleton_file". Name not in the reference map.
    ResourcePtr<CharSkeletonResource> mSkeleton;
    // "hide_inactive": "If true, hides the inactive bones, ones where no DOF
    // are captured." Not saved.
    bool mHideInactive;
};

static_assert(sizeof(CharBoneMaskCom::Constraint) == 16);
static_assert(offsetof(CharBoneMaskCom, mSkeletonFile) == 24);
static_assert(offsetof(CharBoneMaskCom, mSubtreeIndex) == 32);
static_assert(offsetof(CharBoneMaskCom, mBlendMode) == 40);
static_assert(offsetof(CharBoneMaskCom, mBlendFromRoot) == 44);
static_assert(offsetof(CharBoneMaskCom, mBoneChannelMasks) == 48);
static_assert(offsetof(CharBoneMaskCom, mConstraints) == 88);
static_assert(offsetof(CharBoneMaskCom, mPreviewChars) == 128);
static_assert(offsetof(CharBoneMaskCom, mHarvestAnimsUnder) == 168);
static_assert(offsetof(CharBoneMaskCom, mSkeleton) == 176);
static_assert(offsetof(CharBoneMaskCom, mHideInactive) == 184);
static_assert(sizeof(CharBoneMaskCom) == 192);
