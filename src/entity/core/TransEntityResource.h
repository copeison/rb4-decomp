#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"

// Entity resource that is created in code rather than loaded
// (entity/TransEntityResource.o). The class has not been reconstructed; only
// its constructor and the overrides that make it concrete are declared. The
// vtable is at 0x18E9B68; the object is 280 bytes.
class TransEntityResource : public EntityResource {
public:
    TransEntityResource();  // 0x1BAAA0

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x1BB7E0
    Symbol GetId() const override;                   // slot 1: 0x1BB7F0
    bool IsA(Symbol type) const override;            // slot 2: 0x1BB890
    ~TransEntityResource() override;                 // slots 10-11: 0x1BB8C0, 0x1BB950

    // Field names are not in the reference map.
    // EntityResource members after mEntity that its constructor (0xFD9A0)
    // sets and EntityResource does not model.
    unsigned char mEntityResourceState[144];
    // Two eastl::vector<unsigned char> the constructor sets up (200 and
    // 232). The map's TransEntityResource::SetIconData(void const*,
    // unsigned long) suggests icon data; the evidence is weak.
    unsigned char mIconData[64];
    // Set for a resource whose entity enters with the poll flag cleared
    // (EnterEntity, 0x1BB580) and is not polled while its entity is
    // entered (PollEntity, 0x1BB7B0). The meaning is inferred.
    bool mSuppressPoll;
    // The loaded revision; Load (0x1BAC60) reads revisions 12 to 15 and
    // Save (0x1BB4A0) writes 15.
    int mRev;
    // The class symbol of the entity's instance component; saved from
    // revision 15.
    Symbol mInstanceComId;
};

static_assert(offsetof(TransEntityResource, mIconData) == 200);
static_assert(offsetof(TransEntityResource, mSuppressPoll) == 264);
static_assert(offsetof(TransEntityResource, mRev) == 268);
static_assert(offsetof(TransEntityResource, mInstanceComId) == 272);

static_assert(sizeof(TransEntityResource) == 280);
