// ui/UIMgr.o (0x8C8440 to 0x8CC5B7). The object also emits ResourceLoader's
// inline members (entity/resources/ResourceLoader.h) and the eastl insert of
// the preloaded scenes (0x8CC490). The empty function at 0x8CA390 has no
// caller and no recoverable name.
#include "ui/manager/UIMgr.h"

#include "entity/core/Entity.h"
#include "entity/core/EntityHeaderCom.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "math/color/Color.h"
#include "os/joypads/JoypadClient.h"
#include "os/joypads/Keyboard.h"
#include "os/locale/Locale.h"
#include "os/memory/MemMgr.h"
#include "os/system/System.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndDevice.h"
#include "ui/layout/UILayoutResource.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"

// Sets the property at the path from the value of the type. Emitted in
// entity/PropUtl.o; declared here until PropUtl.h declares it. The map's
// _SetProp(Component&, PropPath const&, PropInfo const&, void*, void
// const*, PropertyType, bool).
bool _SetProp(
    Component& component,
    const PropPath& path,
    const PropInfo& info,
    void* storage,
    const void* value,
    PropertyType type,
    bool notify);  // 0x18A440

UIMgr* theUI;

namespace {

// A node holding the path's characters as a resource path, as the
// transition messages carry them. Name not in the reference map.
DataNode ResourcePathNode(const ResourcePath& path) {
    DataNode node;
    node.mType = kDataResourcePath;
    node.mValue.symbol = path.Str();
    return node;
}

// Whether the layout is there and loaded. Name not in the reference map.
bool IsLoaded(const UILayoutResource* layout) {
    return layout != nullptr && !layout->Fail();
}

}  // namespace

// Reconstructed from eboot.elf at 0x8C8440. EnableInputHandling(true) is
// inlined.
UIMgr::UIMgr()
    : mCurrentLayout(),
      mTransitionState(kTransitionNone),
      mLayoutLoader(nullptr),
      mPreloadingEnabled(true),
      mSceneLoaders(),
      mPreloadedScenes(),
      mPreviousLayoutName(),
      mReloadOnTransition(false),
      mReleasingLayout(false),
      mKeyboardSink() {
    mJoypadClient = new JoypadClient(this);
    mJoypadClient->SetVirtualDpad(true);
    mJoypadClient->SetRepeatMask(0xF000);
    EnableInputHandling(true);

    {
        DataNode self;
        self.mType = kDataObject;
        self.mValue.object = this;
        DataSetGlobal(Symbol("ui"), self);
    }

    DataArray* const config = SystemConfig(Symbol("ui"));
    bool verboseLocaleNotifies = false;
    config->FindData(Symbol("verbose_locale_notifies"), verboseLocaleNotifies, false);
    Locale::sVerboseNotify = verboseLocaleNotifies;
    config->FindData(Symbol("preloading_enabled"), mPreloadingEnabled, true);
    config->FindData(Symbol("reload_on_transition"), mReloadOnTransition, true);
}

// Reconstructed from eboot.elf at 0x8C86B0.
void UIMgr::EnableInputHandling(bool enable) {
    if (enable) {
        KeyboardSubscribe(&mKeyboardSink, this);
    } else if (mKeyboardSink.mSource != nullptr) {
        mKeyboardSink.mSource->RemoveSink(&mKeyboardSink);
    }
    if (mJoypadClient != nullptr) {
        mJoypadClient->SetInputEnabled(enable);
    }
}

// Reconstructed from eboot.elf at 0x8C8710. A linked subscription leaves its
// source before the link unlinks itself.
UIMgr::~UIMgr() {
    for (SceneLoader* const loader : mSceneLoaders) {
        delete loader;
    }
    mSceneLoaders.clear();
    if (mLayoutLoader != nullptr) {
        ResourceLoader::ReleaseLoader(mLayoutLoader);
    }
    delete mJoypadClient;
    mJoypadClient = nullptr;
    LinkedList::Node& link = mKeyboardSink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mKeyboardSink.mSource->RemoveSink(&mKeyboardSink);
    }
}

// Reconstructed from eboot.elf at 0x8C8880.
void UIMgr::Init() {
    theUI = new UIMgr;
}

// Reconstructed from eboot.elf at 0x8C88B0.
void UIMgr::Terminate() {
    delete mJoypadClient;
    mJoypadClient = nullptr;
    if (mKeyboardSink.mSource != nullptr) {
        mKeyboardSink.mSource->RemoveSink(&mKeyboardSink);
    }
}

// Reconstructed from eboot.elf at 0x8C8900.
void UIMgr::Poll() {
    bool transitionDone = false;
    Symbol fromName;
    Symbol toName;
    switch (mTransitionState) {
    case kTransitionEntered:
        if (IsLoaded(mCurrentLayout) && mCurrentLayout->IsEntering()) {
            break;
        }
        mTransitionState = kTransitionNone;
        if (IsLoaded(mCurrentLayout)) {
            toName = mCurrentLayout->mPath.mPath;
        }
        fromName = mPreviousLayoutName;
        mPreviousLayoutName = Symbol("");
        transitionDone = true;
        break;
    case kTransitionStarted:
        if (mReloadOnTransition && mCurrentLayout != nullptr) {
            ReleaseCurrentLayout();
        }
        mLayoutLoader->Start();
        mTransitionState = kTransitionLoading;
        [[fallthrough]];
    case kTransitionLoading: {
        if (mLayoutLoader == nullptr || !mLayoutLoader->mDone) {
            break;
        }
        if (mCurrentLayout != nullptr) {
            if (!mCurrentLayout->Fail() && mCurrentLayout->IsExiting()) {
                break;
            }
            if (!mCurrentLayout->Fail()) {
                mPreviousLayoutName = mCurrentLayout->mPath.mPath;
            }
        }
        ResourcePtr<UILayoutResource> next = resource_ptr_cast<UILayoutResource>(
            ResourcePtr<Resource>(mLayoutLoader->mResource.Get()));
        _ResetSceneDrawRegistrations(
            ResourcePtr<UILayoutResource>(mCurrentLayout.Get()),
            ResourcePtr<UILayoutResource>(next.Get()));
        ReleaseCurrentLayout();

        static Symbol destroyedCurrentLayout;
        if (destroyedCurrentLayout == Symbol()) {
            destroyedCurrentLayout = Symbol("destroyed_current_layout");
        }
        Message<0> destroyed(destroyedCurrentLayout);
        Export(destroyed.mData);

        mCurrentLayout = next.Get();
        ResourceLoader::ReleaseLoader(mLayoutLoader);
        if (IsLoaded(mCurrentLayout)) {
            mCurrentLayout->EnterEntity(mCurrentLayout->mEntity);
        }
        mTransitionState = kTransitionEntered;
        if (IsLoaded(mCurrentLayout)) {
            static Symbol transitionEnteredNewLayout;
            if (transitionEnteredNewLayout == Symbol()) {
                transitionEnteredNewLayout = Symbol("transition_entered_new_layout");
            }
            Message<2> entered(transitionEnteredNewLayout);
            entered.mData->Node(2) = DataNode(Symbol());
            entered.mData->Node(3) = DataNode(Symbol());
            mCurrentLayout->HandleMsg(entered.mData, false);
            Export(entered.mData);
        }
        break;
    }
    default:
        break;
    }

    if (IsLoaded(mCurrentLayout)) {
        if (transitionDone) {
            static Symbol transitionDoneBeforePoll;
            if (transitionDoneBeforePoll == Symbol()) {
                transitionDoneBeforePoll = Symbol("transition_done_before_poll");
            }
            {
                Message<2> before(transitionDoneBeforePoll);
                before.mData->Node(2) = DataNode(fromName);
                before.mData->Node(3) = DataNode(toName);
                mCurrentLayout->HandleMsg(before.mData, false);
                Export(before.mData);
            }
            mCurrentLayout->PollEntity(mCurrentLayout->mEntity);
            static Symbol transitionDoneName;
            if (transitionDoneName == Symbol()) {
                transitionDoneName = Symbol("transition_done");
            }
            Message<2> done(transitionDoneName);
            done.mData->Node(2) = DataNode(fromName);
            done.mData->Node(3) = DataNode(toName);
            mCurrentLayout->HandleMsg(done.mData, false);
            Export(done.mData);
        } else {
            mCurrentLayout->PollEntity(mCurrentLayout->mEntity);
        }
    }

    // Collect the scenes the loaders finished; the type check is compiled
    // out of this build and leaves its class symbols evaluated.
    for (SceneLoader** it = mSceneLoaders.begin(); it != mSceneLoaders.end();) {
        SceneLoader* const loader = *it;
        if (!loader->mDone) {
            ++it;
            continue;
        }
        ResourcePtr<EntityResource> scene(static_cast<EntityResource*>(loader->mResource.Get()));
        if (scene != nullptr) {
            if (scene->GetId() != RndSceneResource::Id()) {
                static_cast<void>(scene->GetId());
                static_cast<void>(UILayoutResource::Id());
            }
            mPreloadedScenes.push_back(scene);
        }
        delete loader;
        it = mSceneLoaders.erase(it);
    }
}

// Reconstructed from eboot.elf at 0x8C96A0.
bool UIMgr::IsInTransition() const {
    return mTransitionState != kTransitionNone;
}

// Reconstructed from eboot.elf at 0x8C96B0.
void UIMgr::ReleaseCurrentLayout() {
    if (mCurrentLayout == nullptr) {
        return;
    }
    MemBeginBatchingFreeListInsertions();
    mReleasingLayout = true;
    mCurrentLayout = nullptr;
    MemEndBatchingAndProcessFreeListInsertions();
    mReleasingLayout = false;
}

// Reconstructed from eboot.elf at 0x8C9900.
void UIMgr::_ResetSceneDrawRegistrations(
    ResourcePtr<UILayoutResource> from,
    ResourcePtr<UILayoutResource> to) {
    if (!IsLoaded(from) || !IsLoaded(to)) {
        return;
    }
    const eastl::vector<RndSceneInstanceCom*> fromScenes = from->GetSceneInstances();
    const eastl::vector<RndSceneInstanceCom*> toScenes = to->GetSceneInstances();
    const eastl::vector<RndSceneCom*> scenes = _GetSceneComs(fromScenes, toScenes);
    if (scenes.empty()) {
        return;
    }
    MemBeginBatchingFreeListInsertions();
    for (RndSceneCom* const scene : scenes) {
        scene->ResetDrawRegistrations();
    }
    MemEndBatchingAndProcessFreeListInsertions();
}

// Reconstructed from eboot.elf at 0x8C9A80.
void UIMgr::Draw() {
    RndSceneDrawParams params;
    params.mBuffers.clear();
    for (RndBufferCollection* const buffers : gRndDevice->mCurrentTargets) {
        params.mBuffers.push_back(buffers);
    }
    params.mClearColor = Hmx::Color::GetZero();
    if (!IsLoaded(mCurrentLayout)) {
        RndDevice* const device = gRndDevice;
        if (device->mBeginFramePending) {
            device->_FlushPendingBeginFrame();
        }
        const VectorAdapter<RndBufferCollection*> targets = {
            params.mBuffers.mData, params.mBuffers.mSize};
        RndDrawUtl::ClearBuffers(*device->mImmediateContext, targets, params.mClearColor);
        return;
    }
    mCurrentLayout->Draw(mCurrentLayout->mEntity, params);
}

// Reconstructed from eboot.elf at 0x8C9BE0.
void UIMgr::Goto(ResourcePtr<UILayoutResource> layout) {
    const ResourcePath path = layout->mPath;
    _StartTransition(path);
}

// Reconstructed from eboot.elf at 0x8C9C30.
void UIMgr::_StartTransition(const ResourcePath& path) {
    if (mTransitionState != kTransitionNone) {
        return;
    }
    if (IsLoaded(mCurrentLayout) && mCurrentLayout->mPath == path) {
        return;
    }
    const ResourcePath from =
        IsLoaded(mCurrentLayout) ? mCurrentLayout->mPath : ResourcePath("");

    static Symbol transitionStarted;
    if (transitionStarted == Symbol()) {
        transitionStarted = Symbol("transition_started");
    }
    Message<2> started(transitionStarted);
    started.mData->Node(2) = ResourcePathNode(from);
    started.mData->Node(3) = ResourcePathNode(path);
    Export(started.mData);

    mTransitionState = kTransitionStarted;
    const bool start = false;
    mLayoutLoader = ResourceLoader::AcquireLoader(path, start);
    if (IsLoaded(mCurrentLayout)) {
        mCurrentLayout->Exit();
    }
}

// Reconstructed from eboot.elf at 0x8C9F50.
void UIMgr::Goto(const ResourcePath& path) {
    _StartTransition(path);
}

// Reconstructed from eboot.elf at 0x8C9F60.
eastl::vector<RndSceneCom*> UIMgr::_GetSceneComs(
    const eastl::vector<RndSceneInstanceCom*>& first,
    const eastl::vector<RndSceneInstanceCom*>& second) const {
    eastl::vector<RndSceneCom*> scenes;
    scenes.reserve(first.size() + second.size());
    for (const RndSceneInstanceCom* const instance : first) {
        const Entity* const entity = instance->mRuntime.mEntity;
        if (entity == nullptr) {
            continue;
        }
        RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
        if (scene != nullptr) {
            scenes.push_back(scene);
        }
    }
    for (const RndSceneInstanceCom* const instance : second) {
        const Entity* const entity = instance->mRuntime.mEntity;
        if (entity == nullptr) {
            continue;
        }
        RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
        if (scene != nullptr) {
            scenes.push_back(scene);
        }
    }
    return scenes;
}

// Reconstructed from eboot.elf at 0x8CA310.
bool UIMgr::IsTransitionReady() const {
    if (mTransitionState == kTransitionNone || mLayoutLoader == nullptr) {
        return false;
    }
    return mLayoutLoader->mDone;
}

// Reconstructed from eboot.elf at 0x8CA330.
void UIMgr::AddScene(const ResourcePath& path, unsigned int flags) {
    if (IsLoaded(mCurrentLayout) && !mReleasingLayout) {
        mCurrentLayout->AddSceneToFront(path, flags);
    }
}

// Reconstructed from eboot.elf at 0x8CA3A0.
void UIMgr::AddSceneAtDrawOrder(const ResourcePath& path, GameObjectId before, unsigned int flags) {
    if (IsLoaded(mCurrentLayout) && !mReleasingLayout) {
        mCurrentLayout->AddSceneAtDrawOrder(path, before, flags);
    }
}

// Reconstructed from eboot.elf at 0x8CA400.
void UIMgr::AddSceneAfterObject(const ResourcePath& path, GameObjectId after, unsigned int flags) {
    if (IsLoaded(mCurrentLayout) && !mReleasingLayout) {
        mCurrentLayout->AddSceneAfterObject(path, after, flags);
    }
}

// Reconstructed from eboot.elf at 0x8CA460.
void UIMgr::RemoveScene(const ResourcePath& path, unsigned int flags) {
    if (IsLoaded(mCurrentLayout) && !mReleasingLayout) {
        mCurrentLayout->RemoveScene(path, flags);
    }
}

// Reconstructed from eboot.elf at 0x8CA4C0.
UILayoutResource* UIMgr::GetCurrentLayout() const {
    if (mReleasingLayout) {
        return nullptr;
    }
    return mCurrentLayout;
}

// Reconstructed from eboot.elf at 0x8CA4E0.
void UIMgr::SetSceneShowing(const ResourcePath& path, bool showing) {
    if (IsLoaded(mCurrentLayout)) {
        mCurrentLayout->SetSceneShowing(path, showing);
    }
}

// Reconstructed from eboot.elf at 0x8CA530. The binary also reads the
// thread's poll context and discards it. The layout check before the
// search is compiled out but its Fail call remains, and the type check of
// the preloaded layout leaves its class symbols evaluated.
void UIMgr::SwapToPreloadedLayout(const ResourcePath& path, bool releaseCurrent) {
    if (mCurrentLayout != nullptr) {
        static_cast<void>(mCurrentLayout->Fail());
    }
    if (releaseCurrent) {
        ReleaseCurrentLayout();
    } else {
        mCurrentLayout->SetScenesActive(false);
        const ResourcePtr<EntityResource> current(mCurrentLayout.Get());
        mPreloadedScenes.push_back(current);
    }
    for (ResourcePtr<EntityResource>* it = mPreloadedScenes.begin(); it != mPreloadedScenes.end();
         ++it) {
        if ((*it)->mPath == path) {
            static_cast<void>((*it)->GetId());
            static_cast<void>(UILayoutResource::Id());
            mCurrentLayout = static_cast<UILayoutResource*>(it->Get());
            mPreloadedScenes.erase(it);
            break;
        }
    }
    if (mCurrentLayout != nullptr) {
        static_cast<void>(mCurrentLayout->Fail());
    }
    mCurrentLayout->SetScenesActive(true);
}

// Reconstructed from eboot.elf at 0x8CA800.
bool UIMgr::IsPreloadedScene(const ResourcePath& path, bool includeLoading) const {
    if (includeLoading) {
        for (const SceneLoader* const loader : mSceneLoaders) {
            if (path == loader->mPath) {
                return true;
            }
        }
    }
    for (const ResourcePtr<EntityResource>& scene : mPreloadedScenes) {
        if (path == scene->mPath) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8CA870. As in SwapToPreloadedLayout,
// the thread's poll context is read and discarded, and the checks around
// the layout are compiled out.
void UIMgr::GotoPreloadedLayout(const ResourcePath& path) {
    if (mCurrentLayout != nullptr) {
        static_cast<void>(mCurrentLayout->Fail());
    }
    const ResourcePath from = mCurrentLayout->mPath;
    ReleaseCurrentLayout();
    for (ResourcePtr<EntityResource>* it = mPreloadedScenes.begin(); it != mPreloadedScenes.end();
         ++it) {
        if ((*it)->mPath == path) {
            static_cast<void>((*it)->GetId());
            static_cast<void>(UILayoutResource::Id());
            mCurrentLayout = static_cast<UILayoutResource*>(it->Get());
            mPreloadedScenes.erase(it);
            break;
        }
    }
    if (mCurrentLayout != nullptr) {
        static_cast<void>(mCurrentLayout->Fail());
    }
    mCurrentLayout->EnterEntity(mCurrentLayout->mEntity);

    static Symbol transitionDoneBeforePoll;
    if (transitionDoneBeforePoll == Symbol()) {
        transitionDoneBeforePoll = Symbol("transition_done_before_poll");
    }
    {
        Message<2> before(transitionDoneBeforePoll);
        before.mData->Node(2) = ResourcePathNode(from);
        before.mData->Node(3) = ResourcePathNode(mCurrentLayout->mPath);
        mCurrentLayout->HandleMsg(before.mData, false);
        Export(before.mData);
    }
    mCurrentLayout->PollEntity(mCurrentLayout->mEntity);
    static Symbol transitionDone;
    if (transitionDone == Symbol()) {
        transitionDone = Symbol("transition_done");
    }
    Message<2> done(transitionDone);
    done.mData->Node(2) = ResourcePathNode(from);
    done.mData->Node(3) = ResourcePathNode(mCurrentLayout->mPath);
    mCurrentLayout->HandleMsg(done.mData, false);
    Export(done.mData);
}

// Reconstructed from eboot.elf at 0x8CAF20.
SceneLoader::SceneLoader(const ResourcePath& path, bool copyOnInstance, bool enterOnLoad)
    : ResourceLoader(path), mCopyOnInstance(copyOnInstance), mEnterOnLoad(enterOnLoad) {}

// Reconstructed from eboot.elf at 0x8CAFE0. Sets the root's
// EntityHeaderCom "copy_on_instance" property. The failure report is
// compiled out, but its message is still formatted. Name not in the
// reference map.
static void SetCopyOnInstance(Entity* entity) {
    Component* const header = entity->GetRoot()->GetCom<EntityHeaderCom>();
    PropPath path;
    PropPath::Node node;
    node = Symbol("copy_on_instance");
    path.PushNode(node);
    const bool value = true;
    void* storage = header;
    const PropInfo* info = header->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr ||
        !_SetProp(*header, path, *info, storage, &value, kPropertyBool, false)) {
        static_cast<void>(header->MakeErrorName());
        char buffer[512] = {};
        path.ToString(buffer, sizeof(buffer));
    }
}

// Reconstructed from eboot.elf at 0x8CAF50.
void SceneLoader::ThreadDone(int result) {
    static_cast<void>(result);
    mDone = true;
    if (mResource == nullptr) {
        return;
    }
    ResourcePtr<EntityResource> scene(static_cast<EntityResource*>(mResource.Get()));
    if (scene->Fail()) {
        return;
    }
    if (mCopyOnInstance) {
        SetCopyOnInstance(scene->mEntity);
    }
    if (mEnterOnLoad) {
        scene->EnterEntity(scene->mEntity);
    }
}

// Reconstructed from eboot.elf at 0x8CB2D0. The type check of a scene
// loaded at once is compiled out and leaves its class symbols evaluated.
void UIMgr::PreloadScene(const ResourcePath& path, const PreloadArgs& args) {
    if (!mPreloadingEnabled) {
        return;
    }
    for (const SceneLoader* const loader : mSceneLoaders) {
        if (path == loader->mPath) {
            return;
        }
    }
    for (const ResourcePtr<EntityResource>& scene : mPreloadedScenes) {
        if (path == scene->mPath) {
            return;
        }
    }
    if (args.mLoadNow) {
        ResourcePtr<EntityResource> scene = Resource::GetOrLoad<EntityResource>(path, false);
        if (scene == nullptr || scene->Fail()) {
            return;
        }
        if (args.mCopyOnInstance) {
            SetCopyOnInstance(scene->mEntity);
        }
        if (scene->GetId() != RndSceneResource::Id()) {
            static_cast<void>(scene->GetId());
            static_cast<void>(UILayoutResource::Id());
        }
        mPreloadedScenes.push_back(scene);
        return;
    }
    unsigned int temp;
    MemPushTemp(temp, true, true);
    SceneLoader* const loader = new SceneLoader(path, args.mCopyOnInstance, args.mEnterOnLoad);
    MemPopTemp(temp);
    mSceneLoaders.push_back(loader);
    loader->Start();
}

// Reconstructed from eboot.elf at 0x8CB620. The scene is released while
// the allocator batches the frees.
bool UIMgr::UnloadPreloadedScene(const ResourcePath& path) {
    bool unloaded = false;
    for (ResourcePtr<EntityResource>* it = mPreloadedScenes.begin(); it != mPreloadedScenes.end();) {
        ResourcePtr<EntityResource> scene(it->Get());
        if (scene != nullptr) {
            static_cast<void>(scene->Fail());
        }
        if (path == scene->mPath) {
            it = mPreloadedScenes.erase(it);
            MemBeginBatchingFreeListInsertions();
            scene = nullptr;
            MemEndBatchingAndProcessFreeListInsertions();
            unloaded = true;
        } else {
            ++it;
        }
    }
    return unloaded;
}

// Reconstructed from eboot.elf at 0x8CB750.
ResourcePtr<EntityResource> UIMgr::GetPreloadedScene(const ResourcePath& path) const {
    for (const ResourcePtr<EntityResource>& scene : mPreloadedScenes) {
        if (path == scene->mPath) {
            return ResourcePtr<EntityResource>(scene.Get());
        }
    }
    return ResourcePtr<EntityResource>();
}

// Reconstructed from eboot.elf at 0x8CB7B0.
DataNode UIMgr::_OnInputMsg(DataArray* msg) {
    if (IsLoaded(mCurrentLayout) && mTransitionState == kTransitionNone &&
        mCurrentLayout->HandleMsg(msg, true)) {
        return DataNode(0);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x8CB820.
DataNode UIMgr::_OnConnectMsg(DataArray* msg) {
    if (IsLoaded(mCurrentLayout)) {
        mCurrentLayout->HandleMsg(msg, false);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8CB870.
DataNode UIMgr::_OnGoto(const DataArray* msg) {
    if (msg->Size() != 3) {
        return DataNode();
    }
    _StartTransition(ResourcePath(msg->Str(2)));
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8CB910.
DataNode UIMgr::_OnAddScene(const DataArray* msg) {
    if (msg->Size() != 3) {
        return DataNode();
    }
    const ResourcePath path(msg->Str(2));
    AddScene(path, 0);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8CB9D0.
DataNode UIMgr::_OnRemoveScene(const DataArray* msg) {
    if (msg->Size() != 3) {
        return DataNode();
    }
    const ResourcePath path(msg->Str(2));
    RemoveScene(path, 0);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8CBA90.
DataNode UIMgr::_OnSetReloadOnTransition(const DataArray* msg) {
    if (msg->Size() != 3) {
        return DataNode();
    }
    mReloadOnTransition = msg->Int(2) != 0;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8CBAE0. A handler that leaves the
// message unhandled lets the later types be tried.
DataNode UIMgr::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);

    static Symbol key;
    if (key == Symbol()) {
        key = Symbol("key");
    }
    if (type == key) {
        DataNode result = _OnInputMsg(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol keyRelease;
    if (keyRelease == Symbol()) {
        keyRelease = Symbol("key_release");
    }
    if (type == keyRelease) {
        DataNode result = _OnInputMsg(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol buttonDown;
    if (buttonDown == Symbol()) {
        buttonDown = Symbol("button_down");
    }
    if (type == buttonDown) {
        DataNode result = _OnInputMsg(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol buttonUp;
    if (buttonUp == Symbol()) {
        buttonUp = Symbol("button_up");
    }
    if (type == buttonUp) {
        DataNode result = _OnInputMsg(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol joypadConnect;
    if (joypadConnect == Symbol()) {
        joypadConnect = Symbol("joypad_connect");
    }
    if (type == joypadConnect) {
        return _OnConnectMsg(msg);
    }
    static Symbol gotoName;
    if (gotoName == Symbol()) {
        gotoName = Symbol("goto");
    }
    if (type == gotoName) {
        DataNode result = _OnGoto(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol addScene;
    if (addScene == Symbol()) {
        addScene = Symbol("add_scene");
    }
    if (type == addScene) {
        DataNode result = _OnAddScene(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol removeScene;
    if (removeScene == Symbol()) {
        removeScene = Symbol("remove_scene");
    }
    if (type == removeScene) {
        DataNode result = _OnRemoveScene(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol setReloadOnTransition;
    if (setReloadOnTransition == Symbol()) {
        setReloadOnTransition = Symbol("set_reload_on_transition");
    }
    if (type == setReloadOnTransition) {
        DataNode result = _OnSetReloadOnTransition(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol isInTransition;
    if (isInTransition == Symbol()) {
        isInTransition = Symbol("is_in_transition");
    }
    if (type == isInTransition) {
        return DataNode(static_cast<int>(IsInTransition()));
    }
    return DataNode();
}
