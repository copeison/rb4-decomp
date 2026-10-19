#pragma once

#include <cstddef>

#include "anim/drivers/AnimDriver.h"

// The driver of character animations and clips (anim/CharAnimDriver.o):
// it blends the playbacks of CharAnimResources and CharClipResources into
// the skeleton's pose. The vtable is at 0x18F51A0. Only the layout
// AnimDriverCom holds is declared; the class is not reconstructed.
class CharAnimDriver : public AnimDriver {
public:
    CharAnimDriver();  // 0x2DD2F0
    // Slots 0-1: 0x2DD380 and 0x2DD450.
    ~CharAnimDriver() override;
    AnimInstID StartAnim(Resource* resource, const AnimOptions& options) override;  // slot 2: 0x2DE2E0
    void _Poll(Entity* entity) override;  // slot 4: 0x2DDA60

    // The members after the base: two flags, the animation buffer built by
    // 0x331A50 and two vectors; not decoded.
    unsigned char mMembers[336];
};

static_assert(sizeof(CharAnimDriver) == 400);
