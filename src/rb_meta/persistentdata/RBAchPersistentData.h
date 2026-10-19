#pragma once

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// The achievement progress a profile saves (rb_meta/RBAchPersistentData.o):
// the platform half derives from it, and the profile saves it through this
// type (Revision::AsOfOp<RBAchPersistentData> in the map). In this build it
// also carries the achievement interface at offset 24 and the profile at
// 32. Declared only: the constructor (0xF07280) lies with the platform
// half, and the class is not reconstructed.
class RBAchPersistentData : public PersistentData {
public:
    explicit RBAchPersistentData(RBProfile* profile);  // 0xF07280
    ~RBAchPersistentData() override;
};
