#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

// A scene file: an entity resource whose root object carries the scene
// components. The binary derives it from an intermediate entity resource
// class (vtable 0x1937A08, constructor 0x6C0CA0) that is not modelled; its
// members are covered by the padding. The vtable is at 0x19013C0.
class RndSceneResource : public EntityResource {
public:
    RndSceneResource();  // 0x4384A0

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x43AB50
    Symbol GetId() const override;                   // slot 1: 0x43AB60
    bool IsA(Symbol type) const override;            // slot 2: 0x43AC00
    ~RndSceneResource() override;                    // slots 10-11: 0x4384D0, 0x4384E0
    // Creates the entity and its root's scene components.
    Entity* CreateEntity() override;                 // slot 13: 0x439350

    // The class id, created on first use. Inlined into its users, for
    // example Resource::GetOrLoad<RndSceneResource> at 0x6C0160; the local
    // static is RndSceneResource::Id()::id in the map.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndSceneResource");
        }
        return id;
    }

    // Field names are not in the reference map.
    unsigned char mUnknown56[728];
};

static_assert(sizeof(RndSceneResource) == 784);
