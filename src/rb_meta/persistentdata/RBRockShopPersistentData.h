#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/containers/Set.h"

class BinStream;
class RBProfile;
class ResourcePath;

// The outfit pieces a profile unlocked, owns and viewed in the Rock Shop
// (rb_meta/RBRockShopPersistentData.o, 0xD62D50-0xD650EF). The vtable at
// 0x197DF90 has PersistentData's 7 slots; the object is 200 bytes. Declared
// only: the object is not reconstructed.
class RBRockShopPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBRockShopPersistentData(RBProfile* profile);  // 0xD62D50
    // Slots 0-1: 0xD62EC0, 0xD62F10.
    ~RBRockShopPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD63090
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD63250

    // The profile's platform name for logs, or "NULL_PROFILE_POINTER!!!".
    const char* _GetProfileName() const;  // 0xD62F70
    // Forgets the pieces.
    void Clear();  // 0xD62F90

    // RBProfile's forwarders call these, in the map's order.
    bool OutfitPieceIsUnlocked(const ResourcePath& piece) const;  // 0xD634A0
    bool OutfitPieceIsOwned(const ResourcePath& piece) const;     // 0xD635D0
    void UnlockOutfitPiece(const ResourcePath& piece);            // 0xD63710
    void AwardOutfitPiece(const ResourcePath& piece);             // 0xD63B40
    void PurchaseOutfitPiece(const ResourcePath& piece);          // 0xD640D0

    // Field names are not in the reference map. The sets' order follows the
    // map's methods; the evidence for it is weak.
    RBProfile* mProfile;
    eastl::set<unsigned long> mUnlockedPieces;
    eastl::set<unsigned long> mOwnedPieces;
    eastl::set<unsigned long> mViewedPieces;
};

static_assert(offsetof(RBRockShopPersistentData, mProfile) == 24);
static_assert(offsetof(RBRockShopPersistentData, mUnlockedPieces) == 32);
static_assert(offsetof(RBRockShopPersistentData, mViewedPieces) == 144);
static_assert(sizeof(RBRockShopPersistentData) == 200);
