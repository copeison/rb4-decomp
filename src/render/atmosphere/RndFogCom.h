#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "math/vector/Vector3.h"
#include "render/atmosphere/RndAtmosphereCom.h"
#include "utl/text/Symbol.h"

class RndBufferCollection;
class RndContext;
class RndShaderCBuffer;
class RndTextureBase;
struct RndSceneDrawTarget;

// The material-based fog (render/RndFogCom.o, 0x451D10-0x452C9F). Its class
// id is "Fog". Between the atmosphere's start and end distances a pixel
// fades into the sky's atmosphere texture along the falloff function: the
// deferred pass draws it over the light accumulation, and forward-shaded
// materials read the same falloff from the scene constants. The vtable at
// 0x1902D30 has 41 slots. The object is 64 bytes.
class RndFogCom : public RndAtmosphereCom {
public:
    // The fog's derived state, which imprints do not copy. The map has a
    // RuntimeData on its older RndAtmosphereCom; field names are not in the
    // reference map.
    struct RuntimeData {
        // The shader's gFalloffParams: the reciprocal of the fog range, the
        // start distance scaled by it and negated, and the falloff function
        // (plus a tenth, so the shader can truncate it).
        Vector3 mFalloffParams;
        // Holds mFalloffParams for the deferred pass.
        RndShaderCBuffer* mCBuffer;
    };

    RndFogCom();  // 0x451D10
    // Inlined into _Imprint: the runtime constants are not copied.
    RndFogCom(const RndFogCom& other)
        : RndAtmosphereCom(other),
          mFalloffFunction(other.mFalloffFunction),
          mRuntime{{0.0F, 0.0F, 0.0F}, nullptr} {}
    ~RndFogCom() override;  // slots 0-1: 0x451D60, 0x451D90

    Symbol GetId() const override;         // slot 4: 0x4529D0
    Symbol GetClassName() const override;  // slot 5: 0x4529E0
    int CurrentRev() const override;       // slot 7: 0x4529F0
    bool IsA(Symbol type) const override;  // slot 8: 0x452A10
    Component* AsComponent() override;     // slot 9: 0x452A40
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x452A50
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x452BB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x452BC0
    // Slot 29: creates the fog's constant buffer and fills it.
    bool _OnResourcesLoaded() override;  // 0x452750
    // Slots 33 and 38 at 0x4528B0 and 0x452940: _SyncFalloffParams, inlined.
    void _Poll() override;
    void _EditPoll() override;

    // Draws the fog over the target's light accumulation, against the
    // frame's depth, from the atmosphere texture (or the default black
    // texture). Name not in the reference map; the map's older atmosphere
    // has ApplyDeferredAndSetupFwdShaded.
    void ApplyDeferred(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target,
        RndTextureBase* atmosphereTexture);  // 0x452390
    // The texture forward shading samples for the fog in the mode: the
    // atmosphere texture, or a default 2D texture when it is null or the
    // mode uses none (1, 3, 6, 7 and 9 give the black texture, 8 and 10 the
    // white one, any other mode past 10 the error texture). The fog itself
    // is not read. Name not in the reference map; the meaning of the mode
    // is uncertain (the scene drawer passes 5).
    RndTextureBase* TextureOrDefault(
        RndTextureBase* texture,
        unsigned int mode) const;  // 0x452690
    // Writes the falloff constants into the scene buffer. The map has it
    // on its older RndAtmosphereCom.
    void SetFwdShadingConstants(RndShaderCBuffer& cbuffer) const;  // 0x4526E0
    // Writes zero falloff constants. The map has it on its older
    // RndAtmosphereCom.
    static void SetNoAtmosphereFwdShadingConstants(
        RndShaderCBuffer& cbuffer);  // 0x452710
    // Recomputes the falloff constants from the distances and the falloff
    // function and writes them into the fog's constant buffer. No direct
    // caller; the polls inline it. The map has it on its older
    // RndAtmosphereCom.
    void _SyncFalloffParams();  // 0x452820

    // Describes the class and registers "falloff_function" after
    // RndAtmosphereCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x451DD0

    static Symbol sId;  // 0x1A75890, "Fog"
    // Also "Fog". Name not in the reference map.
    static Symbol sClassName;           // 0x1A75898
    static PropRegistry sPropRegistry;  // 0x1A758A0
    static ComMetaData sMetaData;       // 0x1A75940

    // Field names are not in the reference map.
    int mFalloffFunction;  // "falloff_function"
    RuntimeData mRuntime;
};

static_assert(offsetof(RndFogCom, mFalloffFunction) == 0x20);
static_assert(offsetof(RndFogCom, mRuntime) == 0x28);
static_assert(offsetof(RndFogCom::RuntimeData, mCBuffer) == 0x10);
static_assert(sizeof(RndFogCom) == 0x40);
