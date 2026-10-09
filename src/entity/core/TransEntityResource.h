#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/Vector.h"

// Entity resource that is created in code rather than loaded
// (entity/TransEntityResource.o, 0x1BAAA0-0x1BB9F2). Its root object
// carries a TransCom, and its entity can enter without the poll context's
// immediate flag. The vtable is at 0x18E9B68; the object is 280 bytes.
class TransEntityResource : public EntityResource {
public:
    // An element of mPropRegistries: a registry and 16 bytes after it that
    // nothing in this build reads or writes. Name not in the reference map.
    struct RegistryEntry {
        PropRegistry mRegistry;
        unsigned char mPadding[16];  // Never read or written.
    };

    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19C6328.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("TransEntityResource");
        }
        return id;
    }

    TransEntityResource();  // 0x1BAAA0

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x1BB7E0
    Symbol GetId() const override;                   // slot 1: 0x1BB7F0
    bool IsA(Symbol type) const override;            // slot 2: 0x1BB890
    // Slot 5: writes revision 15 and the instance component's class, then
    // the entity.
    void Save(BinStream& stream, bool cached) override;  // 0x1BB4A0
    ~TransEntityResource() override;                 // slots 10-11: 0x1BB8C0, 0x1BB950
    // Slot 14. Reads the revision, the root data of revisions 12-13 and the
    // instance component's class from revision 15, loads the entity, and
    // resets the root's TransCom unless the instance component drives the
    // parent. The map's TransEntityResource::Load(BinStream&, bool).
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x1BAF10
    // Slot 15. For revisions before 15, takes the instance component's
    // class from the root's first component whose class serves as one.
    bool _PostLoad(BinStream& stream, bool cached) override;  // 0x1BB0E0
    // Slot 16. Enters with the thread's immediate flag cleared unless
    // `flags` has bit 0 set or polling is suppressed. The map's
    // EnterEntity(EntityPtr).
    void _EnterEntity(Entity* entity, unsigned int flags) override;  // 0x1BB580
    // Slot 18. A suppressed, entered entity is polled only when its root's
    // instance component asks for it, once, with the immediate flag
    // cleared. The map's PollEntity(EntityPtr).
    void _PollEntity(Entity* entity) override;  // 0x1BB660
    // Slots 20-21: skipped while polling is suppressed.
    void _EnterImmediately(Entity* entity) override;  // 0x1BB7B0
    void _PollImmediately(Entity* entity) override;   // 0x1BB7D0
    // Slot 26 at 0x1BB9F0: true.
    bool IsTransient() const override {
        return true;
    }
    // Slot 27: adds a TransCom to every new object, parented to the root
    // when the TransCom parent option is set. The map's InitObject(ObjPtr&).
    void InitObject(GameObject* object) override;  // 0x1BB520
    // Slot 32: reads the root, taking the root data from the instance
    // component's icon data for revisions before 12. The map's
    // _LoadRoot(BinStream&, EntityPtr, vector<unsigned char>&,
    // vector<ResourcePath>&).
    void _LoadRoot(
        BinStream& stream,
        Entity* entity,
        eastl::vector<ResourcePath>* paths,
        eastl::vector<unsigned char>* rootData) override;  // 0x1BAC60

    // Builds the class's metadata: extension "transentity", category
    // "Trans Entities".
    static void _Init(ResourceMetaData& metaData);  // 0x1BAB40
    // The number of properties of the root's component with the class or
    // base-class symbol, or zero. Name not in the reference map. Not
    // reconstructed.
    unsigned long GetNumComProps(Symbol com) const;  // 0x1BB170

    static ResourceMetaData sMetaData;  // 0x19E4980

    // Field names are not in the reference map.
    // The map's TransEntityResource::SetIconData(void const*, unsigned
    // long) suggests icon data; no reader is identified, so the evidence is
    // weak.
    eastl::vector<unsigned char> mIconData;
    // Property registries the resource builds; the destructor destroys them
    // with PropRegistry's destructor (0x180540). The map's
    // SetupPropRegistry() suggests their use; nothing in this build adds
    // one.
    eastl::vector<RegistryEntry> mPropRegistries;
    // Set for a resource whose entity enters with the poll flag cleared
    // (_EnterEntity) and is not polled at once while its entity is entered
    // (_EnterImmediately). The meaning is inferred.
    bool mSuppressPoll;
    // The loaded revision; _LoadEntity reads revisions 12 to 15 and Save
    // writes 15.
    int mRev;
    // The class symbol of the entity's instance component; saved from
    // revision 15.
    Symbol mInstanceComId;
};

static_assert(sizeof(TransEntityResource::RegistryEntry) == 176);
static_assert(offsetof(TransEntityResource, mIconData) == 200);
static_assert(offsetof(TransEntityResource, mPropRegistries) == 232);
static_assert(offsetof(TransEntityResource, mSuppressPoll) == 264);
static_assert(offsetof(TransEntityResource, mRev) == 268);
static_assert(offsetof(TransEntityResource, mInstanceComId) == 272);
static_assert(sizeof(TransEntityResource) == 280);
