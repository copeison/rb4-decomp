#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "rb_meta/character/RBCharacterSceneTrackerCom.h"
#include "utl/text/Symbol.h"

class BandUser;
class Entity;
class RndSceneInstanceCom;

// The component of a UI layout that tracks its character entities ("Handles
// layout-level tracking of character entities. Points at a scene
// containing characters and an RBCharacterSceneTrackerCom, and forwards
// requests on to it."; rb_meta/RBCharacterLayoutTrackerCom.o,
// 0xE5A540-0xE5AEFF with the static initializer at 0xE5AE30). Loading finds
// the "character_scene_obj_id" object's scene instance, and entering finds
// the RBCharacterSceneTrackerCom on that scene's root. The vtable at
// 0x1987D68 has 41 slots. The object is 56 bytes.
class RBCharacterLayoutTrackerCom : public Component {
public:
    RBCharacterLayoutTrackerCom();  // 0xE5A540
    // Copies the property for an imprint and starts afresh. Inlined into
    // _Imprint (0xE5ACD0). Not in the reference map.
    RBCharacterLayoutTrackerCom(const RBCharacterLayoutTrackerCom& other)
        : Component(other),
          mCharacterSceneObj(nullptr),
          mSceneInstance(nullptr),
          mSceneTracker(nullptr),
          mCharacterSceneObjId(other.mCharacterSceneObjId) {}
    // Slots 0-1: 0xE5A580, 0xE5A590.
    ~RBCharacterLayoutTrackerCom() override;

    Symbol GetId() const override;          // slot 4: 0xE5AC50
    Symbol GetClassName() const override;   // slot 5: 0xE5AC60
    int CurrentRev() const override;        // slot 7: 0xE5AC70
    bool IsA(Symbol type) const override;   // slot 8: 0xE5AC90
    Component* AsComponent() override;      // slot 9: 0xE5ACC0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xE5ACD0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xE5AE10
    ComMetaData& _GetMetaData() override;       // slot 23: 0xE5AE20
    // Slot 29: finds the character scene object and its scene instance.
    // The map names the class's override _LoadResources(ObjPtr const&);
    // this build overrides the slot after the resources load.
    bool _OnResourcesLoaded() override;  // 0xE5A980
    // Slot 31: finds the RBCharacterSceneTrackerCom on the root of the
    // scene instance's entity. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xE5AA20

    // The scene tracker's answers, or none without a tracker.
    bool AnyPlayerEntityIsLoading() const;  // 0xE5AAA0
    Entity* GetPlayerEntityByInstrument(const MusicianType::Instrument& instrument) const;  // 0xE5AAC0
    Entity* GetPlayerEntityForUser(const BandUser* user) const;  // 0xE5AAE0
    ResourcePath GetPlayerEntityPathByInstrument(const MusicianType::Instrument& instrument) const;  // 0xE5AB00
    ResourcePath GetPlayerEntityPathForUser(const BandUser* user) const;  // 0xE5AB70
    // The tracker on the root of theUI's current layout, or null. Name not
    // in the reference map.
    static RBCharacterLayoutTrackerCom* GetForCurrentLayout();  // 0xE5ABE0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB750E0.
    static void Init();
    // The class factory, emitted at 0xBA63C0; the placement factory beside
    // it (0xBA63F0) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xE5A5B0

    // "RBCharacterLayoutTrackerCom".
    static Symbol sId;  // 0x1B328F8
    // Also "RBCharacterLayoutTrackerCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B32900
    static PropRegistry sPropRegistry;  // 0x1B32910
    static ComMetaData sMetaData;       // 0x1B329B0

    // Field names are not in the reference map.
    // The "character_scene_obj_id" object, found on loading.
    GameObject* mCharacterSceneObj;
    // That object's scene instance.
    RndSceneInstanceCom* mSceneInstance;
    // The tracker on the scene's root, found on entering.
    RBCharacterSceneTrackerCom* mSceneTracker;
    // "character_scene_obj_id": the object holding the SceneInstance where
    // the characters are.
    GameObjectId mCharacterSceneObjId;
};

static_assert(offsetof(RBCharacterLayoutTrackerCom, mCharacterSceneObj) == 24);
static_assert(offsetof(RBCharacterLayoutTrackerCom, mSceneInstance) == 32);
static_assert(offsetof(RBCharacterLayoutTrackerCom, mSceneTracker) == 40);
static_assert(offsetof(RBCharacterLayoutTrackerCom, mCharacterSceneObjId) == 48);
static_assert(sizeof(RBCharacterLayoutTrackerCom) == 56);
