#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's display of 2D components
// (render/Rnd2DOptionsCom.o, 0x6D1910-0x6D1E6B). Its class id is "2DOptions".
// The vtable at 0x1938830 has 41 slots. The object is 24 bytes.
class Rnd2DOptionsCom : public RndOptionsCom {
public:
    Rnd2DOptionsCom();  // 0x6D1930
    ~Rnd2DOptionsCom() override;  // slots 0-1: 0x6D1960, 0x6D1970

    Symbol GetId() const override;         // slot 4: 0x6D1BC0
    Symbol GetClassName() const override;  // slot 5: 0x6D1BD0
    int CurrentRev() const override;       // slot 7: 0x6D1BE0
    bool IsA(Symbol type) const override;  // slot 8: 0x6D1C00
    Component* AsComponent() override;     // slot 9: 0x6D1C30
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6D1C40
    // Slot 20: stops being theRnd2DOpts.
    void _PreDestroy(DestroyType type) override;  // 0x6D1910
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6D1D80
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6D1D90
    // Slot 29: becomes theRnd2DOpts.
    bool _OnResourcesLoaded() override;  // 0x6D1920

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6D1990

    static Symbol sId;                  // 0x1AB1358, "2DOptions"
    // Also "2DOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1AB1360
    static PropRegistry sPropRegistry;  // 0x1AB1370
    static ComMetaData sMetaData;       // 0x1AB1410

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mShowSelectedScale9Vertices;  // "show_selected_scale9_vertices"
};

static_assert(offsetof(Rnd2DOptionsCom, mShowSelectedScale9Vertices) == 0x17);
static_assert(sizeof(Rnd2DOptionsCom) == 0x18);

// The live component, or null while none has loaded its resources.
extern Rnd2DOptionsCom* theRnd2DOpts;  // 0x1AB15B8
