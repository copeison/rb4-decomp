#pragma once

#include "entity/resources/Resource.h"
#include "rb_world/characters/RBCharOutfitDesc.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// The outfit piece catalogue (rb_world/RBOutfitPieceLoader.o): the pieces'
// files, short names, icons and orderings for the character creator and the
// Rock Shop. Only the lookups the Rock Shop data uses are declared; the
// object is not reconstructed.
class RBOutfitPieceLoader {
public:
    // The piece's short name, or the empty symbol when the path is not a
    // known piece.
    Symbol GetShortNameForPiece(ResourcePath piece) const;  // 0x100DE50
    // The short name of the other gender's version of the piece, or the
    // empty symbol when it has none.
    Symbol GetGenderSwapForOutfit(const Symbol& piece) const;  // 0x1010EF0
    // The Rock Shop's pieces for the slot and gender; `desc` filters the
    // pieces that depend on another slot's piece. The band users pass no
    // description. The match of this body to the map's name is weak (the
    // character creator's GetCharCreatorFilesForOutfitSlot has the same
    // signature).
    void GetRockShopFilesForOutfitSlot(const RBChar::RBCharOutfitSlot& slot,
                                       const RBChar::RBCharGender& gender,
                                       const RBChar::RBCharDesc* desc,
                                       eastl::vector<ResourcePath>& files) const;  // 0x100DED0
};

extern RBOutfitPieceLoader* theOutfitPieceLoader;  // 0x1B4E300
