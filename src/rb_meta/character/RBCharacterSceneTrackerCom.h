#pragma once

#include "entity/core/Component.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class BandUser;
class Entity;

// The musician kinds of rb_world/RBMusicianTypes.o. Only the instrument
// type is declared; its four enumerators are not recovered.
namespace MusicianType {
enum Instrument : int {};
}  // namespace MusicianType

// The component of a character scene's root that knows the scene's player
// characters by instrument (rb_meta/RBCharacterSceneTrackerCom.o). Only the
// members RBCharacterLayoutTrackerCom forwards to are declared; the class
// is not reconstructed.
class RBCharacterSceneTrackerCom : public Component {
public:
    // The player entity of the instrument (one of four), or null.
    Entity* GetPlayerEntityByInstrument(const MusicianType::Instrument& instrument) const;  // 0xB0BF70
    // The player entity of the user's instrument, or null.
    Entity* GetPlayerEntityForUser(const BandUser* user) const;  // 0xB0BF90
    // Whether a player entity of the scene is still loading.
    bool AnyPlayerEntityIsLoading() const;  // 0xB0C020
    ResourcePath GetPlayerEntityPathByInstrument(const MusicianType::Instrument& instrument) const;  // 0xB0C330
    ResourcePath GetPlayerEntityPathForUser(const BandUser* user) const;  // 0xB0C3E0

    // "RBCharacterSceneTrackerCom".
    static Symbol sId;  // 0x1AF9DB0
};
