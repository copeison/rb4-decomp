#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/editor/RndEditorDrawCom.h"
#include "utl/text/Symbol.h"

// The editor companion of RndDrawNodeCom (render/RndDrawNodeEditCom.o,
// 0x6C9450-0x6C9B9A; the static initializer at 0x6C9AD0). Its class id is
// "DrawNodeEditorData". It frames the object by its draw node's world
// sphere and draws that sphere for a selected object when the selection
// options show bounding spheres. The vtable at 0x1938068 has 59 slots. The
// object is 48 bytes.
class RndDrawNodeEditCom : public RndEditorDrawCom {
public:
    RndDrawNodeEditCom();            // 0x6C9450
    ~RndDrawNodeEditCom() override;  // slots 0-1: 0x6C9480, 0x6C9490

    Symbol GetId() const override;         // slot 4: 0x6C98F0
    Symbol GetClassName() const override;  // slot 5: 0x6C9900
    int CurrentRev() const override;       // slot 7: 0x6C9910
    bool IsA(Symbol type) const override;  // slot 8: 0x6C9930
    Component* AsComponent() override;     // slot 9: 0x6C9960
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6C9970
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6C9AB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6C9AC0
    // Slot 41: 0x42, the draw node's sphere among them.
    unsigned int _GetCapabilityFlagsImpl() const override;  // 0x6C94B0
    // Slot 42: only a selected object, while the selection options show
    // bounding spheres and are not suppressed.
    bool _FilterShowingImpl(
        const RndCameraContext* camera,
        bool selected) const override;  // 0x6C94C0
    // Slot 46: the draw node's world sphere as a translucent yellow mesh.
    void _EditorDrawImpl(
        RndContext& context,
        bool selected,
        const VectorAdapter<unsigned long>& subEditables) override;  // 0x6C94F0

    // Registers the class. Emitted in render/RndInit.o at 0x3FE220. Not
    // reconstructed: the registration helpers it inlines are not
    // modelled.
    static void Init();
    // The class factory, emitted with the other render factories. The
    // placement factory beside it (0x406080) is not modelled.
    static Component* _Create();  // 0x406050
    // Describes the class after RndEditorDrawCom::_Init: allowed in
    // TransEntityResources, requiring a draw node and created with one.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6C95F0

    static Symbol sId;                  // 0x1AB0848, "DrawNodeEditorData"
    // Also "DrawNodeEditorData". Name not in the reference map.
    static Symbol sClassName;           // 0x1AB0850
    static PropRegistry sPropRegistry;  // 0x1AB0860
    static ComMetaData sMetaData;       // 0x1AB0900
};

static_assert(sizeof(RndDrawNodeEditCom) == 48);
