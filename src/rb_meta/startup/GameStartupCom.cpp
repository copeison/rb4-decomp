// rb_meta/GameStartupCom.o (0xDB23B0 to 0xDB39CF). The property accessors
// are the lambdas after the identity slots (0xDB38C0-0xDB38F0). The static
// initializer is at 0xDB3900. The class's registration and factory are
// emitted elsewhere (0xB643C0, 0xBA0BD0).
#include "rb_meta/startup/GameStartupCom.h"

#include <cstring>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/resources/ResourceLoader.h"
#include "os/content/ContentMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/system/System.h"
#include "rb_meta/character/RBCharacterPreviewMgrCom.h"
#include "rb_meta/dialog/DialogMgrCom.h"
#include "rb_meta/songs/RBSongJumpListProviderCom.h"
#include "rb_meta/songs/RBSongLibSortListProviderCom.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "rb_world/characters/RBMusicianInstanceCom.h"
#include "rb_world/perf/RBPerfTestMgr.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "utl/text/Str.h"
#include "utl/time/Timer.h"

namespace {

// The id of no object, at 0x1B25E78, which the static initializer sets.
// Each object that uses it has its own copy. Name not in the reference
// map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// Times the startup states; the times it measures are only logged in
// debug builds. At 0x1B25E80. Name not in the reference map.
Hmx::Timer gStartupTimer = {0, 0, 0};

// The shell's sound banks, whose load lets the meta state apply the volume
// options. Name not in the reference map.
const char* const kShellBanksScenePath = "audio_layers/shell_banks.scene";

}  // namespace

// The object's statics, in the order of its static initializer (0xDB3900).
Symbol GameStartupCom::sId("GameStartupCom");
Symbol GameStartupCom::sClassName("GameStartupCom");
PropRegistry GameStartupCom::sPropRegistry;
ComMetaData GameStartupCom::sMetaData;

// Inlined into the component's destructor.
GameStartupCom::RuntimeData::~RuntimeData() {
    ResourceLoader::ReleaseLoader(mTitleLoader);
}

// Reconstructed from eboot.elf at 0xDB23B0. The deleting destructor is at
// 0xDB2410.
GameStartupCom::~GameStartupCom() {}

// Reconstructed from eboot.elf at 0xDB2480.
void GameStartupCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Handles showing logos at startup.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());

    GameStartupCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }

    eastl::vector<AllowedValue<int>> states;
    states.push_back(AllowedValue<int>{kStartupStateESRBLogo, String("Showing ESRB Logo"), String("")});
    states.push_back(AllowedValue<int>{kStartupStateHarmonixLogo, String("Showing Harmonix Logo"), String("")});
    states.push_back(AllowedValue<int>{kStartupStateGoToTitle, String("Go to title screen."), String("")});

    IntMetadata<int>& queuedState = static_cast<IntMetadata<int>&>(
        *registry
             .RegisterProp(
                 "queued_state",
                 offsetof(GameStartupCom, mRuntime) + offsetof(RuntimeData, mQueuedState),
                 kPropertyInt,
                 sizeof(StartupState))
             .mMetadata.Get());
    queuedState.mDefault = prototype.mRuntime.mQueuedState;
    static_cast<void>(Symbol("prop"));
    queuedState.mHelp = String("The next state we should go into.");
    queuedState.mSaves = false;
    queuedState.mAllowedValues = states;

    BoolMetadata& isBusy = TypeSpecificMetadata(
        registry.RegisterProp(
            "is_busy", offsetof(GameStartupCom, mRuntime) + offsetof(RuntimeData, mIsBusy), kPropertyBool, sizeof(bool)),
        prototype.mRuntime.mIsBusy);
    isBusy.mDefault = prototype.mRuntime.mIsBusy;
    static_cast<void>(Symbol("prop"));
    isBusy.mHelp = String("Is the game startup com busy loading assets for the current startup phase?");
    isBusy.mSaves = false;

    const Symbol symbolType;
    SymbolMetadata& region = TypeSpecificMetadata(registry.RegisterProp("region", -1, kPropertySymbol, 0), symbolType);
    static_cast<void>(Symbol("prop"));
    region.mHelp = String("What region are we in?");
    region.mReadOnly = true;
    region.mGetter = [](const PropAccessorArgs&) { return PlatformRegionToSymbol(ThePlatformMgr->GetRegion()); };

    SymbolMetadata& language =
        TypeSpecificMetadata(registry.RegisterProp("language", -1, kPropertySymbol, 0), symbolType);
    static_cast<void>(Symbol("prop"));
    language.mHelp = String("What language are we in?");
    language.mReadOnly = true;
    language.mGetter = [](const PropAccessorArgs&) { return SystemLanguage(); };

    const bool boolType = false;
    BoolMetadata& perfTestMode =
        TypeSpecificMetadata(registry.RegisterProp("perf_test_mode", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    perfTestMode.mReadOnly = true;
    perfTestMode.mGetter = [](const PropAccessorArgs&) { return RBPerfTestMgr::PerfTestModeEnabled(); };
}

// Reconstructed from eboot.elf at 0xDB2C40.
void GameStartupCom::_Enter() {
    static_cast<void>(gStartupTimer.SplitMs());
    gStartupTimer.Restart();
}

// Reconstructed from eboot.elf at 0xDB2CB0.
void GameStartupCom::_Poll() {
    mObject->mEntity->QueueForPostPoll(this);
}

// Reconstructed from eboot.elf at 0xDB2CD0. The scenes and the loaders are
// followed in order, and the first one still loading keeps the component
// busy.
void GameStartupCom::_PostPoll() {
    if (mRuntime.mRefreshingContent && TheContentMgr.IsRefreshDone()) {
        TheContentMgr.SetDiscoverIncludedContentOnly(false);
        mRuntime.mRefreshingContent = false;
        static_cast<void>(gStartupTimer.SplitMs());
    }
    mRuntime.mIsBusy = mRuntime.mRefreshingContent;

    while (!mRuntime.mPreloadingScenes.empty()) {
        const ResourcePath scene = mRuntime.mPreloadingScenes.front();
        if (!theUI->IsPreloadedScene(scene, false)) {
            mRuntime.mIsBusy = true;
            break;
        }
        if (strcmp(scene.Str(), kShellBanksScenePath) == 0) {
            theRBMetaState.mApplyVolumeOptions = true;
        }
        mRuntime.mPreloadingScenes.erase(mRuntime.mPreloadingScenes.begin());
    }

    while (!mRuntime.mLoaders.empty()) {
        if (!mRuntime.mLoaders.front()->mDone) {
            mRuntime.mIsBusy = true;
            break;
        }
        mRuntime.mLoaders.erase(mRuntime.mLoaders.begin());
    }

    if (mRuntime.mIsBusy || mRuntime.mQueuedState == mRuntime.mState) {
        return;
    }
    mRuntime.mState = mRuntime.mQueuedState;
    switch (mRuntime.mState) {
    case kStartupStateESRBLogo:
        static_cast<void>(gStartupTimer.SplitMs());
        gStartupTimer.Restart();
        if (!TheContentMgr.IsRefreshInProgress()) {
            TheContentMgr.SetDiscoverIncludedContentOnly(true);
            TheContentMgr.ForceDirty();
            TheContentMgr.StartRefresh();
        }
        mRuntime.mRefreshingContent = true;
        _PreloadScenes(0);
        _PreloadMusician(0);
        _PreloadMusician(1);
        _PreloadMusician(2);
        _PreloadMusician(3);
        _PreloadScenes(1);
        break;
    case kStartupStateHarmonixLogo:
        static_cast<void>(gStartupTimer.SplitMs());
        gStartupTimer.Restart();
        break;
    case kStartupStateGoToTitle:
        static_cast<void>(gStartupTimer.SplitMs());
        gStartupTimer.Restart();
        _GotoTitle();
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0xDB30B0.
void GameStartupCom::_PreloadScenes(int phase) {
    if (RBPerfTestMgr::PerfTestModeEnabled()) {
        return;
    }
    UIMgr::PreloadArgs args = {false, true, false};
    if (phase == 0) {
        args.mCopyOnInstance = true;
        _QueueSceneForPreload(ResourcePath(kShellBanksScenePath), args);
        _QueueSceneForPreload(ResourcePath(RBSongJumpListProviderCom::kJumpListScenePath), args);
        _QueueSceneForPreload(ResourcePath(RBSongLibSortListProviderCom::kSortListScenePath), args);
        _QueueSceneForPreload(ResourcePath("char/character_preload.scene"), args);
        static Symbol charPreview;
        if (charPreview == Symbol()) {
            charPreview = Symbol("char_preview");
        }
        if (RndSceneInstanceCom::ShouldLoadInstanceWithTag(charPreview)) {
            _QueueSceneForPreload(ResourcePath(RBCharacterPreviewMgrCom::kCharPreviewScenePath), args);
            _QueueSceneForPreload(ResourcePath(RBCharacterPreviewMgrCom::kCharPreviewBackplateScenePath), args);
        }
    } else if (phase == 1) {
        _QueueSceneForPreload(ResourcePath("fonts/preloaded_fonts.scene"), args);
        _QueueSceneForPreload(ResourcePath("ui/shared/preload.scene"), args);
        _QueueSceneForPreload(ResourcePath(DialogMgrCom::kDialogScenePath), args);
        args.mCopyOnInstance = false;
        static Symbol metaBackground;
        if (metaBackground == Symbol()) {
            metaBackground = Symbol("meta_background");
        }
        if (RndSceneInstanceCom::ShouldLoadInstanceWithTag(metaBackground)) {
            _QueueSceneForPreload(ResourcePath("ui/background/meta_background.scene"), args);
        }
        const bool start = true;
        mRuntime.mTitleLoader = ResourceLoader::AcquireLoader(ResourcePath("ui/title/title.layout"), start);
        mRuntime.mLoaders.push_back(mRuntime.mTitleLoader);
    }
}

// Reconstructed from eboot.elf at 0xDB3470.
void GameStartupCom::_PreloadMusician(int player) {
    if (RBPerfTestMgr::PerfTestModeEnabled()) {
        return;
    }
    mRuntime.mLoaders.push_back(RBMusicianInstanceCom::PreloadMusician(player));
}

// Reconstructed from eboot.elf at 0xDB3590.
void GameStartupCom::_QueueSceneForPreload(ResourcePath path, UIMgr::PreloadArgs& args) {
    mRuntime.mPreloadingScenes.push_back(path);
    theUI->PreloadScene(path, args);
}

// Reconstructed from eboot.elf at 0xDB36B0.
Symbol GameStartupCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xDB36C0.
Symbol GameStartupCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xDB36D0.
int GameStartupCom::CurrentRev() const {
    return const_cast<GameStartupCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xDB36F0.
bool GameStartupCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xDB3720.
Component* GameStartupCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xDB3730. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* GameStartupCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<GameStartupCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) GameStartupCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xDB3890.
PropRegistry& GameStartupCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xDB38A0.
ComMetaData& GameStartupCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xDB38B0.
bool GameStartupCom::_HasPostPoll() const {
    return true;
}

// Reconstructed from eboot.elf at 0xBA0BD0. The binary emits the factory
// with the other rb_meta factories.
Component* GameStartupCom::_Create() {
    return new GameStartupCom();
}

// Reconstructed from eboot.elf at 0xB643C0. The binary emits the
// registration with the other rb_meta classes'.
void GameStartupCom::Init() {
    InitComponentClass<GameStartupCom>();
}
