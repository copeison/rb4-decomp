// anim/CharAnimCom.o (0x2D8150 to 0x2DD2EF). The class's registration and
// factory are emitted in anim/AnimInit.o. The binary emits the event sort
// (eastl::sort over AnimResource::Event, 0x2DCA10-0x2DD21F), the
// PropArray<AnimResource::Event> element operations (vtable 0x18F4D40) and
// the std::function wrappers of the registry's lambdas (vtables 0x18F4D90
// to 0x18F5150) here; the templates model them.
#include "anim/char/CharAnimCom.h"

#include <functional>
#include <new>

#include "anim/char/CharAnim.h"
#include "anim/char/CharAnimRawResource.h"
#include "anim/char/CharAnimResource.h"
#include "anim/char/CharBoneMaskResource.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "utl/containers/Sort.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

// The object's statics, in the order of its static initializer (0x2DD220),
// which first sets a shared header's null object id to -1 (0x19F83A8).
Symbol CharAnimCom::sId("CharAnim");
Symbol CharAnimCom::sClassName("CharAnim");
PropRegistry CharAnimCom::sPropRegistry;
ComMetaData CharAnimCom::sMetaData;
Symbol CharAnimCom::kSubObjectResource("rawanim");

namespace {

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

// The path of the event's member `name`, from the path of the event or of
// one of its members. Name not in the reference map.
PropPath MemberPath(const PropPath& path, Symbol name) {
    PropPath member = path;
    PropPath::Node node;
    member.PushNode(node = name);
    return member;
}

// The animation interface of the component whose event a view or tag
// reads. Names not in the reference map.
AnimResource* EventAnim(const TagArgs& args) {
    return static_cast<const CharAnimCom&>(args.mComponent).mAnim;
}
AnimResource* EventAnim(const PropAccessorArgs& args) {
    return static_cast<CharAnimCom*>(args.mComponent)->mAnim;
}

// The fraction of the playback range an event frame lies at (`empty` when
// the range is empty), and the time on the animation's own timeline of a
// fraction of the range. Inlined into the event views. Names not in the
// reference map.
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

// The units label of the event views: "beats" for an animation on the
// beat timeline, else "seconds". Name not in the reference map.
const char* EventTimeUnits(const AnimResource* anim) {
    if (anim == nullptr) {
        return "seconds";
    }
    return anim->GetUnits() == kTimeUnitsBeats ? "beats" : "seconds";
}

// Reconstructed from eboot.elf at 0x2D8EE0. Registers the events array:
// each element shows its name and a summary, inserts between its
// neighbours, and has a "frame_editor_view" that edits its frame as a
// frame, a time and a percentage of the animation, bounded by the
// animation's authored range. CharClipCom.o has its own copy (0x2E8F10).
// Name not in the reference map.
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
    // ignores the result.
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
    itemMetadata.mDisplayName._Set([](const TagArgs& args) {
        static Symbol event;
        if (event == Symbol()) {
            event = Symbol("event");
        }
        Symbol value;
        ReadProp(args.mComponent, MemberPath(args.mPath, event), value, kPropertySymbol);
        return String(value.Str());
    });
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
    // Hidden while the animation's range has no duration.
    viewFrame.mIsHidden._Set([](const TagArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return true;
        }
        const float end = anim->GetTimeFromFrame(anim->GetEndFrame(), kNumTimeUnits);
        return end - anim->GetTimeFromFrame(anim->GetStartFrame(), kNumTimeUnits) == 0.0f;
    });
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
    viewTime.mGetter = [](const PropAccessorArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return FractionToTime(anim, RangeFraction(anim, GetEventFrame(args), 1.0f));
    };
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
    viewPercent.mGetter = [](const PropAccessorArgs& args) {
        const AnimResource* const anim = EventAnim(args);
        if (anim == nullptr) {
            return 0.0f;
        }
        return RangeFraction(anim, GetEventFrame(args), 1.0f) * 100.0f;
    };
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

}  // namespace

// Reconstructed from eboot.elf at 0x2D8150.
CharAnimCom::CharAnimCom()
    : mPlaybackUnits(0),
      mRelative(kRelativeNone),
      mRelativeAnim(),
      mIsLooping(false),
      mEvents(),
      mAnimSourceFile(),
      mBoneMaskFile() {}

CharAnimCom::CharAnimCom(const CharAnimCom& other)
    : Component(other),
      mPlaybackUnits(other.mPlaybackUnits),
      mRelative(other.mRelative),
      mRelativeAnim(other.mRelativeAnim),
      mIsLooping(other.mIsLooping),
      mEvents(other.mEvents),
      mAnimSourceFile(other.mAnimSourceFile),
      mBoneMaskFile(other.mBoneMaskFile) {}

// Reconstructed from eboot.elf at 0x2D81D0. The deleting destructor is at
// 0x2D82A0.
CharAnimCom::~CharAnimCom() {}

// Reconstructed from eboot.elf at 0x2D8320.
int CharAnimCom::GetRelative(ResourcePath& path) const {
    path = mRelative == kRelativeOther ? mRelativeAnim : ResourcePath();
    return mRelative;
}

// Reconstructed from eboot.elf at 0x2D8350. "relative_anim" is hidden
// unless the animation is relative to another; its file browser starts in
// the folder of the entity's file. The revision handler (0x2DC5B0) fixes
// the sub-object class of "anim_source_file" from revisions 0 and 1 and
// turns the "is_relative" (revision 0) and "self_relative" (revisions 1 and
// 2) flags into "relative". Every property registration builds a "prop"
// symbol it does not use.
void CharAnimCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    CharAnimCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    IntMetadata<int>& relative = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("relative", offsetof(CharAnimCom, mRelative), kPropertyInt, sizeof(int))
             .mMetadata.Get());
    relative.mDefault = prototype.mRelative;
    static_cast<void>(Symbol("prop"));
    relative.mHelp = String("Whether it's played back additively, relative to the bind pose or another file");
    relative.mAllowedValues = CharAnim::GetRelativeAllowedValues();

    ResourcePathMetadata& relativeAnim = TypeSpecificMetadata(
        registry.RegisterProp(
            "relative_anim", offsetof(CharAnimCom, mRelativeAnim), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mRelativeAnim);
    relativeAnim.mDefault = prototype.mRelativeAnim;
    static_cast<void>(Symbol("prop"));
    relativeAnim.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const CharAnimCom&>(args.mComponent).mRelative != kRelativeOther;
    });
    relativeAnim.mHelp = String(
        "Plays back relative to the frame zero of this anim, must have same bone mask, and must be set if "
        "[relative] is other_relative");
    relativeAnim.mResourceId = CharAnimResource::Id();
    relativeAnim.mDefaultExtension._Set([](const TagArgs& args) {
        char folder[512] = {};
        const char* const path = args.mComponent.mObject->mEntity->mResource->mPath.Str();
        return Symbol(FileGetPath(path, folder));
    });

    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(
        *registry
             .RegisterProp("playback_units", offsetof(CharAnimCom, mPlaybackUnits), kPropertyInt, sizeof(int))
             .mMetadata.Get());
    units.mDefault = prototype.mPlaybackUnits;
    static_cast<void>(Symbol("prop"));
    units.mHelp = String("How to play back the animation, which timeline, and whether aligned or not");
    units.mAllowedValues = CharAnim::GetUnitsAllowedValues();

    BoolMetadata& looping = TypeSpecificMetadata(
        registry.RegisterProp("is_looping", offsetof(CharAnimCom, mIsLooping), kPropertyBool, sizeof(bool)),
        prototype.mIsLooping);
    looping.mDefault = prototype.mIsLooping;
    static_cast<void>(Symbol("prop"));
    looping.mHelp =
        String("Should this animation loop by default when played, implies is seamlessly self looping");

    RegisterAnimEvents(registry, Symbol("events"), offsetof(CharAnimCom, mEvents));

    ResourcePathMetadata& boneMask = TypeSpecificMetadata(
        registry.RegisterProp(
            "bone_mask_file", offsetof(CharAnimCom, mBoneMaskFile), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mBoneMaskFile);
    boneMask.mDefault = prototype.mBoneMaskFile;
    static_cast<void>(Symbol("prop"));
    boneMask.mResourceId = CharBoneMaskResource::Id();
    boneMask.mReadOnly = true;

    ResourcePathMetadata& source = TypeSpecificMetadata(
        registry.RegisterProp(
            "anim_source_file", offsetof(CharAnimCom, mAnimSourceFile), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mAnimSourceFile);
    source.mDefault = prototype.mAnimSourceFile;
    static_cast<void>(Symbol("prop"));
    source.mResourceId = CharAnimRawResource::Id();
    source.mReadOnly = true;

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            CharAnimCom* const com = static_cast<CharAnimCom*>(args.mComponent);
            if (args.mSavedRev <= 1) {
                com->mAnimSourceFile.FixSubObjectPath(kSubObjectResource);
            }
            if (args.mSavedRev > 2) {
                return;
            }
            Component& saved = *args.mSavedProps;
            const PropPath path(Symbol(args.mSavedRev > 0 ? "self_relative" : "is_relative"));
            bool selfRelative = false;
            ReadProp(saved, path, selfRelative, kPropertyBool);
            if (selfRelative) {
                com->mRelativeAnim = com->mAnimSourceFile;
                com->mRelative = kRelativeOther;
            } else {
                com->mRelative = com->mRelativeAnim.mPath != Symbol() ? kRelativeOther : kRelativeNone;
            }
        };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(4);
        sPropRegistry.mRevisionFunc = onRevision;
    }

    metadata.mDescription = "Com that contains a static CharAnim and its meta data.";
    metadata.mAuthor = "James Fleming";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(CharAnimResource::Id());
    metadata.mAliases.push_back(Symbol("CharAnimCom"));
}

// Reconstructed from eboot.elf at 0x2D9AB0.
bool CharAnimCom::_OnResourcesLoaded() {
    Resource* const resource = mObject->mEntity->mResource;
    CharAnimResource* const anim =
        resource != nullptr && resource->IsA(CharAnimResource::Id()) ? static_cast<CharAnimResource*>(resource)
                                                                      : nullptr;
    mAnim = anim;
    eastl::sort(mEvents.begin(), mEvents.end(), [](const AnimResource::Event& a, const AnimResource::Event& b) {
        return a.mFrame < b.mFrame;
    });
    return true;
}

// Reconstructed from eboot.elf at 0x2D9BC0.
void CharAnimCom::_Enter() {}

// Reconstructed from eboot.elf at 0x2D9BD0.
void CharAnimCom::_Poll() {}

// Reconstructed from eboot.elf at 0x2D9BE0.
AnimResource* CharAnimCom::GetAnim() const {
    return mAnim;
}

// Reconstructed from eboot.elf at 0x2D9BF0.
unsigned long CharAnimCom::FindPrevEvent(Symbol name, unsigned long index) const {
    while (index != 0) {
        --index;
        if (mEvents[index].mName == name) {
            return index;
        }
    }
    return mEvents.size();
}

// Reconstructed from eboot.elf at 0x2D9C20.
unsigned long CharAnimCom::FindNextEvent(Symbol name, unsigned long index) const {
    for (; index < mEvents.size(); ++index) {
        if (mEvents[index].mName == name) {
            break;
        }
    }
    return index;
}

// Reconstructed from eboot.elf at 0x2D9C60.
Symbol CharAnimCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2D9C70.
Symbol CharAnimCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2D9C80.
int CharAnimCom::CurrentRev() const {
    return const_cast<CharAnimCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2D9CA0.
bool CharAnimCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2D9CD0.
Component* CharAnimCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2D9CE0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CharAnimCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CharAnimCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CharAnimCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2D9E60.
PropRegistry& CharAnimCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2D9E70.
ComMetaData& CharAnimCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2D9E80.
void CharAnimCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x2B5F00. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* CharAnimCom::_Create() {
    return new CharAnimCom();
}

// Reconstructed from eboot.elf at 0x2AE440. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharAnimCom::Init() {
    InitComponentClass<CharAnimCom>();
}
