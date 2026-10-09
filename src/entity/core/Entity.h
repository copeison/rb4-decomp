#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "utl/text/Symbol.h"

// A set of game objects grouped into layers (entity/Entity.o). Only the
// layer table and the object API the renderer uses are modelled.
class Entity {
public:
    // One layer's objects. The map names the type; the field names are not
    // in the reference map.
    struct Layer {
        unsigned char mUnknown0[8];
        GameObject** mObjects;
        int mNumObjects;
        unsigned char mUnknown20[28];
    };

    // Creates an object in the layer with room for the given number of
    // components. The map's signature is CreateObject(unsigned long); this
    // build adds the component reserve.
    GameObject* CreateObject(unsigned long layer, unsigned long numComs);  // 0xF0A10
    // The object with the name, or null. The failure report is made only
    // when fail is set.
    GameObject* TryGetObject(Symbol name, bool fail) const;  // 0xF0FF0
    // The first object of any layer. The map's signature is
    // BeginObject() const; this build forwards to NextObject.
    GameObject* BeginObject() const;  // 0xEF180
    // The object after the given one that holds a component with the class
    // or base-class symbol; the empty symbol matches any object. The map's
    // signature is NextObject(GameObjectId, Symbol) const; this build takes
    // the object.
    GameObject* NextObject(const GameObject* object, Symbol com) const;  // 0xEF1A0

    // The object with the id, which must exist. Inlined into
    // RndDefaults::_SyncEnabledLights at 0x6BFAA4; the map only has
    // GetObject(Symbol) const.
    GameObject* GetObject(GameObjectId id) const {
        return mLayers[id.Layer()].mObjects[id.Index()];
    }

    // The entity's root object, the first object of its first layer.
    // Inlined into RndDefaults::_LoadLighting at 0x6BEFDE. Name not in the
    // reference map.
    GameObject* GetRoot() const {
        return mLayers[0].mObjects[0];
    }

    // Field names are not in the reference map.
    unsigned char mUnknown0[176];
    Layer* mLayers;
    unsigned int mNumLayers;
};

static_assert(sizeof(Entity::Layer) == 48);
static_assert(offsetof(Entity::Layer, mObjects) == 8);
static_assert(offsetof(Entity::Layer, mNumObjects) == 16);
static_assert(offsetof(Entity, mLayers) == 176);
static_assert(offsetof(Entity, mNumLayers) == 184);
