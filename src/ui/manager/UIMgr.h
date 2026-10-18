#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "entity/resources/Resource.h"
#include "entity/resources/ResourceLoader.h"
#include "ui/layout/UILayoutResource.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class DataArray;
class Entity;
class JoypadClient;
class RndSceneCom;
class RndSceneInstanceCom;

// Loads a scene for UIMgr::PreloadScene on the call thread and, once it is
// loaded, prepares it as the preload arguments ask. Its inline destructors
// are emitted at 0x8CC290 and 0x8CC2D0, with the vtable at 0x195E518; the
// object is 40 bytes.
class SceneLoader : public ResourceLoader {
public:
    // The map's SceneLoader(ResourcePath, bool); this build passes the
    // path by reference and adds the second flag. Inlined into
    // PreloadScene.
    SceneLoader(const ResourcePath& path, bool copyOnInstance, bool enterOnLoad);  // 0x8CAF20
    ~SceneLoader() override {}

    // Slot 3: marks the load done and prepares a loaded scene.
    void ThreadDone(int result) override;  // 0x8CAF50

    // Field names are not in the reference map.
    // UIMgr::PreloadArgs::mCopyOnInstance and mEnterOnLoad.
    bool mCopyOnInstance;
    bool mEnterOnLoad;
};

static_assert(offsetof(SceneLoader, mCopyOnInstance) == 32);
static_assert(offsetof(SceneLoader, mEnterOnLoad) == 33);
static_assert(sizeof(SceneLoader) == 40);

// The UI manager (ui/UIMgr.o, 0x8C8440 to 0x8CC5B7). It owns the current
// layout, moves to a new one through a transition, keeps preloaded scenes,
// and passes keyboard and joypad input to the layout. theUI holds the one
// instance, which scripts reach as the "ui" global. The vtable at
// 0x195E4D0 has MsgSource's seven slots; the object is 184 bytes.
class UIMgr : public MsgSource {
public:
    // How PreloadScene loads a scene. The map names the type; the field
    // names are not in the reference map.
    struct PreloadArgs {
        // Loads the scene at once instead of on the call thread.
        bool mLoadNow;
        // Sets the scene's "copy_on_instance" header property.
        bool mCopyOnInstance;
        // Enters the scene's entity once it is loaded on the call thread.
        bool mEnterOnLoad;
    };

    // The steps of a transition (mTransitionState). Names not in the
    // reference map.
    enum TransitionState : int {
        kTransitionNone = 0,
        // Goto started it; the next poll starts the load.
        kTransitionStarted = 1,
        // The new layout loads while the old one exits.
        kTransitionLoading = 2,
        // The new layout entered and waits for its scenes' enter
        // transitions.
        kTransitionEntered = 3,
    };

    // Creates the joypad client, subscribes to the keyboard, sets the "ui"
    // global and reads the "ui" config.
    UIMgr();  // 0x8C8440
    // Slots 0-1: 0x8C8710, 0x8C8860.
    ~UIMgr() override;
    // Slot 2: passes key, button and joypad connection messages to the
    // layout and handles the script commands.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x8CBAE0

    // Creates theUI. The map's Init() is a member of the older manager.
    static void Init();  // 0x8C8880
    // Deletes the joypad client and leaves the keyboard source.
    void Terminate();  // 0x8C88B0
    // Subscribes to or leaves the keyboard and the joypad client's source.
    void EnableInputHandling(bool enable);  // 0x8C86B0
    // Advances the transition and polls the current layout, then collects
    // the scenes the loaders finished.
    void Poll();  // 0x8C8900
    bool IsInTransition() const;  // 0x8C96A0
    // Releases the current layout while the allocator batches the frees.
    // Emitted at 0x8C96B0 and inlined into its users. Name not in the
    // reference map.
    void ReleaseCurrentLayout();
    // Draws the current layout into the device's targets, or clears them
    // when there is no layout.
    void Draw();  // 0x8C9A80
    // Starts a transition to the layout. The map's Goto(ResourcePtr<
    // UILayoutResource>) and Goto(ResourcePath const&).
    void Goto(ResourcePtr<UILayoutResource> layout);  // 0x8C9BE0
    void Goto(const ResourcePath& path);              // 0x8C9F50
    // Whether a transition's layout finished loading. No caller in this
    // build. Name not in the reference map; the evidence is weak.
    bool IsTransitionReady() const;  // 0x8CA310
    // Queue scene changes on the current layout, unless there is none or
    // it is being released (UILayoutResource::SceneQueueFlags). The map's
    // AddScene(ResourcePath const&), AddSceneAtDrawOrder(ResourcePath
    // const&, int) and RemoveScene(ResourcePath const&); this build adds
    // the flags and places a scene next to another scene object.
    // AddSceneAfterObject is not in the reference map.
    void AddScene(const ResourcePath& path, unsigned int flags);  // 0x8CA330
    void AddSceneAtDrawOrder(const ResourcePath& path, GameObjectId before, unsigned int flags);  // 0x8CA3A0
    void AddSceneAfterObject(const ResourcePath& path, GameObjectId after, unsigned int flags);  // 0x8CA400
    void RemoveScene(const ResourcePath& path, unsigned int flags);  // 0x8CA460
    // The current layout, or null while it is being released.
    UILayoutResource* GetCurrentLayout() const;  // 0x8CA4C0
    void SetSceneShowing(const ResourcePath& path, bool showing);  // 0x8CA4E0
    // Makes the preloaded layout at the path current without a transition.
    // The old layout is released, or deactivated and kept with the
    // preloaded scenes. No caller in this build. Name not in the reference
    // map.
    void SwapToPreloadedLayout(const ResourcePath& path, bool releaseCurrent);  // 0x8CA530
    // Whether the scene is preloaded or, with `includeLoading`, loading.
    bool IsPreloadedScene(const ResourcePath& path, bool includeLoading) const;  // 0x8CA800
    // Makes the preloaded layout at the path current, enters and polls it,
    // and sends the end-of-transition messages. No caller in this build.
    // Name not in the reference map.
    void GotoPreloadedLayout(const ResourcePath& path);  // 0x8CA870
    // Loads the scene and keeps it with the preloaded scenes, unless
    // preloading is disabled or the scene is loaded or loading.
    void PreloadScene(const ResourcePath& path, const PreloadArgs& args);  // 0x8CB2D0
    // Releases the preloaded scene at the path; true when there was one.
    // Name not in the reference map.
    bool UnloadPreloadedScene(const ResourcePath& path);  // 0x8CB620
    // The preloaded scene at the path, or null. No caller in this build.
    // Name not in the reference map.
    ResourcePtr<EntityResource> GetPreloadedScene(const ResourcePath& path) const;  // 0x8CB750

    // The message handlers, inlined into Handle. The input handler sends
    // the message to the layout's scenes until one handles it, outside a
    // transition; the connection handler sends it to every scene.
    DataNode _OnInputMsg(DataArray* msg);        // 0x8CB7B0
    DataNode _OnConnectMsg(DataArray* msg);      // 0x8CB820
    DataNode _OnGoto(const DataArray* msg);      // 0x8CB870
    DataNode _OnAddScene(const DataArray* msg);  // 0x8CB910
    DataNode _OnRemoveScene(const DataArray* msg);  // 0x8CB9D0
    // The "set_reload_on_transition" handler. Name not in the reference
    // map.
    DataNode _OnSetReloadOnTransition(const DataArray* msg);  // 0x8CBA90

private:
    // Exports "transition_started" with the old and new paths, acquires the
    // loader of the new layout and exits the old one. Nothing happens
    // during a transition or for the current layout's path.
    void _StartTransition(const ResourcePath& path);  // 0x8C9C30
    // Resets the draw registrations of both layouts' scenes before the old
    // one is released. Name not in the reference map.
    void _ResetSceneDrawRegistrations(
        ResourcePtr<UILayoutResource> from,
        ResourcePtr<UILayoutResource> to);  // 0x8C9900
    // The scene components of the two lists' instanced entities, the
    // first list's first. Name not in the reference map.
    eastl::vector<RndSceneCom*> _GetSceneComs(
        const eastl::vector<RndSceneInstanceCom*>& first,
        const eastl::vector<RndSceneInstanceCom*>& second) const;  // 0x8C9F60

public:
    // Field names are not in the reference map.
    JoypadClient* mJoypadClient;
    ResourcePtr<UILayoutResource> mCurrentLayout;
    TransitionState mTransitionState;
    // The loader of the transition's layout.
    ResourceLoader* mLayoutLoader;
    // The "ui" config's preloading_enabled, true by default.
    bool mPreloadingEnabled;
    eastl::vector<SceneLoader*> mSceneLoaders;
    // The preloaded scenes. The map's vector holds
    // ResourcePtr<RndSceneResource>; this build keeps layouts there too.
    eastl::vector<ResourcePtr<EntityResource>> mPreloadedScenes;
    // The path of the layout the transition left, for the
    // "transition_done" messages.
    Symbol mPreviousLayoutName;
    // The "ui" config's reload_on_transition: the old layout is released
    // before the new one loads.
    bool mReloadOnTransition;
    // Set while ReleaseCurrentLayout releases the layout; GetCurrentLayout
    // and the scene functions then see no layout.
    bool mReleasingLayout;
    // The keyboard subscription.
    MsgSource::EventSinkElem mKeyboardSink;
};

static_assert(offsetof(UIMgr, mJoypadClient) == 24);
static_assert(offsetof(UIMgr, mCurrentLayout) == 32);
static_assert(offsetof(UIMgr, mTransitionState) == 40);
static_assert(offsetof(UIMgr, mLayoutLoader) == 48);
static_assert(offsetof(UIMgr, mPreloadingEnabled) == 56);
static_assert(offsetof(UIMgr, mSceneLoaders) == 64);
static_assert(offsetof(UIMgr, mPreloadedScenes) == 96);
static_assert(offsetof(UIMgr, mPreviousLayoutName) == 128);
static_assert(offsetof(UIMgr, mReloadOnTransition) == 136);
static_assert(offsetof(UIMgr, mReleasingLayout) == 137);
static_assert(offsetof(UIMgr, mKeyboardSink) == 144);
static_assert(sizeof(UIMgr) == 184);

// The UI manager, created by UIMgr::Init and deleted by UI::Terminate.
extern UIMgr* theUI;  // 0x1AC9A78
