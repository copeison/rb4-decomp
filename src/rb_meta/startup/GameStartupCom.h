#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "ui/manager/UIMgr.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class ResourceLoader;

// The startup screen's component ("Handles showing logos at startup.";
// rb_meta/GameStartupCom.o, 0xDB23B0-0xDB39CF with the static initializer
// at 0xDB3900). The layout's script queues the logo states; once nothing is
// loading, the component enters the queued state: the ESRB logo starts the
// content refresh and preloads the shell's scenes, the musicians and the
// title layout, and the last state goes to the title screen. The vtable at
// 0x1981620 has 41 slots. The object is 112 bytes.
class GameStartupCom : public Component {
public:
    // The startup states ("queued_state"); the texts are the allowed
    // values' names. Names not in the reference map.
    enum StartupState {
        kStartupStateNone = 0,
        kStartupStateESRBLogo = 1,
        kStartupStateHarmonixLogo = 2,
        kStartupStateGoToTitle = 3,
    };

    // The component's state, which the imprint's copy starts afresh. The
    // map names the type (its constructor, copy constructor and
    // destructor are inlined here); the field names are not in the
    // reference map.
    struct RuntimeData {
        RuntimeData()
            : mState(kStartupStateNone),
              mQueuedState(kStartupStateNone),
              mIsBusy(false),
              mRefreshingContent(false),
              mTitleLoader(nullptr) {}
        // Releases the title layout's loader.
        ~RuntimeData();

        // The state the component entered last.
        StartupState mState;
        // "queued_state": the state to enter once nothing is loading.
        StartupState mQueuedState;
        // "is_busy": set while a scene, a loader or the content refresh is
        // still under way.
        bool mIsBusy;
        // Set while the content refresh the ESRB state started runs.
        bool mRefreshingContent;
        // The title layout's loader, also in mLoaders.
        ResourceLoader* mTitleLoader;
        // The scenes queued for preloading that have not loaded yet.
        eastl::vector<ResourcePath> mPreloadingScenes;
        // The loaders that have not finished yet.
        eastl::vector<ResourceLoader*> mLoaders;
    };

    // Inlined into the factory and the registration.
    GameStartupCom() : mRuntime() {}
    // The imprint's copy starts afresh. Inlined into _Imprint (0xDB3730).
    GameStartupCom(const GameStartupCom& other) : Component(other), mRuntime() {}
    // Slots 0-1: 0xDB23B0, 0xDB2410.
    ~GameStartupCom() override;

    Symbol GetId() const override;          // slot 4: 0xDB36B0
    Symbol GetClassName() const override;   // slot 5: 0xDB36C0
    int CurrentRev() const override;        // slot 7: 0xDB36D0
    bool IsA(Symbol type) const override;   // slot 8: 0xDB36F0
    Component* AsComponent() override;      // slot 9: 0xDB3720
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xDB3730
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xDB3890
    ComMetaData& _GetMetaData() override;       // slot 23: 0xDB38A0
    // Slot 31: restarts the startup timer. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xDB2C40
    // Slot 33: queues the object for the post-poll. The map's
    // _Poll(ObjPtr const&).
    void _Poll() override;  // 0xDB2CB0
    // Slot 39: true.
    bool _HasPostPoll() const override;  // 0xDB38B0
    // Slot 40: follows the refresh, the queued scenes and the loaders, and
    // enters the queued state once none is left. The map's
    // _PostPoll(ObjPtr const&).
    void _PostPoll() override;  // 0xDB2CD0

    // Phase 0 preloads the shell banks, the song list's jump list and sort
    // list, the character preload scene and, for "char_preview", the
    // character preview's scenes; phase 1 the fonts, the shared preload
    // scene, the dialog, "meta_background"'s scene (not copied on instance)
    // and the title layout's loader. Nothing is preloaded in the
    // performance test mode.
    void _PreloadScenes(int phase);  // 0xDB30B0
    // Starts loading the player's musician and follows its loader.
    void _PreloadMusician(int player);  // 0xDB3470
    // Goes to the title screen and sets a flag of the platform manager
    // (+0x83) and of the server manager (+0x80), which are not modelled.
    // Inlined into _PostPoll, with a release of ContentMgr's (0x393250, an
    // empty function) and the idle archive file handles (0x3A8B60) before
    // it; the binary keeps an uncalled copy. Not reconstructed.
    void _GotoTitle();  // 0xDB3550
    // Follows the scene and asks the UI manager to preload it.
    void _QueueSceneForPreload(ResourcePath path, UIMgr::PreloadArgs& args);  // 0xDB3590

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB643C0.
    static void Init();
    // The class factory, emitted at 0xBA0BD0; the placement factory beside
    // it (0xBA0C50) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xDB2480

    // "GameStartupCom".
    static Symbol sId;  // 0x1B25E98
    // Also "GameStartupCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B25EA0
    static PropRegistry sPropRegistry;  // 0x1B25EB0
    static ComMetaData sMetaData;       // 0x1B25F50

    RuntimeData mRuntime;
};

static_assert(offsetof(GameStartupCom::RuntimeData, mQueuedState) == 4);
static_assert(offsetof(GameStartupCom::RuntimeData, mIsBusy) == 8);
static_assert(offsetof(GameStartupCom::RuntimeData, mTitleLoader) == 16);
static_assert(offsetof(GameStartupCom::RuntimeData, mPreloadingScenes) == 24);
static_assert(offsetof(GameStartupCom::RuntimeData, mLoaders) == 56);
static_assert(offsetof(GameStartupCom, mRuntime) == 24);
static_assert(sizeof(GameStartupCom) == 112);
