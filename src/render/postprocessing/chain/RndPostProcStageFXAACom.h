#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/postprocessing/chain/RndPostProcStageCom.h"
#include "utl/text/Symbol.h"

// The FXAA stage (render/RndPostProcStageFXAACom.o, 0x6331B0-0x633B3F).
// Its class id is "PProcFXAA"; its metadata calls it a *last-gen*
// antialiasing effect. The vtable at 0x192E8D0 has 43 slots. The object is
// the base's 24 bytes.
class RndPostProcStageFXAACom : public RndPostProcStageCom {
public:
    RndPostProcStageFXAACom();  // 0x6331B0
    ~RndPostProcStageFXAACom() override;  // slots 0-1: 0x6331E0, 0x6331F0

    Symbol GetId() const override;         // slot 4: 0x6338A0
    Symbol GetClassName() const override;  // slot 5: 0x6338B0
    int CurrentRev() const override;       // slot 7: 0x6338C0
    bool IsA(Symbol type) const override;  // slot 8: 0x6338E0
    Component* AsComponent() override;     // slot 9: 0x633910
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // base's flag.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x633920
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x633A50
    ComMetaData& _GetMetaData() override;       // slot 23: 0x633A60
    // Slot 41: FXAA from the source buffer into the destination buffer.
    void _DrawImpl(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch) override;  // 0x6333C0
    // Slot 42: swaps the draw target's buffers as the draw does.
    void _UpdateDrawTarget(RndSceneDrawTarget& target) override;  // 0x633880

    // Registers the class. Emitted with the renderer's components at
    // 0x3F9E90. Not reconstructed: the registration helpers it inlines are
    // not modelled.
    static void Init();
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x405A90
    // Registers the class description; the class has no properties of its
    // own. Not reconstructed: ComMetaData's string and list members are
    // written inline through helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x633210

    // The object's statics, in the order of its static initializer
    // (0x633A70). The three ints it first sets (0x1AAB178: -1, the invalid
    // GameObjectId; 0x1AAB17C: 8; 0x1AAB180: 4) come from a shared header and are
    // not modelled.
    static Symbol sId;  // 0x1AAB188, "PProcFXAA"
    // The class's second symbol, also "PProcFXAA". Name not in the
    // reference map.
    static Symbol sClassName;           // 0x1AAB190
    static PropRegistry sPropRegistry;  // 0x1AAB1A0
    static ComMetaData sMetaData;       // 0x1AAB240
};

static_assert(sizeof(RndPostProcStageFXAACom) == 0x18);
