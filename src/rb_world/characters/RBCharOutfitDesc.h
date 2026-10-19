#pragma once

#include "entity/props/PropMetadata.h"
#include "utl/containers/Vector.h"

// The character outfit descriptions (rb_world/RBCharOutfitDesc.o). Only the
// table the meta state reads is declared; the object is not reconstructed.
namespace RBChar {

// The character genders as allowed values, built by the object's static
// initializer from the gender name table (0xFE11BF).
extern eastl::vector<AllowedValue<int>> gCharGenderAllowedValues;  // 0x1B4BD70

}  // namespace RBChar
