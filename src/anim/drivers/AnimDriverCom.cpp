// anim/AnimDriverCom.o (0x3297F0 to 0x32C6B6). The class's registration
// and factory are emitted in anim/AnimInit.o. The binary emits the map's
// node insertion and the allowed-value vector insertion as templates here
// (0x32BD10, 0x32BE20, 0x32BF20); the containers model them.
#include "anim/drivers/AnimDriverCom.h"

#include <new>

#include "anim/core/AnimOptions.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/char/CharAnimResource.h"
#include "anim/char/CharClipResource.h"
#include "anim/props/PropAnimResource.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransCom.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropArray.h"
#include "utl/text/MakeString.h"

// The object's statics, in the order of its static initializer (0x32C5E0),
// which first sets a shared header's null object id to -1 (0x19FB688).
Symbol AnimDriverCom::sId("AnimDriver");
Symbol AnimDriverCom::sClassName("AnimDriver");
PropRegistry AnimDriverCom::sPropRegistry;
ComMetaData AnimDriverCom::sMetaData;
std::function<TimeUnits(Entity*, const Resource*)> AnimDriverCom::sProjectTimelineCallback;

// Reconstructed from eboot.elf at 0x329820.
AnimDriverCom::AnimDriverCom() : mDefaultTimeline(kTimeUnitsSeconds), mRuntime() {}

// Reconstructed from eboot.elf at 0x329900. The event parameters are
// registered with the non-static tag functions disallowed.
void AnimDriverCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    AnimDriverCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    IntMetadata<int>& timeline = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("default_timeline", offsetof(AnimDriverCom, mDefaultTimeline), kPropertyInt, 4)
             .mMetadata.Get());
    timeline.mDefault = static_cast<int>(prototype.mDefaultTimeline);
    static_cast<void>(Symbol("prop"));
    timeline.mHelp = String(
        "The default timeline for time-based functionality in this entity/scene. Animations, state graphs, and "
        "(potentially) other time based systems can be configured to override this value, but by default will "
        "operate on this timeline. This is a useful way to (for example) make animations run on UI seconds "
        "automatically in a UI screen, without having to remember to manually set each one to do so.");
    timeline.mAnimatable = false;
    timeline.mAllowedValues = ::GetTimeUnitsAllowedValues();

    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);
    PropRegistry& animEvent =
        metadata.AddExportedEvent(Symbol("anim_event"), String("An event within an animation has been reached."));
    const Symbol eventType;
    SymbolMetadata& event = TypeSpecificMetadata(animEvent.RegisterProp("event", -1, kPropertySymbol, 0), eventType);
    static_cast<void>(Symbol("param"));
    event.mHelp = String("The name of the event as authored within the animation.");
    const float frameType = 0.0f;
    FloatMetadata& frame = TypeSpecificMetadata(animEvent.RegisterProp("frame", -1, kPropertyFloat, 0), frameType);
    static_cast<void>(Symbol("param"));
    frame.mHelp = String("The frame at which the event occurs within the animation.");
    const float weightType = 0.0f;
    FloatMetadata& weight = TypeSpecificMetadata(animEvent.RegisterProp("weight", -1, kPropertyFloat, 0), weightType);
    static_cast<void>(Symbol("param"));
    weight.mHelp = String("The weight of the animation playback instance that sent this event.");
    const ResourcePath animType;
    ResourcePathMetadata& anim =
        TypeSpecificMetadata(animEvent.RegisterProp("anim", -1, kPropertyResourcePath, 0), animType);
    static_cast<void>(Symbol("param"));
    anim.mHelp = String("The resource path of the animation that sent this event.");
    const GameObjectId objectType = {0xFFFFFFFFU};
    GameObjectIdMetadata& object =
        TypeSpecificMetadata(animEvent.RegisterProp("object", -1, kPropertyGameObjectId, 0), objectType);
    static_cast<void>(Symbol("param"));
    object.mHelp = String(
        "If the animation that sent this event is a CharClipResource, this is the object that owns the "
        "corresponding CharClipCom. Otherwise, this parameter will be kNullId (i.e. <None>).");
    SetNonStaticTagFunctionsAllowed(nonStatic);

    metadata.mDescription = "Root Object Com exposes interface to play animations in its entity";
    metadata.mAuthor = "James Fleming";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());
    metadata.mLightweight = true;
    metadata.mAliases.push_back(Symbol("AnimDriverCom"));
}

// Reconstructed from eboot.elf at 0x32A050.
void AnimDriverCom::_GetComponentOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    static_cast<void>(follows);
    precedes.push_back(TransCom::sClassName);
}

// Reconstructed from eboot.elf at 0x32A110.
bool AnimDriverCom::_OnResourcesLoaded() {
    mRuntime.mClock = TheTimeMgr->GetClock(mObject->mEntity);
    mRuntime.mPropAnimDriver.Enter(mObject->mEntity, mRuntime.mClock);
    mRuntime.mCharAnimDriver.Enter(mObject->mEntity, mRuntime.mClock);
    return true;
}

// Reconstructed from eboot.elf at 0x32A170.
void AnimDriverCom::_Poll() {
    mRuntime.mPropAnimDriver.Poll(mObject);
    mRuntime.mCharAnimDriver.Poll(mObject);
}

// Reconstructed from eboot.elf at 0x32A1A0.
void AnimDriverCom::_EditPoll() {
    mRuntime.mPropAnimDriver.Poll(mObject);
    mRuntime.mCharAnimDriver.Poll(mObject);
}

// Reconstructed from eboot.elf at 0x32A1D0.
void AnimDriverCom::_Exit(DestroyType type) {
    if (type == kDestroyRoot) {
        return;
    }
    mRuntime.mPropAnimDriver.Clear();
    mRuntime.mCharAnimDriver.Clear();
    mRuntime.mNamedAnims.clear();
}

// Reconstructed from eboot.elf at 0x32A240.
AnimInstID AnimDriverCom::StartAnim(const ResourcePath& path, const AnimOptions& options) {
    const ResourcePtr<Resource> resource = _FindResource(path);
    return StartAnim(resource.Get(), options);
}

// Reconstructed from eboot.elf at 0x32A2C0.
ResourcePtr<Resource> AnimDriverCom::_FindResource(const ResourcePath& path) {
    const ResourcePtr<Resource> inlined = mObject->mEntity->mResource->TryGetInline<Resource>(path);
    if (inlined != nullptr) {
        return ResourcePtr<Resource>(inlined.Get());
    }
    return Resource::GetOrLoad<Resource>(path, false);
}

// Reconstructed from eboot.elf at 0x32A370. The replaced playback's time
// or frame is read from the new driver, as the binary does.
AnimInstID AnimDriverCom::StartAnim(Resource* resource, const AnimOptions& options) {
    const ResourcePtr<Resource> held(resource);
    if (resource == nullptr || resource->Fail()) {
        return AnimInstID::NullID();
    }
    AnimDriver* const driver = _FindDriver(resource->GetId());
    if (driver == nullptr) {
        return AnimInstID::NullID();
    }
    const Symbol name = options.mName;
    AnimInstID replaced = name == Symbol() ? options.mReplaceID : FindInstId(name);
    AnimOptions start = options;
    if (replaced.mDriver != nullptr && replaced.mDriver->IsAnimating(replaced, true)) {
        if (start.mStartFromCurrentFrame) {
            start.mStartTime = start.mUseFrameTime ? driver->GetAnimFrame(replaced) : driver->GetAnimTime(replaced);
        }
        if (replaced.mDriver != nullptr) {
            replaced.mDriver->StopAnim(replaced);
        }
    }
    if (sProjectTimelineCallback) {
        const TimeUnits units = sProjectTimelineCallback(mObject->mEntity, resource);
        if (units != kNumTimeUnits && units != start.mTimeline) {
            start.mTimeline = units;
        }
    }
    const AnimInstID id = driver->StartAnim(resource, start);
    if (name != Symbol()) {
        mRuntime.mNamedAnims[name] = id;
    }
    return id;
}

// Reconstructed from eboot.elf at 0x32A670.
AnimInstID AnimDriverCom::FindInstId(Symbol name) const {
    if (name == Symbol()) {
        return AnimInstID::NullID();
    }
    auto& named = const_cast<AnimDriverCom*>(this)->mRuntime.mNamedAnims;
    const auto found = named.find(name);
    return found != named.end() ? found->second : AnimInstID::NullID();
}

// Reconstructed from eboot.elf at 0x32A710.
bool AnimDriverCom::IsAnimating(const AnimInstID& id, bool includePaused) const {
    return id.mDriver != nullptr && id.mDriver->IsAnimating(id, includePaused);
}

// Reconstructed from eboot.elf at 0x32A730.
bool AnimDriverCom::StopAnim(AnimInstID& id) {
    return id.mDriver != nullptr && id.mDriver->StopAnim(id);
}

// Reconstructed from eboot.elf at 0x32A740.
bool AnimDriverCom::IsAnimating(const ResourcePath& path, bool includePaused) {
    const AnimDriver* const driver = _FindDriver(ResourceMetaData::GetIdFromPath(path));
    return driver != nullptr && driver->IsAnimating(path, includePaused);
}

// Reconstructed from eboot.elf at 0x32A7D0.
AnimDriver* AnimDriverCom::_FindDriver(const Symbol& resourceId) {
    if (resourceId == PropAnimResource::Id()) {
        return &mRuntime.mPropAnimDriver;
    }
    if (resourceId == CharAnimResource::Id()) {
        return &mRuntime.mCharAnimDriver;
    }
    if (resourceId == CharClipResource::Id()) {
        return &mRuntime.mCharAnimDriver;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x32A970.
void AnimDriverCom::PauseAnim(const AnimInstID& id, bool pause) {
    if (id.mDriver != nullptr) {
        id.mDriver->PauseAnim(id, pause);
    }
}

// Reconstructed from eboot.elf at 0x32A990.
void AnimDriverCom::SetAnimTime(const AnimInstID& id, float time) {
    if (id.mDriver != nullptr) {
        id.mDriver->SetAnimTime(id, time);
    }
}

// Reconstructed from eboot.elf at 0x32A9A0.
float AnimDriverCom::GetAnimTime(const AnimInstID& id) {
    return id.mDriver != nullptr ? id.mDriver->GetAnimTime(id) : 0.0f;
}

// Reconstructed from eboot.elf at 0x32A9C0.
void AnimDriverCom::SetAnimFrame(const AnimInstID& id, float frame) {
    if (id.mDriver != nullptr) {
        id.mDriver->SetAnimFrame(id, frame);
    }
}

// Reconstructed from eboot.elf at 0x32A9D0.
float AnimDriverCom::GetAnimFrame(const AnimInstID& id) {
    return id.mDriver != nullptr ? id.mDriver->GetAnimFrame(id) : 0.0f;
}

// Reconstructed from eboot.elf at 0x32A9F0.
bool AnimDriverCom::IsFinishedPlaying(const AnimInstID& id) const {
    return id.mDriver != nullptr && id.mDriver->IsFinishedPlaying(id);
}

// Reconstructed from eboot.elf at 0x32AA00.
PlaybackInfo* AnimDriverCom::GetPlaybackInfo(const AnimInstID& id) {
    return id.mDriver != nullptr ? id.mDriver->GetPlaybackInfo(id) : nullptr;
}

// Reconstructed from eboot.elf at 0x32AA10.
unsigned long AnimDriverCom::GetActiveAnimsCount() const {
    return mRuntime.mPropAnimDriver.mPlaybacks.size() + mRuntime.mCharAnimDriver.mPlaybacks.size();
}

// Reconstructed from eboot.elf at 0x32AA20.
void AnimDriverCom::Clear() {
    mRuntime.mPropAnimDriver.Clear();
    mRuntime.mCharAnimDriver.Clear();
}

// Reconstructed from eboot.elf at 0x32AA50. The binary appends the
// timelines with one range insertion (0x32BF20).
eastl::vector<AllowedValue<int>> AnimDriverCom::GetTimeUnitsAllowedValues() const {
    const eastl::vector<AllowedValue<int>>& timelines = ::GetTimeUnitsAllowedValues();
    eastl::vector<AllowedValue<int>> values;
    values.reserve(timelines.size() + 1);
    const char* const name = (FormatString("Default (%s)") << GetTimeUnitsName(mDefaultTimeline, true)).Str();
    values.push_back(
        AllowedValue<int>{kNumTimeUnits, String(name), String("The default timeline for this entity/scene.")});
    for (const AllowedValue<int>& timeline : timelines) {
        values.push_back(timeline);
    }
    return values;
}

// Reconstructed from eboot.elf at 0x32ABE0. The keys before the start
// fill the options; "auto_destruct", "weight" and "loop" then set the new
// playback.
DataNode AnimDriverCom::_OnStartAnim(const DataArray* msg) {
    ResourcePath path;
    path = msg->Sym(2).Str();
    static Symbol timeline;
    if (timeline == Symbol()) {
        timeline = Symbol("timeline");
    }
    static Symbol timeRangeBegin;
    if (timeRangeBegin == Symbol()) {
        timeRangeBegin = Symbol("time_range_begin");
    }
    static Symbol timeRangeEnd;
    if (timeRangeEnd == Symbol()) {
        timeRangeEnd = Symbol("time_range_end");
    }
    static Symbol startTime;
    if (startTime == Symbol()) {
        startTime = Symbol("start_time");
    }
    static Symbol useFrameTime;
    if (useFrameTime == Symbol()) {
        useFrameTime = Symbol("use_frame_time");
    }
    static Symbol period;
    if (period == Symbol()) {
        period = Symbol("period");
    }
    static Symbol weight;
    if (weight == Symbol()) {
        weight = Symbol("weight");
    }
    static Symbol scale;
    if (scale == Symbol()) {
        scale = Symbol("scale");
    }
    static Symbol loop;
    if (loop == Symbol()) {
        loop = Symbol("loop");
    }
    static Symbol startFromCurrentFrame;
    if (startFromCurrentFrame == Symbol()) {
        startFromCurrentFrame = Symbol("start_from_current_frame");
    }
    static Symbol startPaused;
    if (startPaused == Symbol()) {
        startPaused = Symbol("start_paused");
    }
    static Symbol autoDestruct;
    if (autoDestruct == Symbol()) {
        autoDestruct = Symbol("auto_destruct");
    }
    static Symbol name;
    if (name == Symbol()) {
        name = Symbol("name");
    }

    AnimOptions options;
    for (int i = 3; i < msg->Size(); ++i) {
        if (msg->Node(i).Type() != kDataArray) {
            continue;
        }
        const DataArray* const entry = msg->Node(i).Array(msg);
        const Symbol key = entry->Sym(0);
        if (key == timeline) {
            options.mTimeline = static_cast<TimeUnits>(entry->Int(1));
        } else if (key == timeRangeBegin) {
            options.mTimeRangeBegin = entry->Float(1);
        } else if (key == timeRangeEnd) {
            options.mTimeRangeEnd = entry->Float(1);
        } else if (key == startTime) {
            options.mStartTime = entry->Float(1);
        } else if (key == useFrameTime) {
            options.mUseFrameTime = entry->Int(1) != 0;
        } else if (key == period) {
            options.mPeriod = entry->Float(1);
        } else if (key == scale) {
            options.mScale = entry->Float(1);
        } else if (key == startFromCurrentFrame) {
            options.mStartFromCurrentFrame = entry->Int(1) != 0;
        } else if (key == startPaused) {
            options.mStartPaused = entry->Int(1) != 0;
        } else if (key == name) {
            options.mName = entry->Sym(1);
        }
    }

    AnimInstID id;
    {
        const ResourcePtr<Resource> resource = _FindResource(path);
        id = StartAnim(resource.Get(), options);
    }
    if (id.mDriver != nullptr) {
        PlaybackInfo* const info = id.mDriver->GetPlaybackInfo(id);
        if (info != nullptr) {
            for (int i = 3; i < msg->Size(); ++i) {
                if (msg->Node(i).Type() != kDataArray) {
                    continue;
                }
                const DataArray* const entry = msg->Node(i).Array(msg);
                const Symbol key = entry->Sym(0);
                if (key == autoDestruct) {
                    info->mAutoDestruct = entry->Int(1) != 0;
                } else if (key == weight) {
                    info->mWeight = entry->Float(1);
                } else if (key == loop) {
                    info->mLooping = entry->Int(1) != 0;
                }
            }
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x32B460. Handle inlines it.
DataNode AnimDriverCom::_OnStopAnim(const DataArray* msg) {
    AnimInstID id = FindInstId(msg->Sym(2));
    const bool stopped = id.mDriver != nullptr && id.mDriver->StopAnim(id);
    return DataNode(static_cast<int>(stopped));
}

// Reconstructed from eboot.elf at 0x32B570.
DataNode AnimDriverCom::_OnApplyPropanimFrame(const DataArray* msg) {
    ResourcePath path;
    path = msg->Str(2);
    ResourcePtr<PropAnimResource> anim;
    {
        const ResourcePtr<Resource> found = _FindResource(path);
        anim = resource_ptr_cast<PropAnimResource>(ResourcePtr<Resource>(found.Get()));
    }
    const float frame = msg->Float(3);
    if (anim == nullptr || anim->Fail()) {
        return DataNode(0);
    }
    anim->ApplyFrame(frame, mObject->mEntity);
    return DataNode(1);
}

// Reconstructed from eboot.elf at 0x32B790. Component's handler runs
// without its warning.
DataNode AnimDriverCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol startAnim;
    if (startAnim == Symbol()) {
        startAnim = Symbol("start_anim");
    }
    if (type == startAnim) {
        DataNode result = _OnStartAnim(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol stopAnim;
    if (stopAnim == Symbol()) {
        stopAnim = Symbol("stop_anim");
    }
    if (type == stopAnim) {
        return _OnStopAnim(msg);
    }
    static Symbol applyPropanimFrame;
    if (applyPropanimFrame == Symbol()) {
        applyPropanimFrame = Symbol("apply_propanim_frame");
    }
    if (type == applyPropanimFrame) {
        DataNode result = _OnApplyPropanimFrame(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    DataNode result = Component::Handle(msg, false);
    if (result.Type() != kDataUnhandled) {
        return result;
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x32BAC0.
Symbol AnimDriverCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x32BAD0.
Symbol AnimDriverCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x32BAE0.
int AnimDriverCom::CurrentRev() const {
    return const_cast<AnimDriverCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x32BB00.
bool AnimDriverCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x32BB30.
Component* AnimDriverCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x32BB40. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* AnimDriverCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<AnimDriverCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) AnimDriverCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x32BCF0.
PropRegistry& AnimDriverCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x32BD00.
ComMetaData& AnimDriverCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2B1D20. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* AnimDriverCom::_Create() {
    return new AnimDriverCom();
}

// Reconstructed from eboot.elf at 0x2A76E0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void AnimDriverCom::Init() {
    InitComponentClass<AnimDriverCom>();
}
