#pragma once

class Entity;

// The game's shared definitions (rb_game/Defines.o). Only what the
// reconstructed meta code uses is declared; the object is not
// reconstructed.

// The four player slots of a band. The map names the type (for example
// RBMetaState::IsSlotJoined(GameSlot)); the enumerator names are not in
// the reference map and follow the slot properties of RBMetaStateCom's
// "joined_slots" ("guitar0", "drum", "vocals", "guitar1"). kNumGameSlots
// also stands for "no particular slot".
enum GameSlot {
    kGameSlotGuitar0 = 0,
    kGameSlotDrum = 1,
    kGameSlotVocals = 2,
    kGameSlotGuitar1 = 3,
    kNumGameSlots = 4,
};

static_assert(sizeof(GameSlot) == 4);

// The entity itself or the nearest entity out through the instancing
// parent objects whose resource is a UILayoutResource, or null. Newer than
// the map; the name is inferred, and its placement in this object rests on
// its neighbours (the city, venue and band name tokens). Not reconstructed.
Entity* GetUILayoutEntity(Entity* entity);  // 0x9A39E0
