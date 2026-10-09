#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/postprocessing/chain/RndPostProcStageCom.h"
#include "utl/text/Symbol.h"

// The bloom stage (render/RndPostProcStageBloomCom.o, 0x62FA00-0x63192F).
// Its class id is "PProcBloom". It thresholds the scene into the
// half-size (or only the quarter-size) downsample buffers, blurs them, and
// composites them over the scene into the destination buffer
// (RndShaderBloom). The vtable at 0x192E3F8 has 43 slots. The object is 48
// bytes.
class RndPostProcStageBloomCom : public RndPostProcStageCom {
public:
    RndPostProcStageBloomCom();            // 0x62FA00
    ~RndPostProcStageBloomCom() override;  // slots 0-1: 0x62FA60, 0x62FA70

    Symbol GetId() const override;         // slot 4: 0x631450
    Symbol GetClassName() const override;  // slot 5: 0x631460
    int CurrentRev() const override;       // slot 7: 0x631470
    bool IsA(Symbol type) const override;  // slot 8: 0x631490
    Component* AsComponent() override;     // slot 9: 0x6314C0
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // settings.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6314D0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x631610
    ComMetaData& _GetMetaData() override;       // slot 23: 0x631620
    // Slot 41: the bloom pass. IDA names it render_bloom_pass.
    void _DrawImpl(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch) override;  // 0x6305A0
    // Slot 42: swaps the draw target's buffers as the pass does.
    void _UpdateDrawTarget(RndSceneDrawTarget& target) override;  // 0x631430

    // Registers the class. Emitted with the renderer's components at
    // 0x3F9720. Not reconstructed: the registration helpers it inlines are
    // not modelled.
    static void Init();
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x4059F0
    // Registers the class description and the properties below, with the
    // property callbacks at 0x631630-0x631850. Not reconstructed:
    // ComMetaData's string and list members and the properties' metadata
    // are written inline through helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x62FA90

    // The object's statics, in the order of its static initializer
    // (0x631860). The three ints it first sets (0x1AAAA08: -1, the invalid
    // GameObjectId; 0x1AAAA0C: 8; 0x1AAAA10: 4) come from a shared header
    // and are not modelled.
    static Symbol sId;  // 0x1AAAA18, "PProcBloom"
    // The class's second symbol, also "PProcBloom". Name not in the
    // reference map.
    static Symbol sClassName;           // 0x1AAAA20
    static PropRegistry sPropRegistry;  // 0x1AAAA30
    static ComMetaData sMetaData;       // 0x1AAAAD0

    // Field names are not in the reference map; the properties are named
    // after them.
    // "intensity": the bloom's strength.
    float mIntensity;
    // "power": the exponent on the overbright values; below one softens.
    float mPower;
    // "shape": the mix between the half-size and quarter-size bloom.
    float mShape;
    // "use_half_size_buffer": also blooms at half size.
    bool mUseHalfSizeBuffer;
    // "value_based": thresholds on luminance rather than per channel.
    bool mValueBased;
    // "overbright_hue_preservation": keeps the hue when compositing.
    bool mOverbrightHuePreservation;
    // "overbright_whitening_strength" and "max_overbright_whitening".
    float mOverbrightWhiteningStrength;
    float mMaxOverbrightWhitening;
};

static_assert(offsetof(RndPostProcStageBloomCom, mIntensity) == 0x18);
static_assert(offsetof(RndPostProcStageBloomCom, mShape) == 0x20);
static_assert(offsetof(RndPostProcStageBloomCom, mUseHalfSizeBuffer) == 0x24);
static_assert(offsetof(RndPostProcStageBloomCom, mOverbrightHuePreservation) == 0x26);
static_assert(offsetof(RndPostProcStageBloomCom, mOverbrightWhiteningStrength) == 0x28);
static_assert(offsetof(RndPostProcStageBloomCom, mMaxOverbrightWhitening) == 0x2C);
static_assert(sizeof(RndPostProcStageBloomCom) == 0x30);
