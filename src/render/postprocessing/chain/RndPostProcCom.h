#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "render/drawing/RndSceneInternalContext.h"
#include "utl/text/Symbol.h"

class RndContext;
class RndSceneDrawParams;
struct RndSceneBatchContext;
struct RndSceneDrawTarget;

// A scene's post-processing chain (render/RndPostProcCom.o,
// 0x62EF10-0x62F9FF). Its class id is "PProc". The scene names up to two
// chains ("post_proc_0" and "post_proc_1"); the scene drawer runs the
// chain of each camera after the scene is drawn (RndSceneDrawer::
// _DrawPostProc at 0x425510), and the threaded scene draw asks it to
// update the draw targets the same way (0x431AB0). Each stage is a
// RndPostProcStageCom on an object of the scene entity. The vtable at
// 0x192E2A0 has 41 slots. The object is 64 bytes.
class RndPostProcCom : public Component {
public:
    RndPostProcCom();            // 0x62EF10
    // Copies the stage list for an imprint; inlined into _Imprint at
    // 0x62F7F4. The array copy is skipped while the thread imprints, which
    // leaves the copy empty until its properties are read.
    RndPostProcCom(const RndPostProcCom& other) : Component(other) {
        mStages.mElemSize = other.mStages.mElemSize;
        mStages.mType = other.mStages.mType;
        mStages._Copy(other.mStages);
    }
    ~RndPostProcCom() override;  // slots 0-1: 0x62EF70, 0x62EFE0

    Symbol GetId() const override;         // slot 4: 0x62F730
    Symbol GetClassName() const override;  // slot 5: 0x62F740
    int CurrentRev() const override;       // slot 7: 0x62F750
    bool IsA(Symbol type) const override;  // slot 8: 0x62F770
    Component* AsComponent() override;     // slot 9: 0x62F7A0
    // Slot 10. The map's _Imprint(char*, Component*&, bool). Copies the
    // stage list.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x62F7B0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x62F910
    ComMetaData& _GetMetaData() override;       // slot 23: 0x62F920
    // Slot 29 at 0x62F650: nothing to load; true. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 38 at 0x62F640: empty. The map's _EditPoll(ObjPtr const&).
    void _EditPoll() override;

    // Draws each stage in order; a stage's object must hold a stage
    // component. The map's Draw(RndContext&, EntityPtr const&,
    // RndSceneDrawParams const&) const; this build drops the EntityPtr and
    // passes the camera's draw data and the batch context, whose draw
    // target the stages advance.
    void Draw(
        RndContext& context,
        const RndSceneInternalContext::CameraData& camera,
        const RndSceneDrawParams& params,
        RndSceneBatchContext& batch) const;  // 0x62F060
    // Lets each stage update the draw target as its draw would. Name not
    // in the reference map.
    void UpdateDrawTarget(RndSceneDrawTarget& target) const;  // 0x62F170
    // Whether some stage is a bloom stage; asked at 0x69482E. Name not in
    // the reference map.
    bool HasBloomStage() const;  // 0x62F660

    // Registers the class. Emitted with the renderer's components at
    // 0x3F17F0. Not reconstructed: the registration helpers it inlines are
    // not modelled.
    static void Init();
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x404840
    // Registers the class description and "stages", an array of object
    // references to RndPostProcStageCom objects. Not reconstructed:
    // ComMetaData's string and list members and the array item's metadata
    // are written inline through helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x62F260

    // The object's statics, in the order of its static initializer
    // (0x62F930). The three ints it first sets (0x1AAA798: -1, the invalid
    // GameObjectId; 0x1AAA79C: 8; 0x1AAA7A0: 4) come from a shared header
    // and are not modelled.
    static Symbol sId;  // 0x1AAA7A8, "PProc"
    // The class's second symbol, also "PProc". Name not in the reference
    // map.
    static Symbol sClassName;           // 0x1AAA7B0
    static PropRegistry sPropRegistry;  // 0x1AAA7C0
    static ComMetaData sMetaData;       // 0x1AAA860

    // "stages": the stage objects, applied in order. The map's
    // PropArray<GameObjectId>; the field name is not in the reference map.
    PropArray<GameObjectId> mStages;
};

static_assert(offsetof(RndPostProcCom, mStages) == 0x18);
static_assert(sizeof(RndPostProcCom) == 0x40);
