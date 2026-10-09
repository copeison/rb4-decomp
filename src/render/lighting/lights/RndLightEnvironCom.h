#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Entity;
class RndBufferCollection;
class RndContext;
class RndLightCom;
struct RndLightProbeParams;
struct RndSceneDrawTarget;

// A light environment: a set of lights that objects are lit by
// (render/RndLightEnvironCom.o, 0x478B70-0x47A3BB). Its class id is
// "LightEnviron". The scene's light manager indexes up to six environments;
// each light sets the bit of every environment it belongs to in its
// "environ_bits", and the deferred pass draws an environment's lights into
// the pixels whose stencil carries its index. The vtable at 0x1905D68 has
// 41 slots. The object is 248 bytes; everything after Component is run-time
// state.
class RndLightEnvironCom : public Component {
public:
    // The index of an environment the light manager does not index. Name
    // not in the reference map.
    static constexpr int kNoEnvironIndex = -2;

    // The run-time state. The constructor is inlined into the component's
    // (0x478B70) and its copy; the destructor is at 0x47A0A0. The map names
    // the type; its field names are not in the reference map.
    struct RuntimeData {
        RuntimeData()
            : mRegistered(false),
              mEnvironIndex(kNoEnvironIndex),
              mHasCulledDirectionalLight(false) {}
        ~RuntimeData();  // 0x47A0A0

        // Set while the environment is registered with its scene's light
        // manager (_Enter); RndLightMgrCom::_Exit clears it when instanced
        // entities exit.
        bool mRegistered;
        // "environ_index": the environment's bit in the lights'
        // "environ_bits", or kNoEnvironIndex.
        int mEnvironIndex;
        // The lights in the environment.
        eastl::vector<RndLightCom*> mLights;
        // "sibling_lights": the lights in the environment's own entity.
        PropArray<GameObjectId> mSiblingLights;
        // "nested_lights": the lights in nested entities, with their
        // entities beside them.
        PropArray<GameObjectId> mNestedLights;
        eastl::vector<Entity*> mNestedLightEntities;
        // The lights the last culling kept, and those that subtract light.
        eastl::vector<RndLightCom*> mCulledLights;
        eastl::vector<RndLightCom*> mCulledNegativeLights;
        bool mHasCulledDirectionalLight;
    };

    RndLightEnvironCom();  // 0x478B70
    // Copies the component for an imprint; the run-time state starts
    // afresh. Inlined into _Imprint.
    RndLightEnvironCom(const RndLightEnvironCom& other) : Component(other) {}
    ~RndLightEnvironCom() override;  // slots 0-1: 0x478D90, 0x478DC0

    Symbol GetId() const override;         // slot 4: 0x479DE0
    Symbol GetClassName() const override;  // slot 5: 0x479DF0
    int CurrentRev() const override;       // slot 7: 0x479E00
    bool IsA(Symbol type) const override;  // slot 8: 0x479E20
    Component* AsComponent() override;     // slot 9: 0x479E50
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x479E60
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x47A080
    ComMetaData& _GetMetaData() override;       // slot 23: 0x47A090
    // Slot 31: registers with the scene's light manager.
    void _Enter() override;  // 0x479B80
    // Slot 32: leaving the object or the component unregisters from the
    // light manager, and stops it using the environment as its default;
    // every exit forgets the lights.
    void _Exit(DestroyType type) override;  // 0x479C70
    // Slots 36-37 at 0x479D50 and 0x479DD0: as in the game mode.
    void _EditEnter() override;
    void _EditExit(DestroyType type) override;

    // Moves the environment's bit in its lights' "environ_bits" to the new
    // index. The map's SetEnvironIndex(EntityPtr const&,
    // RndLightEnvironIndex).
    void SetEnvironIndex(int index);  // 0x478E00
    // Adds the light to the environment and sets the environment's bit in
    // its "environ_bits". The map's LightAdded(ObjPtr const&, ObjPtr
    // const&).
    void LightAdded(RndLightCom* light);  // 0x478EC0
    // Removes the light and clears the environment's bit. The map's
    // LightRemoved(ObjPtr const&, ObjPtr const&).
    void LightRemoved(RndLightCom* light);  // 0x4790E0
    // Empties the culled-light lists.
    void ClearCulledLights();  // 0x479300
    // Draws the culled lights with the stencil test on the environment's
    // index: the first directional light gets the probe parameters, then
    // the subtracting lights blend on top. The map's signature is
    // DrawDeferredLightNoCompute(RndContext&).
    void DrawDeferredLightNoCompute(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        const RndLightProbeParams& probe);  // 0x479330

    // Adds a light the culling kept: subtracting lights (illumination type
    // 3) to their own list, and a directional light marks the list. Emitted
    // in render/RndLightMgrCom.o at 0x48AE80. Name not in the reference map.
    void AddCulledLight(RndLightCom* light, int type, int illuminationType) {
        if (illuminationType == 3) {
            mRuntime.mCulledNegativeLights.push_back(light);
            return;
        }
        mRuntime.mCulledLights.push_back(light);
        if (type == 2) {
            mRuntime.mHasCulledDirectionalLight = true;
        }
    }

    // The class factory. Emitted at 0x404390 with the other render
    // factories.
    static Component* _Create();

    // Registers "environ_index", "sibling_lights" and "nested_lights". Not
    // reconstructed: the property metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x479650

    static Symbol sId;                  // 0x1A87B60, "LightEnviron"
    // Also "LightEnviron". Name not in the reference map.
    static Symbol sClassName;           // 0x1A87B68
    static PropRegistry sPropRegistry;  // 0x1A87B70
    static ComMetaData sMetaData;       // 0x1A87C10

    RuntimeData mRuntime;
};

static_assert(offsetof(RndLightEnvironCom, mRuntime) == 24);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mEnvironIndex) == 4);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mLights) == 8);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mSiblingLights) == 40);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mNestedLights) == 80);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mNestedLightEntities) == 120);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mCulledLights) == 152);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mCulledNegativeLights) == 184);
static_assert(offsetof(RndLightEnvironCom::RuntimeData, mHasCulledDirectionalLight) == 216);
static_assert(sizeof(RndLightEnvironCom) == 248);
