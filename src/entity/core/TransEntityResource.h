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

    unsigned char mUnknown56[224];  // Field names are not in the reference map.
};

static_assert(sizeof(TransEntityResource) == 280);
