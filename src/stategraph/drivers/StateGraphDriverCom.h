#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "os/threading/CritSec.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class ResourceLoader;
class StateGraphInstance;
class StateNodeInstance;

// Which sound handle tables a lookup or a removal reaches: the driver's
// own, the global one shared by every driver, or both. The state graph's
// sound nodes pass their "instance_name_is_global" flag. Name not in the
// reference map.
enum SoundHandleScope : int {
    kSoundHandleLocal = 0,
    kSoundHandleGlobal = 1,
    kSoundHandleAny = 2,
};

// The component that runs the StateGraph instances of an entity
// (stategraph/StateGraphDriverCom.o, 0x7013A0-0x703CB5; class id
// "StateGraphDriver"). Each running graph is a StateGraphInstance; the
// driver polls them, deletes those that finish, names the sounds their
// nodes play, and loads the resources the graphs assign to resource path
// properties one at a time. The vtable at 0x193A518 has 41 slots. The
// object is 456 bytes.
class StateGraphDriverCom : public Component {
public:
    // A resource path property waiting for its resource to load before it
    // is set (LoadAndSetResourcePathProperty). The map names the type; the
    // field names are not in the reference map.
    struct ResourceLoaderJob {
        PropRef mRef;
        ResourcePath mPath;
    };

    // The driver's state, kept apart from the component so the imprint
    // copy starts empty. The map emits its constructor (0x7013A0),
    // destructor (0x7014F0) and copy constructor; this build's _Imprint
    // inlines the default constructor instead of copying. Field names are
    // not in the reference map.
    struct RuntimeData {
        RuntimeData();   // 0x7013A0
        ~RuntimeData();  // 0x7014F0

        // The running graphs, in activation order.
        eastl::vector<StateGraphInstance*> mInstances;
        // The sounds the graphs play under a name, by name.
        eastl::map<Symbol, unsigned int> mSoundHandles;
        // The sounds played without a name; _Poll drops those that ended.
        eastl::list<unsigned int> mUnnamedSoundHandles;
        // The property assignments waiting for a loader.
        eastl::vector<ResourceLoaderJob> mLoaderJobs;
        // The property the current loader's resource is assigned to.
        PropRef mLoadingRef;
        ResourceLoader* mLoader;
        // Loaders whose resource was assigned, released on the next poll.
        eastl::vector<ResourceLoader*> mFinishedLoaders;
        // Counts the polls; a graph that polls every other frame polls on
        // the counts of its phase.
        unsigned long mPollCount;
        // Whether the next such graph polls on even counts (phase 1) or
        // odd ones (phase 2); flips with each activation.
        bool mNextPollEven;
        // The clock of the entity, set when the resources are loaded.
        TimeMgr::Clock* mClock;
    };

    StateGraphDriverCom() {}
    ~StateGraphDriverCom() override;  // slots 0-1: 0x7015B0, 0x7015E0

    Symbol GetId() const override;          // slot 4: 0x703660
    Symbol GetClassName() const override;   // slot 5: 0x703670
    int CurrentRev() const override;        // slot 7: 0x703680
    bool IsA(Symbol type) const override;   // slot 8: 0x7036A0
    Component* AsComponent() override;      // slot 9: 0x7036D0
    // Slot 10. The copy starts with empty runtime data.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7036E0
    // Slot 18: the driver comes before the AnimDriverCom.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x702D10
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x703920
    ComMetaData& _GetMetaData() override;       // slot 23: 0x703930
    // Slot 29: takes the entity's clock.
    bool _OnResourcesLoaded() override;  // 0x702DD0
    // Slot 31: activates every state graph inlined into the entity whose
    // root node starts on enter. The map's _Enter calls
    // ActivateStartOnEnters; this build inlines it.
    void _Enter() override;  // 0x702E00
    // Slot 32: stops the sounds, releases the loaders and deletes the
    // instances. Name not in the reference map.
    void _Exit(DestroyType type) override;  // 0x7034F0
    // Slot 33: polls the instances due this frame, deleting those that
    // finish, then assigns the loaded resources and forgets the unnamed
    // sounds that ended.
    void _Poll() override;  // 0x7030D0

    // Starts the state graph, restarting a running instance unless no
    // object is given. The map gives no return type.
    void Activate(ResourcePath path, GameObjectId object);  // 0x701B20
    // Whether an instance of the state graph runs.
    bool IsActive(ResourcePath path) const;  // 0x701D70
    // Stops and deletes the instance of the state graph, if any. No caller
    // in this build.
    void Deactivate(ResourcePath path);  // 0x701DD0
    // Stops and deletes every instance, the last first. No caller in this
    // build.
    void DeactivateAll();  // 0x701EB0
    // The instance of the state graph, or the end of the list. No caller
    // in this build.
    StateGraphInstance** _FindInst(ResourcePath path);              // 0x701E60
    StateGraphInstance* const* _FindInst(ResourcePath path) const;  // 0x701F10

    // Registers a sound under the global name; false while a sound plays
    // under it already. SoundManager::sAddSoundHandleFunc points here.
    static bool AddSoundHandle(unsigned int handle, Symbol name);  // 0x701970
    // Registers a sound under the name in the scope's table, or among the
    // unnamed sounds; false while a sound plays under the name already.
    // Name not in the reference map, which has only the global form.
    bool AddSoundHandle(unsigned int handle, Symbol name, SoundHandleScope scope);  // 0x701F60
    // The sound registered under the name in the scope, local first. Name
    // not in the reference map.
    bool HasSoundHandle(Symbol name, unsigned int& handle, SoundHandleScope scope);  // 0x702180
    // The sound registered under the global name. No caller in this build.
    static bool HasSoundHandle(Symbol name, unsigned int& handle);  // 0x7022E0
    // Forgets every name of the sound in the scope. Name not in the
    // reference map.
    bool RemoveSoundHandle(unsigned int handle, SoundHandleScope scope);  // 0x7023C0
    // Forgets every global name of the sound. No caller in this build.
    // Name not in the reference map.
    static bool RemoveGlobalSoundHandle(unsigned int handle);  // 0x702520
    // Stops every globally named sound and forgets the names. No caller in
    // this build. Name not in the reference map.
    static void StopGlobalSounds();  // 0x7025F0
    // Sets the resource path property once the resource is loaded: at once
    // when it needs no load, otherwise through the queue _Poll works off.
    // A newer assignment to the same property replaces a queued one.
    void LoadAndSetResourcePathProperty(const PropRef& ref, ResourcePath path);  // 0x7026D0
    // Whether a property assignment waits for its load.
    bool IsLoadingResources() const;  // 0x702CA0
    // The first node instance of a running graph that answers for the
    // object (StateNodeInstance slot 11). No caller in this build. Name not
    // in the reference map.
    StateNodeInstance* FindNodeInstance(const GameObject* object);  // 0x702CC0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6F6330.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FED90.
    static Component* _Create();
    // Registers the class description and points
    // SoundManager::sAddSoundHandleFunc at AddSoundHandle; the class has
    // no properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x701620

    static Symbol sId;  // 0x1AB29D0, "StateGraphDriver"
    // The class symbol GameObject::CreateComponent takes; also
    // "StateGraphDriver". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB29D8
    static PropRegistry sPropRegistry;  // 0x1AB29E0
    static ComMetaData sMetaData;       // 0x1AB2A80
    // Guards sGlobalSoundHandles. Name not in the reference map.
    static CritSec sGlobalSoundHandlesLock;  // 0x1AB2988
    // The sounds registered under global names, by name. Name not in the
    // reference map.
    static eastl::map<Symbol, unsigned int> sGlobalSoundHandles;  // 0x1AB2998

    // Name not in the reference map.
    RuntimeData mRuntime;
};

static_assert(offsetof(StateGraphDriverCom::ResourceLoaderJob, mPath) == 216);
static_assert(sizeof(StateGraphDriverCom::ResourceLoaderJob) == 224);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mSoundHandles) == 32);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mUnnamedSoundHandles) == 88);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mLoaderJobs) == 120);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mLoadingRef) == 152);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mLoader) == 368);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mFinishedLoaders) == 376);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mPollCount) == 408);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mNextPollEven) == 416);
static_assert(offsetof(StateGraphDriverCom::RuntimeData, mClock) == 424);
static_assert(offsetof(StateGraphDriverCom, mRuntime) == 24);
static_assert(sizeof(StateGraphDriverCom) == 456);
