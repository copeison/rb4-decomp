#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/postprocessing/chain/RndPostProcStageCom.h"
#include "utl/text/Symbol.h"

// The shader-graph stage (render/RndPostProcStageShaderGraphCom.o,
// 0x633B40-0x63463F). Its class id is "PProcShaderGraph". It applies the
// effect a shader graph describes: the material component on its own
// object, drawn as a full-screen quad that reads the scene as its scene
// texture. The vtable at 0x192EA38 has 43 slots. The object is the base's
// 24 bytes.
class RndPostProcStageShaderGraphCom : public RndPostProcStageCom {
public:
    RndPostProcStageShaderGraphCom();  // 0x633B40
    ~RndPostProcStageShaderGraphCom() override;  // slots 0-1: 0x633B70, 0x633B80

    Symbol GetId() const override;         // slot 4: 0x6343A0
    Symbol GetClassName() const override;  // slot 5: 0x6343B0
    int CurrentRev() const override;       // slot 7: 0x6343C0
    bool IsA(Symbol type) const override;  // slot 8: 0x6343E0
    Component* AsComponent() override;     // slot 9: 0x634410
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // base's flag.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x634420
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x634550
    ComMetaData& _GetMetaData() override;       // slot 23: 0x634560
    // Slot 41: draws the object's material over the destination buffer with
    // the source buffer as the scene texture.
    void _DrawImpl(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch) override;  // 0x633E80
    // Slot 42: swaps the draw target's buffers as the draw does.
    void _UpdateDrawTarget(RndSceneDrawTarget& target) override;  // 0x634380

    // Registers the class. Emitted with the renderer's components at
    // 0x3FA240. Not reconstructed: the registration helpers it inlines are
    // not modelled.
    static void Init();
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x405AE0
    // Registers the class description; the class has no properties of its
    // own. Not reconstructed: ComMetaData's string and list members are
    // written inline through helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x633BA0

    // The object's statics, in the order of its static initializer
    // (0x634570). The three ints it first sets (0x1AAB3F8: -1, the invalid
    // GameObjectId; 0x1AAB3FC: 8; 0x1AAB400: 4) come from a shared header and are
    // not modelled.
    static Symbol sId;  // 0x1AAB408, "PProcShaderGraph"
    // The class's second symbol, also "PProcShaderGraph". Name not in the
    // reference map.
    static Symbol sClassName;           // 0x1AAB410
    static PropRegistry sPropRegistry;  // 0x1AAB420
    static ComMetaData sMetaData;       // 0x1AAB4C0
};

static_assert(sizeof(RndPostProcStageShaderGraphCom) == 0x18);
