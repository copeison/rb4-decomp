#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;
class ResourcePath;

// The outfit pieces a profile unlocked, owns and has not viewed yet in the
// Rock Shop (rb_meta/RBRockShopPersistentData.o, 0xD62D50-0xD650EF), kept as
// the pieces' short names (RBOutfitPieceLoader::GetShortNameForPiece). A
// piece and its other gender's version are unlocked, awarded and viewed
// together. The vtable at 0x197DF90 has PersistentData's 7 slots; the object
// is 200 bytes. The profile keeps one (RBProfile::mRockShopData).
class RBRockShopPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBRockShopPersistentData(RBProfile* profile);  // 0xD62D50
    // Slots 0-1: 0xD62EC0, 0xD62F10.
    ~RBRockShopPersistentData() override;

    // Slot 5: the owned, unlocked and unviewed pieces in a revision-3 block.
    void Save(BinStream& stream) const override;  // 0xD63090
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD63250

    // The profile's platform name for logs, or "NULL_PROFILE_POINTER!!!".
    const char* _GetProfileName() const;  // 0xD62F70
    // Forgets the pieces without marking the data dirty.
    void Clear();  // 0xD62F90

    // Owned pieces count as unlocked.
    bool OutfitPieceIsUnlocked(const ResourcePath& piece) const;  // 0xD634A0
    bool OutfitPieceIsOwned(const ResourcePath& piece) const;     // 0xD635D0
    // False for an unknown piece.
    bool OutfitPieceHasBeenViewed(const ResourcePath& piece) const;  // 0xD63670
    // Unlocks the piece and marks it unviewed, unless it is owned. False for
    // an unknown piece. The map has a void return.
    bool UnlockOutfitPiece(const ResourcePath& piece);  // 0xD63710
    // A piece the game gave: owned, unviewed, and logged for the
    // achievements. The map has a void return.
    bool AwardOutfitPiece(const ResourcePath& piece);  // 0xD63B40
    // A piece the player bought: owned. The map has a void return.
    bool PurchaseOutfitPiece(const ResourcePath& piece);  // 0xD640D0
    // Moves the piece from the unlocked to the owned pieces; when `awarded`,
    // also marks it unviewed and calls the profile's achievement interface
    // (RBProfile::mAchData at offset 24, slot 22, which unlocks trophy 33:
    // the map's RBAchPersistentData_PS4::LogRockShopItemAwarded). Not
    // reconstructed: the achievement interface is not modelled.
    bool _AwardOutfitPiece(const ResourcePath& piece, const bool& awarded);  // 0xD63B80
    // Debug versions of the unlock and the award that take the short name
    // and log the outcome (the logs are compiled out); both forget the
    // unviewed mark first.
    bool TestUnlockOutfitPiece(const Symbol& piece);  // 0xD64110
    bool TestAwardOutfitPiece(const Symbol& piece);   // 0xD64400
    // Clears the piece's and its gender swap's unviewed marks. False for an
    // unknown piece.
    bool MarkPieceAsViewed(const ResourcePath& piece);  // 0xD64720
    // The owned pieces, counting a piece and its gender swap once.
    unsigned long CalculateAndReturnUniquePiecesOwned() const;  // 0xD64A50
    bool AnyUnviewedPieces() const;  // 0xD64D10
    void GetUnviewedPieces(eastl::vector<Symbol>& pieces) const;  // 0xD64D20

    // Field names are not in the reference map.
    RBProfile* mProfile;
    eastl::set<Symbol> mUnlockedPieces;
    eastl::set<Symbol> mOwnedPieces;
    eastl::set<Symbol> mUnviewedPieces;
};

static_assert(offsetof(RBRockShopPersistentData, mProfile) == 24);
static_assert(offsetof(RBRockShopPersistentData, mUnlockedPieces) == 32);
static_assert(offsetof(RBRockShopPersistentData, mOwnedPieces) == 88);
static_assert(offsetof(RBRockShopPersistentData, mUnviewedPieces) == 144);
static_assert(sizeof(RBRockShopPersistentData) == 200);
