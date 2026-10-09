#include "math/vector/Vector3.h"

// Initialized data at 0x19B0340, 0x19B034C and 0x19B0358.
Vector3 Vector3::sX = {1.0F, 0.0F, 0.0F};
Vector3 Vector3::sY = {0.0F, 1.0F, 0.0F};
Vector3 Vector3::sZ = {0.0F, 0.0F, 1.0F};

// Zero-initialized storage at 0x19E76F0.
Vector3 Vector3::sZero = {0.0F, 0.0F, 0.0F};
