#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's drawing of cameras
// (render/RndCameraOptionsCom.o, 0x6B9270-0x6B993B). Its class id is
// "CameraOptions". The vtable at 0x1937578 has 41 slots. The object is 32
// bytes.
class RndCameraOptionsCom : public RndOptionsCom {
public:
    RndCameraOptionsCom();  // 0x6B9290
    ~RndCameraOptionsCom() override;  // slots 0-1: 0x6B92D0, 0x6B92E0

    Symbol GetId() const override;         // slot 4: 0x6B9680
    Symbol GetClassName() const override;  // slot 5: 0x6B9690
    int CurrentRev() const override;       // slot 7: 0x6B96A0
    bool IsA(Symbol type) const override;  // slot 8: 0x6B96C0
    Component* AsComponent() override;     // slot 9: 0x6B96F0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6B9700
    // Slot 20: stops being theRndCameraOpts.
    void _PreDestroy(DestroyType type) override;  // 0x6B9270
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6B9850
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6B9860
    // Slot 29: becomes theRndCameraOpts.
    bool _OnResourcesLoaded() override;  // 0x6B9280

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6B9300

    static Symbol sId;                  // 0x1AAFDC8, "CameraOptions"
    // Also "CameraOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1AAFDD0
    static PropRegistry sPropRegistry;  // 0x1AAFDE0
    static ComMetaData sMetaData;       // 0x1AAFE80

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mShowCameras;  // "show_cameras"
    bool mShowSelectedCameraFrusta;  // "show_selected_camera_frusta"
    float mCameraFrustaOpacity;  // "camera_frusta_opacity"
};

static_assert(offsetof(RndCameraOptionsCom, mShowCameras) == 0x17);
static_assert(offsetof(RndCameraOptionsCom, mShowSelectedCameraFrusta) == 0x18);
static_assert(offsetof(RndCameraOptionsCom, mCameraFrustaOpacity) == 0x1C);
static_assert(sizeof(RndCameraOptionsCom) == 0x20);

// The live component, or null while none has loaded its resources.
extern RndCameraOptionsCom* theRndCameraOpts;  // 0x1AB0028
