// anim/CharClipCom.o (0x2E5C90 to 0x2EFF7F). The class's registration and
// factory are emitted in anim/AnimInit.o. The binary emits the element
// operations of PropArray<CharClipConfig::BeatTrackEntry> (vtable 0x18F5BA0,
// 0x2EB030-0x2EB360) and the std::function wrappers of the registry's
// lambdas (vtables 0x18F5BF0 to 0x18F6780, 0x2EB370-0x2EFEBF) here; the
// templates and lambdas model them. The object's static initializer is at
// 0x2EFEC0.
#include "anim/char/CharClipCom.h"

#include <functional>
#include <new>

#include "anim/char/CharAnim.h"
#include "anim/core/AnimOptions.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "anim/skeleton/CharSkeleton.h"
#include "anim/skeleton/CharSkeletonPose.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "render/scene/CharResource.h"
#include "render/skeletons/RndSkeletonPoseCom.h"
#include "utl/containers/Sort.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The -1 of a shared header that the static initializer sets (0x19F8A98).
// Nothing in this object reads it. Name not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The component a tag or an accessor belongs to. Names not in the
// reference map.
CharClipCom& Clip(const TagArgs& args) {
    return const_cast<CharClipCom&>(static_cast<const CharClipCom&>(args.mComponent));
}
CharClipCom& Clip(const PropAccessorArgs& args) {
    return *static_cast<CharClipCom*>(args.mComponent);
}

// The index of the array element the tag's path ends in. Name not in the
// reference map.
unsigned long ElementIndex(const TagArgs& args) {
    return args.mPath[-1].mValue;
}

// The clip's animation interface, or null without a resource. Names not in
// the reference map.
AnimResource* EventAnim(const TagArgs& args) {
    return Clip(args).mResource.Get();
}
AnimResource* EventAnim(const PropAccessorArgs& args) {
    return Clip(args).mResource.Get();
}

// Reads the property at the path into `value`; a missing or unconvertible
// property is reported (the message is compiled out) and leaves the value.
// Inlined into the event views. Name not in the reference map.
template <class T>
void ReadProp(const Component& component, const PropPath& path, T& value, PropertyType type) {
    Component& owner = const_cast<Component&>(component);
    void* storage = &owner;
    const PropInfo* const info = owner._GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _GetProp(component, path, *info, storage, &value, type)) {
        return;
    }
    static_cast<void>(component.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

// Sets the component's bool property, reporting a failure as ReadProp
// does. Inlined into the "preview" callback and _PreDestroy. Name not in
// the reference map.
void SetBoolProp(Component* component, Symbol name, bool value) {
    PropPath path;
    PropPath::Node node;
    path.PushNode(node = name);
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _SetProp(*component, path, *info, storage, &value, kPropertyBool, false)) {
        return;
    }
    static_cast<void>(component->MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

// The path of the member `name` under the path. Name not in the reference
// map.
PropPath MemberPath(const PropPath& path, Symbol name) {
    PropPath member = path;
    PropPath::Node node;
    member.PushNode(node = name);
    return member;
}

// The fraction of the playback range an event frame lies at (`empty` when
// the range is empty), and the time on the clip's timeline of a fraction of
// the range. Inlined into the event views. Names not in the reference map.
float RangeFraction(const AnimResource* anim, float eventFrame, float empty) {
    const float frame = anim->EventFrameToFrame(eventFrame);
    const float start = anim->GetStartFrame();
    const float end = anim->GetEndFrame();
    if (end == start) {
        return empty;
    }
    return (frame - start) / (end - start);
}
float FractionToTime(const AnimResource* anim, float fraction) {
    const float start = anim->GetTimeFromFrame(anim->GetStartFrame(), kNumTimeUnits);
    const float end = anim->GetTimeFromFrame(anim->GetEndFrame(), kNumTimeUnits);
    return start + (end - start) * fraction;
}

// The units label of the event views: "beats" for a clip on the beat
// timeline, else "seconds". Name not in the reference map.
const char* EventTimeUnits(const AnimResource* anim) {
    if (anim == nullptr) {
        return "seconds";
    }
    return anim->GetUnits() == kTimeUnitsBeats ? "beats" : "seconds";
}

// Reconstructed from eboot.elf at 0x2E8F10. Registers the events array:
// each element shows its name and a summary, inserts between its
// neighbours, and has a "frame_editor_view" that edits its frame as a
// frame, a time and a percentage of the clip, bounded by the clip's
// authored range. anim/CharAnimCom.o has its own copy (0x2D8EE0), which
// reads the animation of a CharAnimCom. Name not in the reference map.
void RegisterAnimEvents(PropRegistry& registry, Symbol name, int offset) {
    PropInfo& array = registry.RegisterPropArray(name.Str(), offset, sizeof(PropArray<AnimResource::Event>));
    PropInfo& item = PropRegistry::RegisterPropArrayItem(array, kPropertyStruct, sizeof(AnimResource::Event));
    ArrayMetadata& arrayMetadata = static_cast<ArrayMetadata&>(*array.mMetadata.Get());
    arrayMetadata.mHelp = String(
        "A set of events within this animation resource. They will be exported on the owning entity as they "
        "are passed over during playback. They can also be inspected at any time by code. The array will be "
        "sorted upon loading, though may be temporarily unsorted while editing.");
    // A pasted event goes in at the index; a new one is named
    // "<new_event>" and placed halfway between its neighbours. The binary
    // also searches the sorted events for the pasted event's frame and
    // ignores the result. At 0x2ED260.
    arrayMetadata.mInsert = [](const PropAccessorArgs& args) {
        const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-1].mValue;
        PropArray<AnimResource::Event>& events = *static_cast<PropArray<AnimResource::Event>*>(args.mStorage);
        if (args.mOldValue != nullptr) {
            events._Insert(index, args.mOldValue);
            return;
        }
        AnimResource::Event event;
        event.mName = Symbol("<new_event>");
        event.mFrame = 0.0f;
        const unsigned int size = events.size();
        if (size != 0) {
            if (index == 0) {
                event.mFrame = events[0].mFrame;
            } else if (index == size) {
                event.mFrame = events[size - 1].mFrame;
            } else {
                event.mFrame = (events[index].mFrame + events[index - 1].mFrame) * 0.5f;
            }
        }
        events._Insert(index, &event);
    };

    GroupMetadata& itemMetadata = static_cast<GroupMetadata&>(*item.mMetadata.Get());
    const AnimResource::Event prototype = {0.0f, Symbol()};
    // At 0x2ED520.
    itemMetadata.mDisplayName._Set([](const TagArgs& args) {
        static Symbol event;
        if (event == Symbol()) {
            event = Symbol("event");
        }
        Symbol value;
        ReadProp(args.mComponent, MemberPath(args.mPath, event), value, kPropertySymbol);
        return String(value.Str());
    });
    // At 0x2ED9E0.
    itemMetadata.mSummary._Set([](const TagArgs& args) {
        static Symbol frameEditorView;
        if (frameEditorView == Symbol()) {
            frameEditorView = Symbol("frame_editor_view");
        }
        static Symbol percent;
        if (percent == Symbol()) {
            percent = Symbol("percent");
        }
        static Symbol frame;
        if (frame == Symbol()) {
            frame = Symbol("frame");
        }
        static Symbol time;
        if (time == Symbol()) {
            time = Symbol("time");
        }
        const PropPath view = MemberPath(args.mPath, frameEditorView);
        const char* const units = EventTimeUnits(EventAnim(args));
        float frameValue = 0.0f;
        ReadProp(args.mComponent, MemberPath(view, frame), frameValue, kPropertyFloat);
        float timeValue = 0.0f;
        ReadProp(args.mComponent, MemberPath(view, time), timeValue, kPropertyFloat);
        float percentValue = 0.0f;
        ReadProp(args.mComponent, MemberPath(view, percent), percentValue, kPropertyFloat);
        FormatString format("frame: %.04f, %s: %0.04f, %.04f%%");
        format << frameValue << units << timeValue << percentValue;
        return String(format.Str());
    });
    PropRegistry& members = itemMetadata.mRegistry;

    SymbolMetadata& event = TypeSpecificMetadata(
        members.RegisterProp("event", offsetof(AnimResource::Event, mName), kPropertySymbol, sizeof(Symbol)),
        prototype.mName);
    event.mDefault = prototype.mName;
    static_cast<void>(Symbol("prop"));
    event.mHelp = String("Name of the event");

    FloatMetadata& frame = TypeSpecificMetadata(
        members.RegisterProp("frame", offsetof(AnimResource::Event, mFrame), kPropertyFloat, sizeof(float)),
        prototype.mFrame);
    frame.mDefault = prototype.mFrame;
    static_cast<void>(Symbol("prop"));
    frame.mIsHidden = true;

    GroupMetadata& view = static_cast<GroupMetadata&>(
        *members.RegisterProp("frame_editor_view", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    view.mDisplayName = String("position");
    view.mHelp = String(
        "The position within the animation at which the event occurs.\n\nThis value is represented in three "
        "ways: as a percentage of the animation's duration, as a frame number within the animation, and as an "
        "amount of real time (in seconds or beats, according to the animation's specified units).");
    view.mKind = 1;
    PropRegistry& viewMembers = view.mRegistry;

    const float frameType = 0.0f;
    FloatMetadata& viewFrame =
        TypeSpecificMetadata(viewMembers.RegisterProp("frame", -1, kPropertyFloat, 0), frameType);
    static_cast<void>(Symbol("prop"));
    viewFrame.mHelp = String("The event's position, expressed as a specific frame within the animation.");
    viewFrame.mMin._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        return anim != nullptr ? anim->GetAuthoredStartFrame() : 0.0f;
    });
    viewFrame.mMax._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        return anim != nullptr ? anim->GetAuthoredEndFrame() : 0.0f;
    });
    viewFrame.mUnits = Symbol("(frame)");
    // Hidden while the clip's range has no duration.
    viewFrame.mIsHidden._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return true;
        }
        const float end = anim->GetTimeFromFrame(anim->GetEndFrame(), kNumTimeUnits);
        return end - anim->GetTimeFromFrame(anim->GetStartFrame(), kNumTimeUnits) == 0.0f;
    });
    // At 0x2EEA80 and 0x2EEA90.
    viewFrame.mGetter = [](const PropAccessorArgs& args) { return GetEventFrame(args); };
    viewFrame.mSetter = [](const PropAccessorArgs& args) {
        SetEventFrame(args, *static_cast<const float*>(args.mValue));
    };

    const float timeType = 0.0f;
    FloatMetadata& viewTime = TypeSpecificMetadata(viewMembers.RegisterProp("time", -1, kPropertyFloat, 0), timeType);
    static_cast<void>(Symbol("prop"));
    viewTime.mHelp = String("The event's position, expressed as seconds or beats.");
    viewTime.mMin._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return FractionToTime(anim, RangeFraction(anim, anim->GetAuthoredStartFrame(), 1.0f));
    });
    viewTime.mMax._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return FractionToTime(anim, RangeFraction(anim, anim->GetAuthoredEndFrame(), 1.0f));
    });
    viewTime.mUnits._Set([](const TagArgs& args) { return Symbol(EventTimeUnits(EventAnim(args))); });
    // At 0x2EEFA0.
    viewTime.mGetter = [](const PropAccessorArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return FractionToTime(anim, RangeFraction(anim, GetEventFrame(args), 1.0f));
    };
    // At 0x2EF070.
    viewTime.mSetter = [](const PropAccessorArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return;
        }
        const float start = anim->GetTimeFromFrame(anim->GetStartFrame(), kNumTimeUnits);
        const float end = anim->GetTimeFromFrame(anim->GetEndFrame(), kNumTimeUnits);
        const float fraction = end == start ? 1.0f : (*static_cast<const float*>(args.mValue) - start) / (end - start);
        const float first = anim->GetStartFrame();
        const float frame = first + (anim->GetEndFrame() - first) * fraction;
        SetEventFrame(args, anim->FrameToEventFrame(frame));
    };

    const float percentType = 0.0f;
    FloatMetadata& viewPercent =
        TypeSpecificMetadata(viewMembers.RegisterProp("percent", -1, kPropertyFloat, 0), percentType);
    static_cast<void>(Symbol("prop"));
    viewPercent.mHelp = String("The event's position, expressed as a percentage of the animation's duration.");
    viewPercent.mMin._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return RangeFraction(anim, anim->GetAuthoredStartFrame(), 1.0f) * 100.0f;
    });
    viewPercent.mMax._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return RangeFraction(anim, anim->GetAuthoredEndFrame(), 1.0f) * 100.0f;
    });
    viewPercent.mUnits = Symbol("%");
    // At 0x2EF450.
    viewPercent.mGetter = [](const PropAccessorArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return RangeFraction(anim, GetEventFrame(args), 1.0f) * 100.0f;
    };
    // At 0x2EF4E0.
    viewPercent.mSetter = [](const PropAccessorArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return;
        }
        const float start = anim->GetStartFrame();
        const float end = anim->GetEndFrame();
        const float frame = start + (end - start) * 0.01f * *static_cast<const float*>(args.mValue);
        SetEventFrame(args, anim->FrameToEventFrame(frame));
    };
}

// The mode the entity is entered in: 1 the game mode, 2 the edit mode, 0
// when it is not entered. Inlined into Refresh. Name not in the reference
// map.
unsigned int EnteredMode(const Entity* entity) {
    const unsigned int flags = entity->mFlags;
    return (flags & 0x10) != 0 ? (flags >> 1) & 3 : 0;
}

// The RndSkeletonPoseCom of the object's entity root, which must exist:
// the search has no bound. The clip is built on its skeleton resource.
// Inlined into _OnResourcesLoaded and Refresh. Name not in the reference
// map.
RndSkeletonPoseCom* ExistingSkeletonPose(const GameObject* object) {
    return object->mEntity->GetRoot()->GetExistingCom<RndSkeletonPoseCom>();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x2EFEC0).
Symbol CharClipCom::sId("CharClip");
Symbol CharClipCom::sClassName("CharClip");
PropRegistry CharClipCom::sPropRegistry;
ComMetaData CharClipCom::sMetaData;
CharClipCom* CharClipCom::sPreviewCom;

// Reconstructed from eboot.elf at 0x2E5C90.
CharClipCom::CharClipCom()
    : mConfig(0),
      mPlaybackUnits(0),
      mIsLooping(false),
      mEvents(),
      mClipPath(),
      mResource(),
      mHideDisabledBones(false),
      mSourceChanged(false),
      mRelativeSourceChanged(false),
      mBuiltConfig(2),
      mPreview(false),
      mPause(false),
      mPreviewFrame(0.0f),
      mPreviewID(),
      mCreatedComponents(),
      mNeedsRefresh(false) {}

// Inlined into _Imprint (0x2EACE0).
CharClipCom::CharClipCom(const CharClipCom& other)
    : Component(other),
      mConfig(other.mConfig),
      mPlaybackUnits(other.mPlaybackUnits),
      mIsLooping(other.mIsLooping),
      mEvents(other.mEvents),
      mClipPath(),
      mResource(),
      mHideDisabledBones(false),
      mSourceChanged(false),
      mRelativeSourceChanged(false),
      mBuiltConfig(2),
      mPreview(false),
      mPause(false),
      mPreviewFrame(0.0f),
      mPreviewID(),
      mCreatedComponents(),
      mNeedsRefresh(false) {}

// Reconstructed from eboot.elf at 0x2E5E50. The deleting destructor is at
// 0x2E6020.
CharClipCom::~CharClipCom() {}

// Reconstructed from eboot.elf at 0x2E6040.
void CharClipCom::_PostCreate() {
    mConfig.mBeatTrack.Resize(2);
}

// Reconstructed from eboot.elf at 0x2E6050. Turning "preview" off stops
// the playback and clears sPreviewCom through its callback.
void CharClipCom::_PreDestroy(DestroyType type) {
    if (sPreviewCom != this) {
        return;
    }
    if (type == kDestroyObject || type == kDestroyComponent) {
        static Symbol preview;
        if (preview == Symbol()) {
            preview = Symbol("preview");
        }
        SetBoolProp(this, preview, false);
    } else {
        sPreviewCom = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x2E6360. The registry is at revision 1;
// a clip saved before it gets two beat/frame pairs.
void CharClipCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<CharClipConfig::BeatTrackEntry>(Symbol("CharClipConfig::BeatTrackEntry"));
    CharClipCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    BoolMetadata& preview = TypeSpecificMetadata(
        registry.RegisterProp("preview", offsetof(CharClipCom, mPreview), kPropertyBool, sizeof(bool)),
        prototype.mPreview);
    preview.mDefault = prototype.mPreview;
    static_cast<void>(Symbol("prop"));
    preview.mHelp = String("Should this clip play back while editing?");
    preview.mSaves = false;
    preview.mHistory = false;
    preview.mIsHidden._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return true;
    });
    // Only a clip whose resource loaded can preview.
    preview.mReadOnly._Set([](const TagArgs& args) {
        const CharClipResource* const resource = Clip(args).mResource.Get();
        return resource == nullptr || resource->Fail();
    });
    // At 0x2EB370: turning the preview on stops the previous clip's and
    // plays this clip, looping, at the paused frame; turning it off stops
    // the playback and returns the skeleton to its bind pose once nothing
    // else plays.
    preview.mOnChanged = [](const PropAccessorArgs& args) {
        CharClipCom* clip = &Clip(args);
        AnimDriverCom* const driver = clip->_GetDriver();
        if (*static_cast<const bool*>(args.mValue) && sPreviewCom != clip) {
            if (sPreviewCom != nullptr) {
                static Symbol preview;
                if (preview == Symbol()) {
                    preview = Symbol("preview");
                }
                SetBoolProp(sPreviewCom, preview, false);
                clip = &Clip(args);
            }
            const ResourcePath path = clip->mResource->mPath;
            const AnimOptions options;
            clip->mPreviewID = driver->StartAnim(path, options);
            PlaybackInfo* const info = driver->GetPlaybackInfo(clip->mPreviewID);
            if (info != nullptr) {
                info->mPaused = clip->mPause;
                if (clip->mPause) {
                    info->SetCurrentFrame(clip->mPreviewFrame);
                }
                info->mLooping = true;
            }
            sPreviewCom = clip;
        } else if (sPreviewCom == clip) {
            driver->StopAnim(clip->mPreviewID);
            if (driver->GetActiveAnimsCount() == 0) {
                RndSkeletonPoseCom* const pose = clip->_GetSkeletonPose();
                if (pose != nullptr) {
                    pose->mPose.ResetToBindPose(pose->GetSkeleton());
                }
            }
            sPreviewCom = nullptr;
        }
    };

    BoolMetadata& pause = TypeSpecificMetadata(
        registry.RegisterProp("pause", offsetof(CharClipCom, mPause), kPropertyBool, sizeof(bool)),
        prototype.mPause);
    pause.mDefault = prototype.mPause;
    static_cast<void>(Symbol("prop"));
    pause.mHelp = String("Should the preview playback be paused?");
    pause.mSaves = false;
    pause.mHistory = false;
    pause.mIsHidden._Set([](const TagArgs& args) { return !Clip(args).mPreview; });
    // At 0x2EB850.
    pause.mOnChanged = [](const PropAccessorArgs& args) {
        CharClipCom& clip = Clip(args);
        PlaybackInfo* const info = clip._GetDriver()->GetPlaybackInfo(clip.mPreviewID);
        if (info != nullptr) {
            info->mPaused = *static_cast<const bool*>(args.mValue);
        }
    };

    FloatMetadata& frame = TypeSpecificMetadata(
        registry.RegisterProp("frame", offsetof(CharClipCom, mPreviewFrame), kPropertyFloat, sizeof(float)),
        prototype.mPreviewFrame);
    frame.mDefault = prototype.mPreviewFrame;
    static_cast<void>(Symbol("prop"));
    frame.mHelp = String("When previewing, what frame is currently displayed?");
    frame.mSaves = false;
    frame.mHistory = false;
    frame.mIsHidden._Set([](const TagArgs& args) { return !Clip(args).mPreview; });
    frame.mReadOnly._Set([](const TagArgs& args) { return !Clip(args).mPause; });
    // At 0x2EB8E0: a running preview reports its frame in the source
    // animation.
    frame.mGetter = [](const PropAccessorArgs& args) {
        CharClipCom& clip = Clip(args);
        if (!clip.mPause) {
            PlaybackInfo* const info = clip._GetDriver()->GetPlaybackInfo(clip.mPreviewID);
            if (info != nullptr) {
                clip.mPreviewFrame = static_cast<float>(clip.mConfig.mStartFrame) + info->GetCurrentFrame();
            }
        }
        return clip.mPreviewFrame;
    };
    // At 0x2EB9A0.
    frame.mOnChanged = [](const PropAccessorArgs& args) {
        CharClipCom& clip = Clip(args);
        PlaybackInfo* const info = clip._GetDriver()->GetPlaybackInfo(clip.mPreviewID);
        if (info != nullptr) {
            info->SetCurrentFrame(
                *static_cast<const float*>(args.mValue) - static_cast<float>(clip.mConfig.mStartFrame));
        }
    };
    frame.mMin._Set([](const TagArgs& args) {
        const CharClipCom& clip = Clip(args);
        if (clip.mResource == nullptr || clip.mResource->Fail()) {
            return 0.0f;
        }
        return static_cast<float>(clip.mConfig.mStartFrame) + clip.mResource->GetStartFrame();
    });
    frame.mMax._Set([](const TagArgs& args) {
        const CharClipCom& clip = Clip(args);
        if (clip.mResource == nullptr || clip.mResource->Fail()) {
            return 0.0f;
        }
        return static_cast<float>(clip.mConfig.mStartFrame) + clip.mResource->GetEndFrame();
    });
    frame.mSlider = true;

    ResourcePathMetadata& sourceFbx = TypeSpecificMetadata(
        registry.RegisterProp(
            "source_fbx",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mSourceFbx),
            kPropertyResourcePath,
            sizeof(ResourcePath)),
        prototype.mConfig.mSourceFbx);
    sourceFbx.mDefault = prototype.mConfig.mSourceFbx;
    static_cast<void>(Symbol("prop"));
    sourceFbx.mHelp = String("The FBX file containing the source animation.");
    sourceFbx.mCustomLabel = Symbol("FBX files");
    sourceFbx.mCustomExtensions = {Symbol("fbx")};
    sourceFbx.mAnimatable = false;
    sourceFbx.mInstanceFile = true;
    // At 0x2EBA50.
    sourceFbx.mOnChanged = [](const PropAccessorArgs& args) { Clip(args).mSourceChanged = true; };

    IntMetadata<int>& startFrame = TypeSpecificMetadata(
        registry.RegisterProp(
            "start_frame",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mStartFrame),
            kPropertyInt,
            sizeof(int)),
        prototype.mConfig.mStartFrame);
    startFrame.mDefault = prototype.mConfig.mStartFrame;
    static_cast<void>(Symbol("prop"));
    startFrame.mHelp = String("The first frame of this clip within the source FBX animation.");
    startFrame.mMin._Set([](const TagArgs& args) { return Clip(args).mResource->mSourceStartFrame; });
    startFrame.mMax._Set([](const TagArgs& args) { return Clip(args).mResource->mSourceEndFrame; });
    startFrame.mDefault._Set([](const TagArgs& args) { return Clip(args).mResource->mSourceStartFrame; });
    // At 0x2EBA60.
    startFrame.mOnChanged = [](const PropAccessorArgs& args) { Clip(args).mNeedsRefresh = true; };
    startFrame.mEnabled = false;
    startFrame.mAnimatable = false;

    IntMetadata<int>& endFrame = TypeSpecificMetadata(
        registry.RegisterProp(
            "end_frame",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mEndFrame),
            kPropertyInt,
            sizeof(int)),
        prototype.mConfig.mEndFrame);
    endFrame.mDefault = prototype.mConfig.mEndFrame;
    static_cast<void>(Symbol("prop"));
    endFrame.mHelp = String("The end frame of this clip within the source FBX animation (exclusive).");
    endFrame.mMin._Set([](const TagArgs& args) { return Clip(args).mResource->mSourceStartFrame; });
    endFrame.mMax._Set([](const TagArgs& args) { return Clip(args).mResource->mSourceEndFrame; });
    endFrame.mDefault._Set([](const TagArgs& args) { return Clip(args).mResource->mSourceEndFrame; });
    // At 0x2EBA70.
    endFrame.mOnChanged = [](const PropAccessorArgs& args) { Clip(args).mNeedsRefresh = true; };
    endFrame.mEnabled = false;
    endFrame.mAnimatable = false;

    IntMetadata<int>& relativity = static_cast<IntMetadata<int>&>(
        *registry
             .RegisterProp(
                 "relativity",
                 offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mRelativity),
                 kPropertyInt,
                 sizeof(int))
             .mMetadata.Get());
    relativity.mDefault = prototype.mConfig.mRelativity;
    static_cast<void>(Symbol("prop"));
    relativity.mHelp = String("Whether it's played back additively, relative to the bind pose or another file");
    relativity.mAllowedValues = CharAnim::GetRelativeAllowedValues();
    relativity.mAnimatable = false;

    ResourcePathMetadata& relativeSourceFbx = TypeSpecificMetadata(
        registry.RegisterProp(
            "relative_source_fbx",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mRelativeSourceFbx),
            kPropertyResourcePath,
            sizeof(ResourcePath)),
        prototype.mConfig.mRelativeSourceFbx);
    relativeSourceFbx.mDefault = prototype.mConfig.mRelativeSourceFbx;
    static_cast<void>(Symbol("prop"));
    relativeSourceFbx.mHelp = String("The FBX file containing the animation to which this clip should be relative.");
    relativeSourceFbx.mCustomLabel = Symbol("FBX files");
    relativeSourceFbx.mCustomExtensions = {Symbol("fbx")};
    // Only "Other Relative" plays relative to another file.
    relativeSourceFbx.mIsHidden._Set([](const TagArgs& args) { return Clip(args).mConfig.mRelativity != 2; });
    relativeSourceFbx.mAnimatable = false;
    relativeSourceFbx.mInstanceFile = true;
    // At 0x2EBA80.
    relativeSourceFbx.mOnChanged = [](const PropAccessorArgs& args) { Clip(args).mRelativeSourceChanged = true; };

    IntMetadata<int>& relativeFrame = TypeSpecificMetadata(
        registry.RegisterProp(
            "relative_frame",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mRelativeFrame),
            kPropertyInt,
            sizeof(int)),
        prototype.mConfig.mRelativeFrame);
    relativeFrame.mDefault = prototype.mConfig.mRelativeFrame;
    static_cast<void>(Symbol("prop"));
    relativeFrame.mHelp = String(
        "The frame within the relative animation against which this animation should be relative.");
    relativeFrame.mMin._Set([](const TagArgs& args) { return Clip(args).mResource->mRelativeStartFrame; });
    relativeFrame.mMax._Set([](const TagArgs& args) { return Clip(args).mResource->mRelativeEndFrame; });
    relativeFrame.mIsHidden._Set([](const TagArgs& args) { return Clip(args).mConfig.mRelativity != 2; });
    relativeFrame.mAnimatable = false;

    IntMetadata<int>& playbackUnits = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("playback_units", offsetof(CharClipCom, mPlaybackUnits), kPropertyInt, sizeof(int))
             .mMetadata.Get());
    playbackUnits.mDefault = prototype.mPlaybackUnits;
    static_cast<void>(Symbol("prop"));
    playbackUnits.mHelp = String("How to play back the animation, which timeline, and whether aligned or not");
    playbackUnits.mAllowedValues = CharAnim::GetUnitsAllowedValues();
    playbackUnits.mAnimatable = false;
    // At 0x2EBA90.
    playbackUnits.mOnChanged = [](const PropAccessorArgs& args) {
        CharClipCom& clip = Clip(args);
        clip.mResource->SetUnits(clip.mPlaybackUnits);
        clip._UpdatePreviewRange();
    };

    PropInfo& beatTrack = registry.RegisterPropArray(
        "beat_track",
        offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mBeatTrack),
        sizeof(PropArray<CharClipConfig::BeatTrackEntry>));
    ArrayMetadata& beatTrackMetadata = static_cast<ArrayMetadata&>(*beatTrack.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    beatTrackMetadata.mHelp = String(
        "For beat synced animations, these two beat/frame pairs define the alignment of animation frames to "
        "music beats. Note that beat is zero-based; i.e. 0.0 is beat 1, 1.0 is beat 2, etc.");
    beatTrackMetadata.mFixedSize = true;
    // Real-time clips are not beat synced.
    beatTrackMetadata.mIsHidden._Set([](const TagArgs& args) { return Clip(args).mPlaybackUnits == 0; });
    PropRegistry::RegisterPropArrayItem(beatTrack, kPropertyStruct, sizeof(CharClipConfig::BeatTrackEntry));
    GroupMetadata& beatEntry = static_cast<GroupMetadata&>(*beatTrackMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    {
        const CharClipConfig::BeatTrackEntry entryPrototype;
        // At 0x2ECF20.
        beatEntry.mDisplayName._Set([](const TagArgs& args) {
            FormatString format("beat / frame %d");
            format << ElementIndex(args);
            return String(format.Str());
        });
        beatEntry.mHelp = String(
            "A beat/frame pair that defines the alignment of this animation to the beat timeline. Note that "
            "beat is zero-based; i.e. 0.0 is beat 1, 1.0 is beat 2, etc.");
        beatEntry.mKind = 1;
        PropRegistry& members = beatEntry.mRegistry;

        FloatMetadata& beat = TypeSpecificMetadata(
            members.RegisterProp("beat", offsetof(CharClipConfig::BeatTrackEntry, mBeat), kPropertyFloat, sizeof(float)),
            entryPrototype.mBeat);
        beat.mDefault = entryPrototype.mBeat;
        static_cast<void>(Symbol("prop"));
        beat.mHelp = String(
            "The beat that the specified frame corresponds to. Note that this value is zero-based; i.e. 0.0 is "
            "beat 1, 1.0 is beat 2, etc.");
        beat.mUnits = Symbol("(beat)");

        FloatMetadata& beatFrame = TypeSpecificMetadata(
            members.RegisterProp(
                "frame", offsetof(CharClipConfig::BeatTrackEntry, mFrame), kPropertyFloat, sizeof(float)),
            entryPrototype.mFrame);
        beatFrame.mDefault = entryPrototype.mFrame;
        static_cast<void>(Symbol("prop"));
        beatFrame.mHelp = String("The frame corresponding to the specified beat.");
        beatFrame.mUnits = Symbol("(frame)");
        beatFrame.mMin._Set(
            [](const TagArgs& args) { return static_cast<float>(Clip(args).mResource->mSourceStartFrame); });
        beatFrame.mMax._Set(
            [](const TagArgs& args) { return static_cast<float>(Clip(args).mResource->mSourceEndFrame); });
    }

    BoolMetadata& isLooping = TypeSpecificMetadata(
        registry.RegisterProp("is_looping", offsetof(CharClipCom, mIsLooping), kPropertyBool, sizeof(bool)),
        prototype.mIsLooping);
    isLooping.mDefault = prototype.mIsLooping;
    static_cast<void>(Symbol("prop"));
    isLooping.mHelp = String(
        "Should this animation loop by default when played, implies is seamlessly self looping");
    isLooping.mAnimatable = false;
    // At 0x2EBB70.
    isLooping.mOnChanged = [](const PropAccessorArgs& args) {
        CharClipCom& clip = Clip(args);
        clip.mResource->SetLooping(clip.mIsLooping);
    };

    IntMetadata<int>& blendMode = static_cast<IntMetadata<int>&>(
        *registry
             .RegisterProp(
                 "blend_mode",
                 offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mBlendMode),
                 kPropertyInt,
                 sizeof(int))
             .mMetadata.Get());
    blendMode.mDefault = static_cast<int>(prototype.mConfig.mBlendMode);
    static_cast<void>(Symbol("prop"));
    blendMode.mHelp = String(
        "How to blend in the clip, either additively, normalizing all at the end, or scaling down any "
        "previously blended data");
    blendMode.mAllowedValues = AnimResource::GetBlendModeAllowedValues();
    blendMode.mAnimatable = false;

    IntMetadata<unsigned long>& subtreeIndex = TypeSpecificMetadata(
        registry.RegisterProp(
            "subtree_index",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mSubTreeIndex),
            kPropertyUInt64,
            sizeof(unsigned long)),
        prototype.mConfig.mSubTreeIndex);
    subtreeIndex.mDefault = prototype.mConfig.mSubTreeIndex;
    static_cast<void>(Symbol("prop"));
    subtreeIndex.mHelp = String(
        "Which subtree index am I on? All animations with a given subtree index are blended against each other");

    BoolMetadata& blendFromRoot = TypeSpecificMetadata(
        registry.RegisterProp(
            "blend_from_root",
            offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mBlendFromRoot),
            kPropertyBool,
            sizeof(bool)),
        prototype.mConfig.mBlendFromRoot);
    blendFromRoot.mDefault = prototype.mConfig.mBlendFromRoot;
    static_cast<void>(Symbol("prop"));
    blendFromRoot.mHelp = String("Do we blend relative to the char_root, rather than absolute?");
    blendFromRoot.mAnimatable = false;

    RegisterAnimEvents(registry, Symbol("events"), offsetof(CharClipCom, mEvents));

    ActionMetadata& disableAll =
        static_cast<ActionMetadata&>(*registry.RegisterAction("disable_all_bones").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    disableAll.mHelp = String("Disables all the bone channels.");
    disableAll.mAction = [](const TagArgs& args) { Clip(args).DisableAllBones(); };

    ActionMetadata& enableAll =
        static_cast<ActionMetadata&>(*registry.RegisterAction("enable_all_bones").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    enableAll.mHelp = String("Enables pos and full rotation on all the bone channels.");
    enableAll.mAction = [](const TagArgs& args) { Clip(args).EnableAllBones(); };

    BoolMetadata& hideDisabledBones = TypeSpecificMetadata(
        registry.RegisterProp(
            "hide_disabled_bones", offsetof(CharClipCom, mHideDisabledBones), kPropertyBool, sizeof(bool)),
        prototype.mHideDisabledBones);
    hideDisabledBones.mDefault = prototype.mHideDisabledBones;
    static_cast<void>(Symbol("prop"));
    hideDisabledBones.mHelp = String(
        "If true, hides the inactive bones in the bone mask table (i.e. bones where no DOF are captured.)");
    hideDisabledBones.mSaves = false;
    hideDisabledBones.mHistory = false;
    hideDisabledBones.mAnimatable = false;

    PropInfo& bones = registry.RegisterPropArray(
        "bones",
        offsetof(CharClipCom, mConfig) + offsetof(CharClipConfig, mBones),
        sizeof(PropArray<CharBoneChannelMask>));
    ArrayMetadata& bonesMetadata = static_cast<ArrayMetadata&>(*bones.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    bonesMetadata.mAnimatable = false;
    bonesMetadata.mFixedSize = true;
    PropRegistry::RegisterPropArrayItem(bones, kPropertyStruct, sizeof(CharBoneChannelMask));
    GroupMetadata& bone = static_cast<GroupMetadata&>(*bonesMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    {
        const CharBoneChannelMask bonePrototype;
        bone.mKind = 1;
        // The help names the bone's parent in the skeleton. At 0x2EF850.
        bone.mHelp._Set([](const TagArgs& args) {
            const CharSkeleton* const skeleton = Clip(args)._GetSkeletonPose()->GetSkeleton();
            const Symbol parent = skeleton->mNames[skeleton->mParentIndices[ElementIndex(args)]];
            FormatString format("Name, Position, Rotation, Scale, and InverseParentScale. Parent bone is %s.");
            format << parent;
            return String(format.Str());
        });
        // "+/-" keeps every channel of a masked bone and masks a bone that
        // keeps any. At 0x2EFA60.
        bone.mInlineAction = new PropMetadata::InlineAction();
        bone.mInlineAction->mLabel = "+/-";
        bone.mInlineAction->mEnabled = true;
        bone.mInlineAction->mAction = [](const TagArgs& args) {
            CharBoneChannelMask& element = Clip(args).mConfig.mBones[args.mPath[1].mValue];
            if (element.mUsePosition || element.mUseScale || element.mUseInverseParentScale
                || element.mRotation != CharBoneChannelMask::kRotationNone) {
                element.SetDisable();
            } else {
                element.SetSRTIPS();
            }
        };
        // With "hide_disabled_bones", the bones that keep no channel are
        // hidden.
        bone.mIsHidden._Set([](const TagArgs& args) {
            const CharClipCom& clip = Clip(args);
            if (!clip.mHideDisabledBones) {
                return false;
            }
            const CharBoneChannelMask& element = clip.mConfig.mBones[ElementIndex(args)];
            return !element.mUsePosition && !element.mUseScale && !element.mUseInverseParentScale
                && element.mRotation == CharBoneChannelMask::kRotationNone;
        });
        bone.mDisplayName._Set([](const TagArgs& args) {
            return String(Clip(args).mConfig.mBones[ElementIndex(args)].mName.Str());
        });

        PropRegistry& members = bone.mRegistry;
        SymbolMetadata& name = TypeSpecificMetadata(
            members.RegisterProp("name", offsetof(CharBoneChannelMask, mName), kPropertySymbol, sizeof(Symbol)),
            bonePrototype.mName);
        name.mDefault = bonePrototype.mName;
        static_cast<void>(Symbol("prop"));
        name.mIsHidden = true;
        name.mReadOnly = true;

        BoolMetadata& usePosition = TypeSpecificMetadata(
            members.RegisterProp(
                "use_position", offsetof(CharBoneChannelMask, mUsePosition), kPropertyBool, sizeof(bool)),
            bonePrototype.mUsePosition);
        usePosition.mDefault = bonePrototype.mUsePosition;
        static_cast<void>(Symbol("prop"));
        usePosition.mHelp = String("Should this bone's position be used?");

        IntMetadata<int>& rotation = static_cast<IntMetadata<int>&>(
            *members.RegisterProp("rotation", offsetof(CharBoneChannelMask, mRotation), kPropertyInt, sizeof(int))
                 .mMetadata.Get());
        rotation.mDefault = static_cast<int>(bonePrototype.mRotation);
        static_cast<void>(Symbol("prop"));
        rotation.mAllowedValues = CharBoneChannelMask::GetRotationAllowedValues();
        rotation.mHelp = String("What aspects of this bone's rotation be used (if any)?");

        BoolMetadata& useScale = TypeSpecificMetadata(
            members.RegisterProp("use_scale", offsetof(CharBoneChannelMask, mUseScale), kPropertyBool, sizeof(bool)),
            bonePrototype.mUseScale);
        useScale.mDefault = bonePrototype.mUseScale;
        static_cast<void>(Symbol("prop"));
        useScale.mHelp = String("Should this bone's scale be used?");

        BoolMetadata& useInverseParentScale = TypeSpecificMetadata(
            members.RegisterProp(
                "use_inverse_parent_scale",
                offsetof(CharBoneChannelMask, mUseInverseParentScale),
                kPropertyBool,
                sizeof(bool)),
            bonePrototype.mUseInverseParentScale);
        useInverseParentScale.mDefault = bonePrototype.mUseInverseParentScale;
        static_cast<void>(Symbol("prop"));
        useInverseParentScale.mHelp = String("Should this bone's inverse parent scale be used?");
    }

    registry.SetCurrentRev(1);
    // At 0x2EFE30: a clip saved before revision 1 without beat/frame pairs
    // gets two.
    registry.mRevisionFunc = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev > 0) {
            return;
        }
        CharClipCom& clip = *static_cast<CharClipCom*>(args.mComponent);
        if (clip.mConfig.mBeatTrack.size() == 0) {
            clip.mConfig.mBeatTrack.Resize(2);
        }
    };

    metadata.mDescription = "Defines a BlendTree-compatible character animation clip.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(CharResource::Id());
    metadata.mReserved = true;
}

// Reconstructed from eboot.elf at 0x2E9AE0. The clip lives in a
// "<layer path>_clips" folder (dots become underscores; "temp" for an
// unsaved layer) as "<object name>_<serial>.charclip". The bone masks are
// matched to the skeleton's bones by name, new bones keeping every channel
// when the clip had none. A changed source or relative animation takes the
// resource's frame range, which the rebuild then provides.
bool CharClipCom::_OnResourcesLoaded() {
    const GameObjectId id = mObject->mId;
    Entity* const entity = mObject->mEntity;
    String layer(
        id.Layer() == kMainLayerIndex ? entity->mResource->mPath.mPath : entity->GetLayerPath(id.Layer()).mPath);
    if (layer.c_str()[0] == '\0') {
        layer = "temp";
    }
    layer.ReplaceAll('.', '_');
    layer += "_clips";
    const Symbol objectName = mObject->mName;
    FormatString format("%s/%s_%d.charclip");
    format << layer.c_str() << objectName << (id.mId >> 16);
    ResourcePath path;
    path = format.Str();
    mResource = Resource::GetOrLoad<CharClipResource>(path, false);
    mResource->mObjectId = id;
    mResource->SetUnits(mPlaybackUnits);
    mResource->SetLooping(mIsLooping);

    const CharSkeleton* const skeleton = _GetSkeletonPose()->GetSkeleton();
    if (skeleton != nullptr) {
        const eastl::vector<Symbol>& names = skeleton->mNames;
        const unsigned long count = names.size();
        const unsigned int oldCount = mConfig.mBones.size();
        bool matches = count == oldCount;
        for (unsigned long i = 0; matches && i < count; ++i) {
            matches = mConfig.mBones[i].mName == names[i];
        }
        if (!matches) {
            CharBoneChannelMask fresh;
            if (oldCount == 0) {
                fresh.SetSRTIPS();
            }
            PropArray<CharBoneChannelMask> masks;
            masks.Resize(count);
            for (unsigned long i = 0; i < count; ++i) {
                unsigned long j = 0;
                while (j < oldCount && mConfig.mBones[j].mName != names[i]) {
                    ++j;
                }
                if (j < oldCount) {
                    masks[i] = mConfig.mBones[j];
                } else {
                    masks[i] = fresh;
                    masks[i].mName = names[i];
                }
            }
            mConfig.mBones.Resize(0);
            for (unsigned long i = 0; i < count; ++i) {
                mConfig.mBones._Insert(mConfig.mBones.size(), &masks[i]);
            }
        }

        if (mSourceChanged) {
            mConfig.mStartFrame = 0;
            mConfig.mEndFrame = 0;
        }
        if (mRelativeSourceChanged) {
            mConfig.mRelativeFrame = 0;
        }
        mResource->Create(mConfig, ExistingSkeletonPose(mObject)->mSkeletonResource.Get());
        bool rebuild = false;
        if (mSourceChanged) {
            mConfig.mStartFrame = mResource->mSourceStartFrame;
            mConfig.mEndFrame = mResource->mSourceEndFrame;
            mSourceChanged = false;
            rebuild = true;
        }
        if (mRelativeSourceChanged) {
            mConfig.mRelativeFrame = mResource->mRelativeStartFrame;
            mRelativeSourceChanged = false;
            rebuild = true;
        }
        if (rebuild) {
            mResource->Create(mConfig, ExistingSkeletonPose(mObject)->mSkeletonResource.Get());
        }
        mBuiltConfig = mConfig;
    }

    eastl::sort(mEvents.begin(), mEvents.end(), [](const AnimResource::Event& a, const AnimResource::Event& b) {
        return a.mFrame < b.mFrame;
    });
    return true;
}

// Reconstructed from eboot.elf at 0x2EA310.
void CharClipCom::_Poll() {}

// Reconstructed from eboot.elf at 0x2EA320.
void CharClipCom::_EditPoll() {
    if (mNeedsRefresh || _GetDriver()->IsAnimating(ResourcePath(mResource->mPath), true)) {
        Refresh();
        mNeedsRefresh = false;
    }
}

// Reconstructed from eboot.elf at 0x2EA3F0.
AnimDriverCom* CharClipCom::_GetDriver() const {
    return mObject->mEntity->GetRoot()->GetCom<AnimDriverCom>();
}

// Reconstructed from eboot.elf at 0x2EA450. The rebuild runs while the
// component listens to its entity's "component_created".
void CharClipCom::Refresh() {
    if (_GetSkeletonPose()->GetSkeleton() == nullptr || mConfig == mBuiltConfig) {
        return;
    }
    Entity* const entity = mObject->mEntity;
    MsgSource::EventSinkElem sink;
    sink.mSink = nullptr;
    sink.mSource = nullptr;
    static Symbol componentCreated;
    if (componentCreated == Symbol()) {
        componentCreated = Symbol("component_created");
    }
    entity->AddSink(&sink, reinterpret_cast<MsgSink*>(this), componentCreated, Symbol());
    mResource->Create(mConfig, ExistingSkeletonPose(mObject)->mSkeletonResource.Get());
    _UpdatePreviewRange();
    for (const Symbol* name = mCreatedComponents.begin(); name != mCreatedComponents.end(); ++name) {
        Component* const component = mObject->FindCom(*name);
        if (component == nullptr) {
            continue;
        }
        component->LoadResources(false);
        const unsigned int mode = EnteredMode(entity);
        if (mode == 2) {
            component->mEntered = true;
            component->_EditEnter();
        } else if (mode == 1) {
            component->mEntered = true;
            component->_Enter();
        }
    }
    mCreatedComponents.clear();
    mBuiltConfig = mConfig;
    LinkedList::Node& link = sink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        sink.mSource->RemoveSink(&sink);
    }
}

// Reconstructed from eboot.elf at 0x2EA870. The range runs from the clip's
// start to its end frame on the clip's own timeline.
void CharClipCom::_UpdatePreviewRange() {
    PlaybackInfo* const info = _GetDriver()->GetPlaybackInfo(mPreviewID);
    if (info == nullptr) {
        return;
    }
    info->mUnits = mResource->GetUnits();
    info->_DoAlignment();
    info->mEndTime =
        mResource->GetTimeFromFrame(static_cast<float>(mConfig.mEndFrame - mConfig.mStartFrame), kNumTimeUnits);
}

// Reconstructed from eboot.elf at 0x2EA940.
void CharClipCom::DisableAllBones() {
    for (unsigned long i = 0; i < mConfig.mBones.size(); ++i) {
        mConfig.mBones[i].SetDisable();
    }
}

// Reconstructed from eboot.elf at 0x2EA990. SetSRTIPS is inlined.
void CharClipCom::EnableAllBones() {
    for (unsigned long i = 0; i < mConfig.mBones.size(); ++i) {
        mConfig.mBones[i].SetSRTIPS();
    }
}

// Reconstructed from eboot.elf at 0x2EA9D0.
RndSkeletonPoseCom* CharClipCom::_GetSkeletonPose() const {
    return mObject->mEntity->GetRoot()->GetCom<RndSkeletonPoseCom>();
}

// Reconstructed from eboot.elf at 0x2EAA30. The object id in node 2 is
// evaluated and not used.
DataNode CharClipCom::_OnComponentCreated(DataArray* msg) {
    static_cast<void>(msg->Node(2).ObjectId(msg));
    mCreatedComponents.push_back(msg->Node(3).Sym(msg));
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x2EAB70. Component's handler is not
// consulted.
DataNode CharClipCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol componentCreated;
    if (componentCreated == Symbol()) {
        componentCreated = Symbol("component_created");
    }
    if (type == componentCreated) {
        DataNode result = _OnComponentCreated(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x2EAC60.
Symbol CharClipCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2EAC70.
Symbol CharClipCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2EAC80.
int CharClipCom::CurrentRev() const {
    return const_cast<CharClipCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2EACA0.
bool CharClipCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2EACD0.
Component* CharClipCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2EACE0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CharClipCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CharClipCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CharClipCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2EAFB0.
PropRegistry& CharClipCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2EAFC0.
ComMetaData& CharClipCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2B73A0. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* CharClipCom::_Create() {
    return new CharClipCom();
}

// Reconstructed from eboot.elf at 0x2AFE30. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharClipCom::Init() {
    InitComponentClass<CharClipCom>();
}
