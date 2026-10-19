// stategraph/StateNodeSetAnimFrame.o (0x78DA30 to 0x7913FF). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeSetAnimFrame.h"

#include <cstring>
#include <new>

#include "anim/char/CharClipCom.h"
#include "anim/char/CharClipResource.h"
#include "anim/core/AnimOptions.h"
#include "anim/core/AnimResource.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "anim/props/PropAnimResource.h"
#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropUtl.h"
#include "entity/resources/ResourceMetaData.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1AB9408, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// Writes the value to the component's property at the path, converted
// from `type`; a failure is reported, and the report is compiled out.
// Inlined into the revision handler. Name not in the reference map.
template <class T>
void SetPropValue(Component& component, const PropPath& path, const T& value, PropertyType type) {
    void* storage = &component;
    const PropInfo* const info = component._GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_SetProp(component, path, *info, storage, &value, type, false)) {
        static_cast<void>(component.MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
}

// The property callbacks _Init installs: a changed animation redraws the
// node. Names not in the reference map.
// Reconstructed from eboot.elf at 0x78F1B0.
void OnClipChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeSetAnimFrame*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x78F1C0.
void OnAnimChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeSetAnimFrame*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x791340).
Symbol StateNodeSetAnimFrame::sId("StateNodeSetAnimFrame");
Symbol StateNodeSetAnimFrame::sClassName("StateNodeSetAnimFrame");
PropRegistry StateNodeSetAnimFrame::sPropRegistry;
ComMetaData StateNodeSetAnimFrame::sMetaData;

// Reconstructed from eboot.elf at 0x78DA30.
StateNodeSetAnimFrame::StateNodeSetAnimFrame()
    : mAnimType(PropAnimResource::Id()), mClip(GameObjectId{0xFFFFFFFFU}), mAnim(), mInstanceName() {}

// Reconstructed from eboot.elf at 0x78DC10. "clip" applies to a
// CharClipResource and "anim" to the other types. Revisions 1 to 4 move
// the older value, the type and the remapping into the base's properties.
void StateNodeSetAnimFrame::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeSetAnimFrame prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&own != &registry) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    SymbolMetadata& animType = TypeSpecificMetadata(
        registry.RegisterProp("anim_type", offsetof(StateNodeSetAnimFrame, mAnimType), kPropertySymbol, 8),
        prototype.mAnimType);
    animType.mDefault = prototype.mAnimType;
    static_cast<void>(Symbol("prop"));
    animType.mHelp = String("The type of animation this node plays.");
    animType.mAllowedValues = GetAnimTypeAllowedValues();

    GameObjectIdMetadata& clip = TypeSpecificMetadata(
        registry.RegisterProp("clip", offsetof(StateNodeSetAnimFrame, mClip), kPropertyGameObjectId, 4),
        prototype.mClip);
    clip.mDefault = prototype.mClip;
    static_cast<void>(Symbol("prop"));
    clip.mHelp = String("The animation this node plays.");
    clip.mRequiredComs = {CharClipCom::sId};
    clip.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    clip.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    // At 0x78F250.
    clip.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeSetAnimFrame&>(args.mComponent).mAnimType != CharClipResource::Id();
    });
    clip.mOnChanged = OnClipChanged;

    ResourcePathMetadata& anim = TypeSpecificMetadata(
        registry.RegisterProp("anim", offsetof(StateNodeSetAnimFrame, mAnim), kPropertyResourcePath, 8),
        prototype.mAnim);
    anim.mDefault = prototype.mAnim;
    static_cast<void>(Symbol("prop"));
    anim.mHelp = String("The animation this node plays.");
    // At 0x78F3F0.
    anim.mResourceId._Set(
        [](const TagArgs& args) { return static_cast<const StateNodeSetAnimFrame&>(args.mComponent).mAnimType; });
    anim.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    anim.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    // At 0x78F4F0.
    anim.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeSetAnimFrame&>(args.mComponent).mAnimType == CharClipResource::Id();
    });
    anim.mOnChanged = OnAnimChanged;

    SymbolMetadata& instanceName = TypeSpecificMetadata(
        registry.RegisterProp(
            "anim_instance_name", offsetof(StateNodeSetAnimFrame, mInstanceName), kPropertySymbol, 8),
        prototype.mInstanceName);
    instanceName.mDefault = prototype.mInstanceName;
    static_cast<void>(Symbol("prop"));
    instanceName.mHelp = String(
        "An optional name by which this particular instance of the animation can be interacted with later on, "
        "by other animation nodes. If not set, this node will spawn a new instance of the animation even if "
        "other instances are already playing.");

    // At 0x78F690.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            StateNodeSetAnimFrame* const node = static_cast<StateNodeSetAnimFrame*>(args.mComponent);
            const Component& saved = *args.mSavedProps;
            // The frame was a plain value.
            if (args.mSavedRev <= 0) {
                node->_LoadOldValue(saved);
            }
            // Revision 2 added the type, taken from the animation's path.
            if (args.mSavedRev <= 1) {
                node->mAnimType = ResourceMetaData::GetIdFromPath(node->mAnim);
            }
            if (args.mSavedRev <= 2 && strcmp(node->mAnimType.Str(), "CharBtClipResource") == 0) {
                node->mAnimType = CharClipResource::Id();
            }
            // The node's own remapping became the base's range conversion.
            if (args.mSavedRev <= 3) {
                if (!GetPropValue<bool>(saved, PropPath(Symbol("remap_value")))) {
                    return;
                }
                PropPath::Node first;
                PropPath::Node second;
                SetPropValue(*node, PropPath(Symbol("conversion")), static_cast<int>(kConversionRange), kPropertyInt);
                const PropPath inputMin(first = Symbol("input_range"), second = Symbol("x"));
                SetPropValue(
                    *node,
                    inputMin,
                    GetPropValue<float>(saved, PropPath(first = Symbol("remap_from"), second = Symbol("x"))),
                    kPropertyFloat);
                const PropPath inputMax(first = Symbol("input_range"), second = Symbol("y"));
                SetPropValue(
                    *node,
                    inputMax,
                    GetPropValue<float>(saved, PropPath(first = Symbol("remap_from"), second = Symbol("y"))),
                    kPropertyFloat);
                const PropPath outputMin(first = Symbol("output_range"), second = Symbol("x"));
                SetPropValue(
                    *node,
                    outputMin,
                    GetPropValue<float>(saved, PropPath(first = Symbol("remap_to"), second = Symbol("x"))),
                    kPropertyFloat);
                const PropPath outputMax(first = Symbol("output_range"), second = Symbol("y"));
                SetPropValue(
                    *node,
                    outputMax,
                    GetPropValue<float>(saved, PropPath(first = Symbol("remap_to"), second = Symbol("y"))),
                    kPropertyFloat);
            }
        };
    if (&own == &registry) {
        own.SetCurrentRev(4);
        own.mRevisionFunc = onRevision;
    }

    StateNodeSetValue<float>::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that instantly applies a specific frame of an animation.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("state_creatable_node")] = Symbol("true");
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("r");
}

// Reconstructed from eboot.elf at 0x78E7A0. Empty without an animation
// name.
const char* StateNodeSetAnimFrame::GetEditorText() const {
    String name;
    if (mAnimType != CharClipResource::Id()) {
        char base[512] = {};
        name = FileGetBase(mAnim.Str(), base);
    } else {
        Entity* const prototype = _GetContextEntityPrototype();
        if (mClip.mId != gNullObjectId.mId) {
            const GameObject* const object = prototype->GetObject(mClip);
            if (object != nullptr) {
                name = object->GetSafeName();
            }
        }
    }
    const char* text = "";
    if (name.c_str()[0] != '\0') {
        const char* const value = _GetValueDescription();
        FormatString description("%s: frame = %s");
        description << name.c_str() << value;
        text = description.Str();
    }
    return text;
}

// Reconstructed from eboot.elf at 0x78EA70.
ResourcePath StateNodeSetAnimFrame::GetAnimPath() const {
    if (mAnimType != CharClipResource::Id()) {
        return mAnim;
    }
    Entity* const prototype = _GetContextEntityPrototype();
    if (mClip.mId != gNullObjectId.mId) {
        const GameObject* const object = prototype->GetObject(mClip);
        if (object != nullptr) {
            const CharClipCom* const com = object->GetCom<CharClipCom>();
            if (com != nullptr) {
                const CharClipResource* const resource = com->mResource.Get();
                if (resource != nullptr && !resource->Fail()) {
                    return resource->mPath;
                }
            }
        }
    }
    return ResourcePath();
}

// Reconstructed from eboot.elf at 0x78EBE0. _CreateInstance inlines it.
StateNodeSetAnimFrame::Instance::Instance(const StateNodeSetAnimFrame& node, StateGraphInstance& graph)
    : StateNodeSetValue<float>::Instance(node, graph), mAnimFrameNode(&node), mAnimId(), mAppliedFrame(1e30F) {}

// Reconstructed from eboot.elf at 0x78EC30. Both play the same animation,
// which must be set.
bool StateNodeSetAnimFrame::Instance::_HasSameTarget(const StateNodeInstance& other) const {
    const ResourcePath path = mAnimFrameNode->GetAnimPath();
    return path != ResourcePath() && path == static_cast<const Instance&>(other).mAnimFrameNode->GetAnimPath();
}

// Reconstructed from eboot.elf at 0x78EC70.
bool StateNodeSetAnimFrame::Instance::_GetCurrentValue(float& value) const {
    AnimDriverCom* const driver = mGraph->mAnimDriver;
    if (driver == nullptr) {
        return false;
    }
    const AnimInstID id = driver->FindInstId(mAnimFrameNode->mInstanceName);
    const PlaybackInfo* const playback = driver->GetPlaybackInfo(id);
    if (playback == nullptr) {
        return false;
    }
    value = playback->GetCurrentFrame();
    return true;
}

// Reconstructed from eboot.elf at 0x78ED00. The playback holds the frame:
// its range begins and ends there, and it does not loop.
void StateNodeSetAnimFrame::Instance::_ApplyValue(const float& value) const {
    const ResourcePath path = mAnimFrameNode->GetAnimPath();
    AnimDriverCom* const driver = mGraph->mAnimDriver;
    if (driver == nullptr || path == ResourcePath() || value == mAppliedFrame) {
        return;
    }
    AnimOptions options;
    options.mTimeRangeEnd = value;
    options.mTimeRangeBegin = value;
    options.mName = mAnimFrameNode->mInstanceName;
    options.mUseFrameTime = true;
    mAnimId = driver->StartAnim(path, options);
    PlaybackInfo* const playback = driver->GetPlaybackInfo(mAnimId);
    if (playback != nullptr) {
        playback->mAutoDestruct = mAnimFrameNode->mInstanceName == Symbol();
        playback->mLooping = false;
    }
    mAppliedFrame = value;
}

// Reconstructed from eboot.elf at 0x78EDF0.
bool StateNodeSetAnimFrame::Instance::_IsFinishedApplyingValue() const {
    const AnimDriverCom* const driver = mGraph->mAnimDriver;
    if (driver == nullptr) {
        return true;
    }
    return !driver->IsAnimating(mAnimId, false);
}

// Reconstructed from eboot.elf at 0x78EEF0.
Symbol StateNodeSetAnimFrame::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x78EF00.
Symbol StateNodeSetAnimFrame::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x78EF10.
int StateNodeSetAnimFrame::CurrentRev() const {
    return const_cast<StateNodeSetAnimFrame*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x78EF30.
bool StateNodeSetAnimFrame::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x78EF60.
Component* StateNodeSetAnimFrame::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x78EF70. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeSetAnimFrame::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeSetAnimFrame*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeSetAnimFrame(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x78F0A0.
PropRegistry& StateNodeSetAnimFrame::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x78F0B0.
ComMetaData& StateNodeSetAnimFrame::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x78F0C0.
StateNodeInstance* StateNodeSetAnimFrame::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x6FFF30. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeSetAnimFrame::_Create() {
    return new StateNodeSetAnimFrame();
}

// Reconstructed from eboot.elf at 0x6F9F10. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeSetAnimFrame::Init() {
    InitComponentClass<StateNodeSetAnimFrame>();
}
