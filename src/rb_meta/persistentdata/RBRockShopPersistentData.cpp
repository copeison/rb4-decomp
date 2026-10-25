// rb_meta/RBRockShopPersistentData.o (0xD62D50 to 0xD650EF). The object
// also holds the revision operations Revision::AsOfOp<eastl::set<Symbol>>
// (vtable 0x197DFD8: 0xD63480, 0xD64EE0, Read 0xD64EF0) and
// Revision::AtOp<eastl::set<Symbol>> (vtable 0x197E000: 0xD63490, 0xD65080,
// Read 0xD65090), inline in utl/streams/RevisionStream.h, and the
// operator>> they read the sets with (0xD64F10). The static initializer at
// 0xD650B0 sets the shared header globals every rb_meta object initializes;
// it is not modelled. The map's DumpPieceLists and SpewUnlockedPieces are
// not in this build.
#include "rb_meta/persistentdata/RBRockShopPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/persistentdata/RBAchPersistentData.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_world/characters/RBOutfitPieceLoader.h"
#include "utl/streams/RevisionStream.h"

namespace {

// Whether a loader lookup found nothing: it answers with the empty symbol.
// Name not in the reference map.
bool IsNullSymbol(const Symbol& symbol) {
    return symbol.Str() == gNullStr;
}

bool HasPiece(const eastl::set<Symbol>& pieces, const Symbol& piece) {
    return pieces.find(piece) != pieces.end();
}

const Symbol& PieceAt(const eastl::rbtree_node_base* node) {
    return static_cast<const eastl::set<Symbol>::node_type*>(node)->mValue;
}

// Writes the count and the pieces.
void SavePieces(BinStream& stream, const eastl::set<Symbol>& pieces) {
    unsigned int count = static_cast<unsigned int>(pieces.size());
    stream.WriteEndian(&count, sizeof(count));
    for (const eastl::rbtree_node_base* node = pieces.mAnchor.mpNodeLeft; node != pieces.end();
         node = eastl::RBTreeIncrement(node)) {
        stream << PieceAt(node);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0xD64F10.
template <typename Key>
BinStream& operator>>(BinStream& stream, eastl::set<Key>& set) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (unsigned int i = 0; i < count; ++i) {
        Key key;
        stream >> key;
        set.insert(key);
    }
    return stream;
}

template BinStream& operator>>(BinStream& stream, eastl::set<Symbol>& set);

// Reconstructed from eboot.elf at 0xD62D50.
RBRockShopPersistentData::RBRockShopPersistentData(RBProfile* profile) : mProfile(profile) {
    mParent = profile;
}

// Reconstructed from eboot.elf at 0xD62EC0. The deleting destructor is at
// 0xD62F10.
RBRockShopPersistentData::~RBRockShopPersistentData() {}

// Reconstructed from eboot.elf at 0xD62F70.
const char* RBRockShopPersistentData::_GetProfileName() const {
    if (mProfile != nullptr) {
        return mProfile->GetPlatformName();
    }
    return "NULL_PROFILE_POINTER!!!";
}

// Reconstructed from eboot.elf at 0xD62F90.
void RBRockShopPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mOwnedPieces.clear();
    mUnlockedPieces.clear();
    mUnviewedPieces.clear();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD63090.
void RBRockShopPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 3, 0);
        revisionStream.BeginWrite();
        SavePieces(revisionStream, mOwnedPieces);
        SavePieces(revisionStream, mUnlockedPieces);
        SavePieces(revisionStream, mUnviewedPieces);
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD63250. Revision 2 had a fourth set,
// which is read and dropped.
void RBRockShopPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 3, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<eastl::set<Symbol>>(&mOwnedPieces, 1);
            revisionStream >> Revision::AsOfOp<eastl::set<Symbol>>(&mUnlockedPieces, 1);
            eastl::set<Symbol> droppedPieces;
            revisionStream >> Revision::AtOp<eastl::set<Symbol>>(&droppedPieces, 2);
            revisionStream >> Revision::AsOfOp<eastl::set<Symbol>>(&mUnviewedPieces, 3);
        } else {
            // The failure report that named the profile is compiled out.
            _GetProfileName();
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD634A0.
bool RBRockShopPersistentData::OutfitPieceIsUnlocked(const ResourcePath& piece) const {
    if (OutfitPieceIsOwned(piece)) {
        return true;
    }
    const Symbol shortName = theOutfitPieceLoader->GetShortNameForPiece(piece);
    if (IsNullSymbol(shortName)) {
        return false;
    }
    return HasPiece(mUnlockedPieces, shortName);
}

// Reconstructed from eboot.elf at 0xD635D0.
bool RBRockShopPersistentData::OutfitPieceIsOwned(const ResourcePath& piece) const {
    const Symbol shortName = theOutfitPieceLoader->GetShortNameForPiece(piece);
    if (IsNullSymbol(shortName)) {
        return false;
    }
    return HasPiece(mOwnedPieces, shortName);
}

// Reconstructed from eboot.elf at 0xD63670.
bool RBRockShopPersistentData::OutfitPieceHasBeenViewed(const ResourcePath& piece) const {
    const Symbol shortName = theOutfitPieceLoader->GetShortNameForPiece(piece);
    if (IsNullSymbol(shortName)) {
        return false;
    }
    return !HasPiece(mUnviewedPieces, shortName);
}

// Reconstructed from eboot.elf at 0xD63710. OutfitPieceIsOwned is inlined.
bool RBRockShopPersistentData::UnlockOutfitPiece(const ResourcePath& piece) {
    if (OutfitPieceIsOwned(piece)) {
        return true;
    }
    const Symbol shortName = theOutfitPieceLoader->GetShortNameForPiece(piece);
    if (IsNullSymbol(shortName)) {
        return false;
    }
    const Symbol genderSwap = theOutfitPieceLoader->GetGenderSwapForOutfit(shortName);
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    MakeDirty();
    mUnlockedPieces.insert(shortName);
    mUnviewedPieces.insert(shortName);
    if (!IsNullSymbol(genderSwap)) {
        mUnlockedPieces.insert(genderSwap);
        mUnviewedPieces.insert(genderSwap);
    }
    MemPopHeap();
    return true;
}

// Reconstructed from eboot.elf at 0xD63B40.
bool RBRockShopPersistentData::AwardOutfitPiece(const ResourcePath& piece) {
    const bool awarded = true;
    return _AwardOutfitPiece(piece, awarded);
}

// Reconstructed from eboot.elf at 0xD63B80.
bool RBRockShopPersistentData::_AwardOutfitPiece(const ResourcePath& piece, const bool& awarded) {
    const Symbol shortName = theOutfitPieceLoader->GetShortNameForPiece(piece);
    if (IsNullSymbol(shortName)) {
        return false;
    }
    const Symbol genderSwap = theOutfitPieceLoader->GetGenderSwapForOutfit(shortName);
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    MakeDirty();
    mUnlockedPieces.erase(shortName);
    mOwnedPieces.insert(shortName);
    if (awarded) {
        mUnviewedPieces.insert(shortName);
    }
    if (!IsNullSymbol(genderSwap)) {
        mUnlockedPieces.erase(genderSwap);
        mOwnedPieces.insert(genderSwap);
        if (awarded) {
            // The original marks the piece again, not its swap.
            mUnviewedPieces.insert(shortName);
        }
    }
    if (awarded) {
        mProfile->mAchData->LogRockShopItemAwarded();
    }
    MemPopHeap();
    return true;
}

// Reconstructed from eboot.elf at 0xD640D0.
bool RBRockShopPersistentData::PurchaseOutfitPiece(const ResourcePath& piece) {
    const bool awarded = false;
    return _AwardOutfitPiece(piece, awarded);
}

// Reconstructed from eboot.elf at 0xD64110.
bool RBRockShopPersistentData::TestUnlockOutfitPiece(const Symbol& piece) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    MakeDirty();
    mUnviewedPieces.erase(piece);
    if (HasPiece(mOwnedPieces, piece) || HasPiece(mUnlockedPieces, piece)) {
        // The report that the piece is already unlocked is compiled out.
        _GetProfileName();
    } else {
        // The report of the unlock is compiled out.
        _GetProfileName();
        mUnlockedPieces.insert(piece);
    }
    MemPopHeap();
    return true;
}

// Reconstructed from eboot.elf at 0xD64400.
bool RBRockShopPersistentData::TestAwardOutfitPiece(const Symbol& piece) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    MakeDirty();
    mUnviewedPieces.erase(piece);
    if (HasPiece(mOwnedPieces, piece)) {
        // The report that the piece is already owned is compiled out.
        _GetProfileName();
    } else {
        // The report of the award is compiled out.
        _GetProfileName();
        mUnlockedPieces.erase(piece);
        mOwnedPieces.insert(piece);
    }
    MemPopHeap();
    return true;
}

// Reconstructed from eboot.elf at 0xD64720.
bool RBRockShopPersistentData::MarkPieceAsViewed(const ResourcePath& piece) {
    const Symbol shortName = theOutfitPieceLoader->GetShortNameForPiece(piece);
    if (IsNullSymbol(shortName)) {
        return false;
    }
    const Symbol genderSwap = theOutfitPieceLoader->GetGenderSwapForOutfit(shortName);
    if (HasPiece(mUnviewedPieces, shortName) || HasPiece(mUnviewedPieces, genderSwap)) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        MakeDirty();
        mUnviewedPieces.erase(shortName);
        if (!IsNullSymbol(genderSwap)) {
            mUnviewedPieces.erase(genderSwap);
        }
        MemPopHeap();
    }
    return true;
}

// Reconstructed from eboot.elf at 0xD64A50.
unsigned long RBRockShopPersistentData::CalculateAndReturnUniquePiecesOwned() const {
    unsigned long count = mOwnedPieces.size();
    eastl::set<Symbol> countedSwaps;
    for (const eastl::rbtree_node_base* node = mOwnedPieces.mAnchor.mpNodeLeft; node != mOwnedPieces.end();
         node = eastl::RBTreeIncrement(node)) {
        const Symbol& piece = PieceAt(node);
        const Symbol genderSwap = theOutfitPieceLoader->GetGenderSwapForOutfit(piece);
        if (HasPiece(countedSwaps, piece) || IsNullSymbol(genderSwap) || HasPiece(countedSwaps, genderSwap)) {
            continue;
        }
        countedSwaps.insert(genderSwap);
        --count;
    }
    return count;
}

// Reconstructed from eboot.elf at 0xD64D10.
bool RBRockShopPersistentData::AnyUnviewedPieces() const {
    return mUnviewedPieces.size() != 0;
}

// Reconstructed from eboot.elf at 0xD64D20.
void RBRockShopPersistentData::GetUnviewedPieces(eastl::vector<Symbol>& pieces) const {
    pieces.clear();
    pieces.reserve(mUnviewedPieces.size());
    for (const eastl::rbtree_node_base* node = mUnviewedPieces.mAnchor.mpNodeLeft; node != mUnviewedPieces.end();
         node = eastl::RBTreeIncrement(node)) {
        pieces.push_back(PieceAt(node));
    }
}
