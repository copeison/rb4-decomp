#pragma once

#include "math/transform/Transform.h"

// How an object turns to face the camera: the "facing_type" of
// RndLookAtCameraCom and the "billboarding" of RndDrawInstanceCom. The
// labels and help strings are at 0x1901DB0 and 0x1901DE0. Names not in the
// reference map; the enumerator names follow the labels.
enum RndBillboardType : int {
    // "No billboarding is applied".
    kBillboardNone = 0,
    // "Directly face the camera", sharing its up axis.
    kBillboardCamera = 1,
    // "Face the camera in XY, aligning Z straight up".
    kBillboardCameraXY = 2,
    // "Rotate about the object's Z-axis to face the camera".
    kBillboardCameraKeepZ = 3,
    // "Face the camera, but do not match its roll": world Z is up.
    kBillboardCameraNoRoll = 4,
};

// The label and the help text of a billboard type ("" past the last).
// Emitted among the RndUtl helpers. Names not in the reference map.
const char* GetBillboardTypeName(int type);  // 0x441E10
const char* GetBillboardTypeHelp(int type);  // 0x441E30

// Which local axis points at the camera: RndLookAtCameraCom's
// "facing_axis", whose allowed values are these two (0x406710). Only the
// negative Y axis changes the result. Names not in the reference map.
enum RndBillboardAxis : int {
    // "Point my local +Y axis towards the camera".
    kBillboardAxisPosY = 1,
    // "Point my local -Y axis towards the camera".
    kBillboardAxisNegY = 4,
};

// The world transform that turns `xfm` towards the camera at `camera`,
// keeping its position. The forward (+Y) row points from the object to the
// camera, or against the camera's forward axis when the two positions
// meet; kBillboardAxisNegY reverses it. Emitted among the RndUtl helpers
// (0x441C90-0x4425A0). Name not in the reference map.
Transform ComputeBillboardXfm(
    const Transform& xfm,
    const Transform& camera,
    int type,
    int axis);  // 0x441E50
