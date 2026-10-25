// LeaderboardListItemCom's object (0xDB8740 to 0xDB937F; newer than the
// map). The "row_type" accessors are the lambdas after the identity slots
// (0xDB92A0, 0xDB92B0). The static initializer is at 0xDB92C0. The class's
// registration and factory are emitted elsewhere (0xB64BE0, 0xBA0D10).
#include "rb_meta/leaderboards/LeaderboardListItemCom.h"

#include <new>

#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndEntityResource.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1B26588, which the static initializer sets.
// Each object that uses it has its own copy. Name not in the reference
// map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

}  // namespace

// The object's statics, in the order of its static initializer (0xDB92C0).
Symbol LeaderboardListItemCom::sId("LeaderboardListItemCom");
Symbol LeaderboardListItemCom::sClassName("LeaderboardListItemCom");
PropRegistry LeaderboardListItemCom::sPropRegistry;
ComMetaData LeaderboardListItemCom::sMetaData;

// Reconstructed from eboot.elf at 0xDB8740.
LeaderboardListItemCom::LeaderboardListItemCom()
    : mFirstAmongFriends(false),
      mRank(gNullStr),
      mName(gNullStr),
      mPercentage(gNullStr),
      mDifficulty(gNullStr),
      mScore(0),
      mFullCombo(0.0f),
      mPlatformUid(gNullStr),
      mRowType(kRowTypeRank),
      mShowPercent(true),
      mShowDifficulty(true),
      mShowFullCombo(true),
      mIsYou(false) {}

// Inlined into _Imprint.
LeaderboardListItemCom::LeaderboardListItemCom(const LeaderboardListItemCom& other)
    : Component(other),
      mFirstAmongFriends(false),
      mRank(gNullStr),
      mName(gNullStr),
      mPercentage(gNullStr),
      mDifficulty(gNullStr),
      mScore(0),
      mFullCombo(0.0f),
      mPlatformUid(gNullStr),
      mRowType(kRowTypeRank),
      mShowPercent(true),
      mShowDifficulty(true),
      mShowFullCombo(true),
      mIsYou(false) {}

// Reconstructed from eboot.elf at 0xDB87D0. The deleting destructor is at
// 0xDB87E0.
LeaderboardListItemCom::~LeaderboardListItemCom() {}

// Reconstructed from eboot.elf at 0xDB8800.
void LeaderboardListItemCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Component for items in the leaderboard list on song leaderboards.";
    metadata.mAuthor = "Kenny Sun";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndEntityResource::Id());

    LeaderboardListItemCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }

    TypeSpecificMetadata(
        registry.RegisterProp(
            "first_among_friends", offsetof(LeaderboardListItemCom, mFirstAmongFriends), kPropertyBool, sizeof(bool)),
        prototype.mFirstAmongFriends)
        .mDefault = prototype.mFirstAmongFriends;
    TypeSpecificMetadata(
        registry.RegisterProp("rank", offsetof(LeaderboardListItemCom, mRank), kPropertySymbol, sizeof(Symbol)),
        prototype.mRank)
        .mDefault = prototype.mRank;
    TypeSpecificMetadata(
        registry.RegisterProp("name", offsetof(LeaderboardListItemCom, mName), kPropertySymbol, sizeof(Symbol)),
        prototype.mName)
        .mDefault = prototype.mName;
    TypeSpecificMetadata(
        registry.RegisterProp(
            "percentage", offsetof(LeaderboardListItemCom, mPercentage), kPropertySymbol, sizeof(Symbol)),
        prototype.mPercentage)
        .mDefault = prototype.mPercentage;
    TypeSpecificMetadata(
        registry.RegisterProp(
            "difficulty", offsetof(LeaderboardListItemCom, mDifficulty), kPropertySymbol, sizeof(Symbol)),
        prototype.mDifficulty)
        .mDefault = prototype.mDifficulty;
    TypeSpecificMetadata(
        registry.RegisterProp("score", offsetof(LeaderboardListItemCom, mScore), kPropertyInt, sizeof(int)),
        prototype.mScore)
        .mDefault = prototype.mScore;
    TypeSpecificMetadata(
        registry.RegisterProp(
            "full_combo", offsetof(LeaderboardListItemCom, mFullCombo), kPropertyFloat, sizeof(float)),
        prototype.mFullCombo)
        .mDefault = prototype.mFullCombo;
    TypeSpecificMetadata(
        registry.RegisterProp(
            "platfrom_uid", offsetof(LeaderboardListItemCom, mPlatformUid), kPropertySymbol, sizeof(Symbol)),
        prototype.mPlatformUid)
        .mDefault = prototype.mPlatformUid;

    const int intType = 0;
    IntMetadata<int>& rowType = TypeSpecificMetadata(registry.RegisterProp("row_type", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{kRowTypePrevious, String("kRowTypePrevious"), String("")});
        values.push_back(AllowedValue<int>{kRowTypeNext, String("kRowTypeNext"), String("")});
        values.push_back(AllowedValue<int>{kRowTypeRank, String("kRowTypeRank"), String("")});
        values.push_back(AllowedValue<int>{kRowTypePercentile, String("kRowTypePercentile"), String("")});
        rowType.mAllowedValues = values;
    }
    rowType.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<int>(static_cast<LeaderboardListItemCom*>(args.mComponent)->mRowType);
    };
    rowType.mSetter = [](const PropAccessorArgs& args) {
        static_cast<LeaderboardListItemCom*>(args.mComponent)->mRowType =
            static_cast<RowType>(*static_cast<const int*>(args.mValue));
    };

    TypeSpecificMetadata(
        registry.RegisterProp(
            "show_percent", offsetof(LeaderboardListItemCom, mShowPercent), kPropertyBool, sizeof(bool)),
        prototype.mShowPercent)
        .mDefault = prototype.mShowPercent;
    TypeSpecificMetadata(
        registry.RegisterProp(
            "show_difficulty", offsetof(LeaderboardListItemCom, mShowDifficulty), kPropertyBool, sizeof(bool)),
        prototype.mShowDifficulty)
        .mDefault = prototype.mShowDifficulty;
    TypeSpecificMetadata(
        registry.RegisterProp(
            "show_full_combo", offsetof(LeaderboardListItemCom, mShowFullCombo), kPropertyBool, sizeof(bool)),
        prototype.mShowFullCombo)
        .mDefault = prototype.mShowFullCombo;
    TypeSpecificMetadata(
        registry.RegisterProp("is_you", offsetof(LeaderboardListItemCom, mIsYou), kPropertyBool, sizeof(bool)),
        prototype.mIsYou)
        .mDefault = prototype.mIsYou;
}

// Reconstructed from eboot.elf at 0xDB9070.
Symbol LeaderboardListItemCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xDB9080.
Symbol LeaderboardListItemCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xDB9090.
int LeaderboardListItemCom::CurrentRev() const {
    return const_cast<LeaderboardListItemCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xDB90B0.
bool LeaderboardListItemCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xDB90E0.
Component* LeaderboardListItemCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xDB90F0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* LeaderboardListItemCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<LeaderboardListItemCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) LeaderboardListItemCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xDB9280.
PropRegistry& LeaderboardListItemCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xDB9290.
ComMetaData& LeaderboardListItemCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBA0D10. The binary emits the factory
// with the other rb_meta factories.
Component* LeaderboardListItemCom::_Create() {
    return new LeaderboardListItemCom();
}

// Reconstructed from eboot.elf at 0xB64BE0. The binary emits the
// registration with the other rb_meta classes'.
void LeaderboardListItemCom::Init() {
    InitComponentClass<LeaderboardListItemCom>();
}
