#pragma once

#include <cstddef>

#include "entity/props/PropMetadata.h"
#include "math/color/Color.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;

// The character outfit descriptions (rb_world/RBCharOutfitDesc.o). Only the
// tables and the description types other modules read are declared; the
// object is not reconstructed.
namespace RBChar {

// The outfit slots, in the order of the slot name table (0x19BE790, the
// map's gOutfitSlotNameArray). Enumerator names not in the reference map;
// they follow the slot names.
enum RBCharOutfitSlot {
    kOutfitSlotGuitar = 0,
    kOutfitSlotBass = 1,
    kOutfitSlotDrums = 2,
    kOutfitSlotMic = 3,
    kOutfitSlotSkin = 4,
    kOutfitSlotTop = 5,
    kOutfitSlotBottom = 6,
    kOutfitSlotHead = 7,
    kOutfitSlotHair = 8,
    kOutfitSlotEyebrows = 9,
    kOutfitSlotFacialHair = 10,
    kOutfitSlotEarring = 11,
    kOutfitSlotFacialPiercing = 12,
    kOutfitSlotGlasses = 13,
    kOutfitSlotMakeupLips = 14,
    kOutfitSlotMakeupEyes = 15,
    kOutfitSlotMakeupFace = 16,
    kOutfitSlotMakeupFullFace = 17,
    kOutfitSlotRings = 18,
    kOutfitSlotGuitarPick = 19,
    kOutfitSlotTambourine = 20,
    kOutfitSlotCowbell = 21,
    kOutfitSlotDrumsticks = 22,
    kOutfitSlotEye = 23,
    kOutfitSlotMicStand = 24,
    kOutfitSlotCostume = 25,
    kNumOutfitSlots = 26,
};

// A character's gender. The map names the type (the outfit loader's
// queries take it by reference); its values are not recovered.
enum RBCharGender {
};

// A configuration value of a piece (the map's
// GetOutfitPieceSettingForConfig reads one by name). Field names not in the
// reference map.
struct RBOutfitPieceConfigurationSetting {
    Symbol mConfig;
    unsigned int mValue = 0;
};

static_assert(sizeof(RBOutfitPieceConfigurationSetting) == 16);

// A color of a piece; the color defaults to opaque black. Field names not
// in the reference map, and the two flags' meanings are weak.
struct RBOutfitPieceColorSetting {
    bool mEnabled = false;
    Hmx::Color mColor = Hmx::Color(0.0F, 0.0F, 0.0F, 1.0F);
    bool mOverride = false;
};

static_assert(offsetof(RBOutfitPieceColorSetting, mColor) == 4);
static_assert(offsetof(RBOutfitPieceColorSetting, mOverride) == 20);
static_assert(sizeof(RBOutfitPieceColorSetting) == 24);

// The piece worn in one slot. Field names not in the reference map.
struct RBCharOutfitPieceDesc {
    Symbol mName;
    // Set on a new piece; RBCharacterPersistentData::_CheckForDefaultPieces
    // reports named default pieces outside the instrument slots. The name
    // is weak.
    bool mIsDefault = true;
    RBOutfitPieceConfigurationSetting mConfigs[4];
    RBOutfitPieceColorSetting mColors[3];
};

static_assert(offsetof(RBCharOutfitPieceDesc, mIsDefault) == 8);
static_assert(offsetof(RBCharOutfitPieceDesc, mConfigs) == 16);
static_assert(offsetof(RBCharOutfitPieceDesc, mColors) == 80);
static_assert(sizeof(RBCharOutfitPieceDesc) == 152);

// A full outfit, one piece per slot. Field names not in the reference map.
struct RBCharOutfitDesc {
    // The first value OutfitDescIntoDataArray (0xFDEE20) writes. The name
    // is weak.
    int mVersion = 0;
    RBCharOutfitPieceDesc mPieces[kNumOutfitSlots];
    // Not written by OutfitDescIntoDataArray; the name is weak.
    bool mModified = false;
};

static_assert(offsetof(RBCharOutfitDesc, mPieces) == 8);
static_assert(offsetof(RBCharOutfitDesc, mModified) == 3960);
static_assert(sizeof(RBCharOutfitDesc) == 3968);

// A character: its name, gender and outfit. Plain data, copied with
// memcpy (3984 bytes); the users of the profile's characters inline the
// default constructor. Field names not in the reference map.
struct RBCharDesc {
    // Writes the description as a DataArray (version 9, the name, the
    // gender, the outfit) behind a presence byte.
    void Save(BinStream& stream) const;  // 0xFE0E40
    void Load(BinStream& stream);        // 0xFE0F20

    Symbol mName;
    // An RBCharGender, stored in a byte. Its values are not recovered.
    unsigned char mGender = 0;
    RBCharOutfitDesc mOutfit;
};

static_assert(offsetof(RBCharDesc, mGender) == 8);
static_assert(offsetof(RBCharDesc, mOutfit) == 16);
static_assert(sizeof(RBCharDesc) == 3984);

// Call Save and Load.
BinStream& operator<<(BinStream& stream, const RBCharDesc& desc);  // 0xFE0FE0
BinStream& operator>>(BinStream& stream, RBCharDesc& desc);        // 0xFE1000

// The slot's display name ("Guitar", ...) or "INVALID_OUTFIT_SLOT".
const char* GetOutfitSlotName(const RBCharOutfitSlot& slot);  // 0xFDD5B0
// True for the instrument and prop slots: guitar, bass, drums, mic, guitar
// pick, tambourine, cowbell, drumsticks and mic stand. The map has the
// name; matching it to this body rests on the slots.
bool IsInstrumentOutfitSlot(const RBCharOutfitSlot& slot);  // 0xFDD5D0

// The character genders as allowed values, built by the object's static
// initializer from the gender name table (0xFE11BF).
extern eastl::vector<AllowedValue<int>> gCharGenderAllowedValues;  // 0x1B4BD70
// Every outfit slot in order, built by the same initializer. The map's
// gOutfitSlotVec follows gCharGenderAllowedValues in the same way.
extern eastl::vector<RBCharOutfitSlot> gOutfitSlotVec;  // 0x1B4BD90

}  // namespace RBChar
