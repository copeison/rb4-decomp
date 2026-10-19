#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "entity/resources/ResourceMetaData.h"
#include "utl/text/Symbol.h"

class BinStream;
class Entity;

// A bone mask file, extension "bonemask" (anim/CharBoneMaskResource.o,
// 0x2E4C10-0x2E4ECF): an entity resource whose root object holds the
// CharBoneMaskCom that names the masked bone channels. The vtable at
// 0x18F5920 has 33 slots; the class overrides the identity slots, Save,
// the destructors and CreateEntity. The object is 200 bytes, an
// EntityResource without members of its own.
class CharBoneMaskResource : public EntityResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as CharBoneMaskCom::_Init (0x2E1D90). The local static is at
    // 0x19F7CB8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharBoneMaskResource");
        }
        return id;
    }

    // Inlined into the factory (0x2B5E90).
    CharBoneMaskResource() {}
    // Slots 10-11: 0x2E4E60 (a jump to EntityResource's) and 0x2E4E70.
    ~CharBoneMaskResource() override {}

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x2E4D80
    Symbol GetId() const override;                   // slot 1: 0x2E4D90
    bool IsA(Symbol type) const override;            // slot 2: 0x2E4E30
    // Slot 5: the entity resource's. The map has Load and Save; this build
    // keeps only the Save override, and slot 4 is EntityResource::Load.
    void Save(BinStream& stream, bool cached) override;  // 0x2E4D30
    // Slot 13: a new entity with a CharBoneMaskCom on its root.
    Entity* CreateEntity() override;  // 0x2E4D40

    // Registers the class as an entity resource. The binary emits it in
    // anim/AnimInit.o at 0x2AE160, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B5E90.
    static Resource* _Create();
    // Names the extension ("bonemask") and the category ("Animations").
    static void _Init(ResourceMetaData& metaData);  // 0x2E4C10

    // Constructed by the static initializer at 0x2E4E90, which first sets a
    // shared header's null object id to -1 (0x19F8A18).
    static ResourceMetaData sMetaData;  // 0x19F8A20
};

static_assert(sizeof(CharBoneMaskResource) == 200);
