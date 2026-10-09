#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "entity/resources/ResourceMetaData.h"
#include "utl/text/Symbol.h"

class BinStream;
class Entity;
class GameObject;

// The renderer's options entity (render/RndOptions.o, 0x468000 to
// 0x46A830): one entity, held by the renderer, whose root object carries
// the "*Options" components. The object also emits the nine options
// classes' Init registrations and factories, which are not reconstructed.
class RndOptions {
public:
    // Whether the options are suppressed.
    static bool IsSuppressed();  // 0x469CA0
    // Suppresses or restores the options, copying the state into every
    // options component of the entity's root object.
    static void SetSuppressed(bool suppressed);  // 0x469CB0
    // The options entity's root object.
    static GameObject* GetOwner();  // 0x469D50
    // The class id of the options entity's resource.
    static Symbol GetEntityTypeId();  // 0x469D70

    // The options entity. Its resource, gEntityRes, is not modelled.
    static Entity* gEntity;   // 0x1A868C0
    static bool gSuppressed;  // 0x1A868C8
};

// The resource of the options entity, with the extension "rndopt". The
// vtable is at 0x1904610.
class RndOptionsResource : public EntityResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as RndOptionsCom::_Init (0x46A8E0). The local static is at 0x1A6EFC8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndOptionsResource");
        }
        return id;
    }

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x469FB0
    Symbol GetId() const override;                   // slot 1: 0x469FC0
    bool IsA(Symbol type) const override;            // slot 2: 0x46A060
    // Slot 5 at 0x469F30: as EntityResource's.
    void Save(BinStream& stream, bool cached) override;
    // Slots 10-11: 0x46A090, 0x46A0A0.
    ~RndOptionsResource() override;
    // Slot 13 at 0x469F40: an entity whose root object is named "options".
    Entity* CreateEntity() override;

    // Registers the type under the "Renderer Options" category.
    static void _Init(ResourceMetaData& metaData);  // 0x469E10

    static ResourceMetaData sMetaData;  // 0x1A86860
};

static_assert(sizeof(RndOptionsResource) == 200);
