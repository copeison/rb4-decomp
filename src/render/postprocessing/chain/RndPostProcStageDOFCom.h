#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "render/postprocessing/chain/RndPostProcStageCom.h"
#include "render/textures/RndTexture2DResource.h"
#include "utl/text/Symbol.h"

class RndComputeBuffer;

// The depth-of-field stage (render/RndPostProcStageDOFCom.o,
// 0x631EF0-0x6331AF). Its class id is "PProcDepthOfField"; its metadata
// notes that it is meant for the next-generation renderer. It blurs the
// scene by depth between the near and far blur distances with the disc
// blur compute shader, which also appends bokeh sprites for the
// overbright pixels. The vtable at 0x192E768 has 43 slots; slot 42 keeps
// the base's empty body although the draw swaps the buffers. The object is
// 80 bytes.
class RndPostProcStageDOFCom : public RndPostProcStageCom {
public:
    // The GPU buffers and the bokeh texture (24 bytes). The constructor is
    // also inlined into the component's.
    struct RuntimeData {
        RuntimeData();   // 0x632030
        ~RuntimeData();  // 0x632050

        // Field names are not in the reference map.
        // The bokeh sprites the disc blur appends: 1024 records of 28
        // bytes.
        RndComputeBuffer* mSprites;
        // Four 32-bit words; by their size the indirect arguments of the
        // sprite draw. The name is weak.
        RndComputeBuffer* mSpriteDrawArgs;
        // The "bokeh_tex" texture. Nothing in this build loads it.
        ResourcePtr<RndTexture2DResource> mBokehTexture;
    };

    RndPostProcStageDOFCom();  // 0x631EF0
    // Copies the settings for an imprint; the runtime data starts empty.
    // Inlined into _Imprint at 0x632F84.
    RndPostProcStageDOFCom(const RndPostProcStageDOFCom& other)
        : RndPostProcStageCom(other),
          mNearBlurDistance(other.mNearBlurDistance),
          mFarBlurDistance(other.mFarBlurDistance),
          mBlurRadius(other.mBlurRadius),
          mBokehOverbrightScale(other.mBokehOverbrightScale),
          mBlurFalloffFunction(other.mBlurFalloffFunction),
          mBokehTex(other.mBokehTex) {}
    ~RndPostProcStageDOFCom() override;  // slots 0-1: 0x631F50, 0x631FC0

    Symbol GetId() const override;         // slot 4: 0x632EC0
    Symbol GetClassName() const override;  // slot 5: 0x632ED0
    int CurrentRev() const override;       // slot 7: 0x632EE0
    bool IsA(Symbol type) const override;  // slot 8: 0x632F00
    Component* AsComponent() override;     // slot 9: 0x632F30
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // settings.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x632F40
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x633080
    ComMetaData& _GetMetaData() override;       // slot 23: 0x633090
    // Slot 29: creates the sprite buffers. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0x632060
    // Slot 41: the disc blur.
    void _DrawImpl(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch) override;  // 0x632BB0

    // Registers the class. Emitted with the renderer's components at
    // 0x3F9AD0. Not reconstructed: the registration helpers it inlines are
    // not modelled.
    static void Init();
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x405A40
    // Registers the class description and the properties below, with the
    // falloff function's choices. Not reconstructed: ComMetaData's string
    // and list members and the properties' metadata are written inline
    // through helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6320F0

    // The object's statics, in the order of its static initializer
    // (0x6330E0). The three ints it first sets (0x1AAAF08: -1, the invalid
    // GameObjectId; 0x1AAAF0C: 8; 0x1AAAF10: 4) come from a shared header
    // and are not modelled.
    static Symbol sId;  // 0x1AAAF18, "PProcDepthOfField"
    // The class's second symbol, also "PProcDepthOfField". Name not in the
    // reference map.
    static Symbol sClassName;           // 0x1AAAF20
    static PropRegistry sPropRegistry;  // 0x1AAAF30
    static ComMetaData sMetaData;       // 0x1AAAFD0

    // Field names are not in the reference map; the properties are named
    // after them.
    // "near_blur_distance": where objects begin to blur.
    float mNearBlurDistance;
    // "far_blur_distance": where objects are fully blurred.
    float mFarBlurDistance;
    // "blur_radius": the radius at full blur.
    float mBlurRadius;
    // "bokeh_overbright_scale": how much the overbright amount scales the
    // bokeh sprites.
    float mBokehOverbrightScale;
    // "blur_falloff_function": how the blur grows between the distances.
    int mBlurFalloffFunction;
    // "bokeh_tex": the bokeh highlight texture.
    ResourcePath mBokehTex;
    RuntimeData mRuntimeData;
};

static_assert(sizeof(RndPostProcStageDOFCom::RuntimeData) == 24);
static_assert(offsetof(RndPostProcStageDOFCom, mNearBlurDistance) == 0x18);
static_assert(offsetof(RndPostProcStageDOFCom, mBlurRadius) == 0x20);
static_assert(offsetof(RndPostProcStageDOFCom, mBlurFalloffFunction) == 0x28);
static_assert(offsetof(RndPostProcStageDOFCom, mBokehTex) == 0x30);
static_assert(offsetof(RndPostProcStageDOFCom, mRuntimeData) == 0x38);
static_assert(sizeof(RndPostProcStageDOFCom) == 0x50);
