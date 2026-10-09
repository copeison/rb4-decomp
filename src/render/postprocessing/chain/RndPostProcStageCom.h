#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/drawing/RndSceneInternalContext.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "utl/text/Symbol.h"

class RndContext;
class RndTextureBase;
struct RndSceneBatchContext;

// The base of the post-processing stages a RndPostProcCom applies in order
// (render/RndPostProcStageCom.o, 0x631930-0x631EEF). Its class id is
// "PostProcStage". A stage reads the scene from the draw target's source
// light-accumulation buffer and writes the destination one, then swaps the
// two; slot 42 makes the same swap without drawing, for the threaded scene
// draw that predicts the targets ahead. The vtable at 0x192E600 has 43
// slots. The object is 24 bytes: the flag lives in Component's tail padding.
class RndPostProcStageCom : public Component {
public:
    RndPostProcStageCom();            // 0x631930
    ~RndPostProcStageCom() override;  // slots 0-1: 0x631960, 0x631970

    Symbol GetId() const override;         // slot 4: 0x631C30
    Symbol GetClassName() const override;  // slot 5: 0x631C40
    int CurrentRev() const override;       // slot 7: 0x631C50
    bool IsA(Symbol type) const override;  // slot 8: 0x631C70
    Component* AsComponent() override;     // slot 9: 0x631CA0
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // flag.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x631CB0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x631DE0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x631DF0
    // Slot 29 at 0x631C20: nothing to load; true. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 38 at 0x631C10: empty. The map's _EditPoll(ObjPtr const&).
    void _EditPoll() override;
    // Slot 41 at 0x631E00: draws the stage; empty here. The map's
    // _DrawImpl(ObjPtr const&, RndContext&, RndSceneDrawParams const&);
    // this build drops the ObjPtr and adds the camera and the batch
    // context.
    virtual void _DrawImpl(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch);
    // Slot 42 at 0x631E10: updates the draw target as _DrawImpl would;
    // empty here. Name not in the reference map.
    virtual void _UpdateDrawTarget(RndSceneDrawTarget& target);

    // Draws the stage when it is enabled. The map's Draw(ObjPtr const&,
    // RndContext&, RndSceneDrawParams const&).
    void Draw(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch);  // 0x631BD0
    // Updates the draw target when the stage is enabled. Name not in the
    // reference map.
    void UpdateDrawTarget(RndSceneDrawTarget& target);  // 0x631BF0

    // The light-accumulation buffer the index selects in the collection:
    // its own pair when a scene context is active, else the active frame
    // interval's partial-framerate buffer. Inlined into every stage. Name
    // not in the reference map.
    static RndTextureBase* _GetLightAccum(const RndBufferCollection& buffers, unsigned long index) {
        return buffers.mActiveSceneContext != 0
            ? buffers.mLightAccum[index]
            : buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval].mPartialLightAccum;
    }
    // The resource state a light-accumulation buffer is read in: pixel
    // shaders, and the compute shaders too (0x40) with tiled lighting.
    // Inlined into the stages, for example at 0x63082B. Name not in the
    // reference map.
    static RndResourceState _SceneReadState() {
        constexpr unsigned int kNonPixelShaderResource = 0x40;
        const unsigned int read = static_cast<unsigned int>(RndResourceState::kPixelShaderResource);
        return static_cast<RndResourceState>(
            TheRndDevice()->mSettings->mUseTiledLighting ? read | kNonPixelShaderResource : read);
    }
    // Swaps the source and destination buffers and forgets the separate
    // result texture after a stage drew. Inlined into the stages; the
    // slot-42 overrides (0x631430, 0x633880, 0x634380) are this body alone.
    // Name not in the reference map.
    static void _SwapLightAccum(RndSceneDrawTarget& target) {
        const unsigned long source = target.mSrcLightAccum;
        target.mSrcLightAccum = target.mDstLightAccum;
        target.mDstLightAccum = source;
        target.mResult = nullptr;
    }

    // Registers the class. Emitted with the renderer's components at
    // 0x3F1440. Not reconstructed: the registration helpers it inlines are
    // not modelled.
    static void Init();
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x4047F0
    // Registers the class description and "enabled". Not reconstructed:
    // ComMetaData's string and list members are written inline through
    // helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x631990

    // The object's statics, in the order of its static initializer
    // (0x631E20). The three ints it first sets (0x1AAAC98: -1, the invalid
    // GameObjectId; 0x1AAAC9C: 8; 0x1AAACA0: 4) come from a shared header
    // and are not modelled.
    static Symbol sId;  // 0x1AAACA8, "PostProcStage"
    // The class's second symbol, also "PostProcStage"; GameObject's
    // base-class lookups match it. Name not in the reference map.
    static Symbol sClassName;           // 0x1AAACB0
    static PropRegistry sPropRegistry;  // 0x1AAACC0
    static ComMetaData sMetaData;       // 0x1AAAD60

    // "enabled": whether the stage draws. Field name not in the reference
    // map.
    bool mEnabled;
};

static_assert(offsetof(RndPostProcStageCom, mEnabled) == 0x16);
static_assert(sizeof(RndPostProcStageCom) == 0x18);
