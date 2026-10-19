// anim/PropertyBlenderCom.o (0x2D48E0 to 0x2D814F). The struct type's
// registration (the map's PropArray<PropertyBlenderCom::Configuration>::
// Init(Symbol), 0x2D5710), the element operations of
// PropArray<Configuration> (vtable 0x18F4AF8, 0x2D5EA0 and 0x2D7240-0x2D7390),
// the property accessors and the std::function wrappers of the registry's
// lambdas (vtables 0x18F4B48 and 0x18F4B98, 0x2D6E30-0x2D76FF), the guarded
// initializers of the struct type's names (0x2D6DD0, 0x2D6E00) and the
// templates Resource::GetOrLoad<PropAnimResource> (0x2D7710), EASTL's vector
// swap and growth for the keys and drivers (0x2D7860, 0x2D7B60) and
// Waveform<float>::Evaluate (0x2D7C60) are the compiler's; the templates and
// lambdas model them. The object's static initializer is at 0x2D8090. The
// class's registration and factory are emitted in anim/AnimInit.o. The two
// functions before 0x2D48E0 (0x2D47C0, 0x2D48C0) are called from
// anim/AnimInit.o's templates and belong to it.
#include "anim/props/PropertyBlenderCom.h"

#include <new>

#include "anim/core/AnimOptions.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/PropAnimDriver.h"
#include "anim/props/PropKeysCom.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropMetadata.h"
#include "utl/messages/Message.h"
#include "utl/text/Str.h"

namespace {

// The -1 of a shared header that the static initializer sets (0x19F8100).
// Nothing in this object reads it. Name not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The component a tag or an accessor belongs to. Names not in the
// reference map.
const PropertyBlenderCom& Blender(const TagArgs& args) {
    return static_cast<const PropertyBlenderCom&>(args.mComponent);
}
PropertyBlenderCom& Blender(const PropAccessorArgs& args) {
    return *static_cast<PropertyBlenderCom*>(args.mComponent);
}

// The types of the exported messages, created on first use (local statics
// at 0x19F8368 and 0x19F8378). Names not in the reference map.
Symbol PropBlendStart() {
    static Symbol type;
    if (type == Symbol()) {
        type = Symbol("prop_blend_start");
    }
    return type;
}
Symbol PropBlendDone() {
    static Symbol type;
    if (type == Symbol()) {
        type = Symbol("prop_blend_done");
    }
    return type;
}

// Exports the message to the component's entity with the configuration the
// blender came from and the one it goes to. Inlined twice into _Poll. Name
// not in the reference map.
void ExportBlendMessage(const PropertyBlenderCom& blender, Symbol type) {
    Message<2> msg(type);
    msg.mData->Node(2) = DataNode(blender.mPrevInfo.mConfiguration.mName);
    msg.mData->Node(3) = DataNode(blender.mCurrentInfo.mConfiguration.mName);
    blender.mObject->mEntity->Export(msg.mData);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x2D8090).
Symbol PropertyBlenderCom::sId("PropertyBlenderCom");
Symbol PropertyBlenderCom::sClassName("PropertyBlenderCom");
PropRegistry PropertyBlenderCom::sPropRegistry;
ComMetaData PropertyBlenderCom::sMetaData;

// Reconstructed from eboot.elf at 0x2D48E0. The registry stays at revision
// 0. The event parameters are registered with the non-static tag functions
// disallowed.
void PropertyBlenderCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<Configuration>(Symbol("PropertyBlenderCom::Configuration"));
    PropertyBlenderCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    metadata.mDescription = "Used to blend between arbitrary PropAnim frames by name.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());

    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);
    PropRegistry& blendStart = metadata.AddExportedEvent(
        Symbol("prop_blend_start"), String("A PropertyBlenderCom is about to start blending."));
    const Symbol startCurrentType;
    SymbolMetadata& startCurrent =
        TypeSpecificMetadata(blendStart.RegisterProp("current_configuration", -1, kPropertySymbol, 0), startCurrentType);
    static_cast<void>(Symbol("param"));
    startCurrent.mHelp = String("The configuration the blender is currently at, before starting to blend.");
    const Symbol startNewType;
    SymbolMetadata& startNew =
        TypeSpecificMetadata(blendStart.RegisterProp("new_configuration", -1, kPropertySymbol, 0), startNewType);
    static_cast<void>(Symbol("param"));
    startNew.mHelp = String("The configuration the blender is about to blend to.");
    PropRegistry& blendDone = metadata.AddExportedEvent(
        Symbol("prop_blend_done"), String("A PropertyBlenderCom has just finished blending."));
    const Symbol donePreviousType;
    SymbolMetadata& donePrevious = TypeSpecificMetadata(
        blendDone.RegisterProp("previous_configuration", -1, kPropertySymbol, 0), donePreviousType);
    static_cast<void>(Symbol("param"));
    donePrevious.mHelp = String("The configuration the blender was at before the blend that just finished.");
    const Symbol doneNewType;
    SymbolMetadata& doneNew =
        TypeSpecificMetadata(blendDone.RegisterProp("new_configuration", -1, kPropertySymbol, 0), doneNewType);
    static_cast<void>(Symbol("param"));
    doneNew.mHelp = String("The configuration that the blender has just finished blending to.");
    SetNonStaticTagFunctionsAllowed(nonStatic);

    ResourcePathMetadata& anim = TypeSpecificMetadata(
        registry.RegisterProp("anim", offsetof(PropertyBlenderCom, mAnim), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mAnim);
    anim.mDefault = prototype.mAnim;
    static_cast<void>(Symbol("prop"));
    anim.mHelp = String("The propanim our configurations will apply to.");
    anim.mResourceId = PropAnimResource::Id();

    FloatMetadata& duration = TypeSpecificMetadata(
        registry.RegisterProp("duration", offsetof(PropertyBlenderCom, mDuration), kPropertyFloat, sizeof(float)),
        prototype.mDuration);
    duration.mDefault = prototype.mDuration;
    static_cast<void>(Symbol("prop"));
    duration.mHelp = String("The length of time this blend takes to complete.");
    duration.mMin = 0.0F;

    FloatWaveformMetadata& blendCurve = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
        registry.RegisterProp(
            "blend_curve",
            offsetof(PropertyBlenderCom, mBlendCurve),
            kPropertyWaveformFloat,
            sizeof(Waveform<float>)),
        prototype.mBlendCurve));
    blendCurve.mDefault = Waveform<float>(prototype.mBlendCurve);
    static_cast<void>(Symbol("prop"));
    blendCurve.mHelp = String("The blend curve we should use when blending between anim frames.");
    // Without a duration the blend jumps, so the curve is hidden.
    blendCurve.mIsHidden._Set([](const TagArgs& args) { return Blender(args).mDuration == 0.0F; });
    blendCurve.mMin = 0.0F;
    blendCurve.mMax = 1.0F;
    // The binary assigns the list through a shared helper (0x2D58C0).
    blendCurve.mAllowedTypes = {2};

    PropInfo& configurations = registry.RegisterPropArray(
        "configurations", offsetof(PropertyBlenderCom, mConfigurations), sizeof(PropArray<Configuration>));
    ArrayMetadata& configurationsMetadata = static_cast<ArrayMetadata&>(*configurations.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(configurations, kPropertyStruct, sizeof(Configuration));
    PropRegistry& members =
        static_cast<GroupMetadata&>(*configurationsMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const Configuration configurationPrototype;
    SymbolMetadata& name = TypeSpecificMetadata(
        members.RegisterProp("name", offsetof(Configuration, mName), kPropertySymbol, sizeof(Symbol)),
        configurationPrototype.mName);
    name.mDefault = configurationPrototype.mName;
    static_cast<void>(Symbol("prop"));
    name.mHelp = String("The name of the configuration.");
    FloatMetadata& frame = TypeSpecificMetadata(
        members.RegisterProp("frame", offsetof(Configuration, mFrame), kPropertyFloat, sizeof(float)),
        configurationPrototype.mFrame);
    frame.mDefault = configurationPrototype.mFrame;
    static_cast<void>(Symbol("prop"));
    frame.mHelp = String("The frame of the propanim this configuration should blend to.");

    // The current configuration's name, set through SetConfiguration and
    // chosen from the configurations' names. Not kept in the history.
    const Symbol currentType;
    SymbolMetadata& current =
        TypeSpecificMetadata(registry.RegisterProp("current_configuration", -1, kPropertySymbol, 0), currentType);
    static_cast<void>(Symbol("prop"));
    current.mGetter = [](const PropAccessorArgs& args) { return Blender(args).mCurrentInfo.mConfiguration.mName; };
    current.mSetter = [](const PropAccessorArgs& args) {
        Blender(args).SetConfiguration(*static_cast<const Symbol*>(args.mValue));
    };
    current.mAllowedValues._Set([](const TagArgs& args) {
        const PropertyBlenderCom& blender = Blender(args);
        eastl::vector<AllowedValue<Symbol>> values;
        values.reserve(blender.mConfigurations.size());
        for (unsigned long index = 0; index < blender.mConfigurations.size(); ++index) {
            const Symbol configuration = blender.mConfigurations[index].mName;
            values.push_back(AllowedValue<Symbol>{configuration, String(configuration.Str()), String("")});
        }
        return values;
    });
    current.mHistory = false;

    // The current configuration's frame. Setting it makes the frame the
    // current configuration under the name "_custom_"; a previous custom
    // configuration is renamed "_old_custom_" so that the blend still
    // starts.
    const float targetFrameType = 0.0F;
    FloatMetadata& targetFrame =
        TypeSpecificMetadata(registry.RegisterProp("target_frame", -1, kPropertyFloat, 0), targetFrameType);
    static_cast<void>(Symbol("prop"));
    targetFrame.mHelp = String(
        "Represents the frame of the current configuration. If you set this property directly, this PropertyBlender "
        "will blend directly to the specified target frame, treating it as a custom, unnamed configuration.");
    targetFrame.mGetter = [](const PropAccessorArgs& args) {
        return Blender(args).mCurrentInfo.mConfiguration.mFrame;
    };
    targetFrame.mSetter = [](const PropAccessorArgs& args) {
        static Symbol custom;
        if (custom == Symbol()) {
            custom = Symbol("_custom_");
        }
        PropertyBlenderCom& blender = Blender(args);
        if (blender.mPrevInfo.mConfiguration.mName == custom) {
            static Symbol oldCustom;
            if (oldCustom == Symbol()) {
                oldCustom = Symbol("_old_custom_");
            }
            blender.mPrevInfo.mConfiguration.mName = oldCustom;
        }
        blender.mCurrentInfo.mConfiguration.mName = custom;
        blender.mCurrentInfo.mConfiguration.mFrame = *static_cast<const float*>(args.mValue);
    };

    BoolMetadata& enabled = TypeSpecificMetadata(
        registry.RegisterProp("is_enabled", offsetof(PropertyBlenderCom, mEnabled), kPropertyBool, sizeof(bool)),
        prototype.mEnabled);
    enabled.mDefault = prototype.mEnabled;
    static_cast<void>(Symbol("prop"));
    enabled.mDefault = true;
    enabled.mHelp = String(
        "When enabled, the PropertyBlender will behave as you'd expect. When disabled, the PropertyBlender will not "
        "respond to configuration changes and will stop setting properties every frame. This is particularly useful "
        "when you want something else to modify properties without this PropertyBlender immediately clobbering those "
        "changes.");
    enabled.mSaves = false;
    enabled.mHistory = false;

    // Whether the playbacks play, or the configuration changed and the
    // next poll starts a blend.
    const bool blendingType = false;
    BoolMetadata& blending =
        TypeSpecificMetadata(registry.RegisterProp("is_currently_blending", -1, kPropertyBool, 0), blendingType);
    static_cast<void>(Symbol("prop"));
    blending.mHelp = String(
        "Is this PropertyBlender actively interpolating properties toward a target configuration at this time?");
    blending.mReadOnly = true;
    blending.mGetter = [](const PropAccessorArgs& args) {
        const PropertyBlenderCom& blender = Blender(args);
        return blender.mCurrentInfo.mPlaying
            || blender.mCurrentInfo.mConfiguration.mName != blender.mPrevInfo.mConfiguration.mName;
    };

    BoolMetadata& forceBlend = TypeSpecificMetadata(
        registry.RegisterProp("force_blend", offsetof(PropertyBlenderCom, mForceBlend), kPropertyBool, sizeof(bool)),
        prototype.mForceBlend);
    forceBlend.mDefault = prototype.mForceBlend;
    static_cast<void>(Symbol("prop"));
    forceBlend.mDisplayName = String("force blend");
    forceBlend.mDefault = false;
    forceBlend.mHelp = String(
        "Set this flag to true if you want to force the prop blender to re-blend to the current configuration, even "
        "if the configuration hasn't changed. It will re-set itself to false on the next poll, but the blend will "
        "take the normal duration.");
    forceBlend.mSaves = false;
    forceBlend.mHistory = false;
}

// Reconstructed from eboot.elf at 0x2D5A30. The curve starts as the
// default float waveform.
PropertyBlenderCom::PropertyBlenderCom()
    : mAnimResource(),
      mSourceKeys(),
      mBlendAnim(),
      mBlendKeys(),
      mDrivers(),
      mCurrentInfo{Configuration(), nullptr, false},
      mPrevInfo{Configuration(), nullptr, false},
      mElapsed(0.0F),
      mEnabled(true),
      mForceBlend(false),
      mClock(nullptr),
      mAnim(),
      mBlendCurve(GetDefaultFloatWaveformResource().Get()),
      mDuration(0.0F),
      mConfigurations() {}

// Reconstructed from eboot.elf at 0x2D5CF0. The deleting destructor is at
// 0x2D5F00.
PropertyBlenderCom::~PropertyBlenderCom() {
    for (PropDriver* driver : mDrivers) {
        delete driver;
    }
    mDrivers.clear();
    delete mPrevInfo.mPlayback;
    mPrevInfo.mPlayback = nullptr;
    delete mCurrentInfo.mPlayback;
    mCurrentInfo.mPlayback = nullptr;
}

// Reconstructed from eboot.elf at 0x2D5F20.
void PropertyBlenderCom::SetConfiguration(Symbol name) {
    for (unsigned long index = 0; index < mConfigurations.size(); ++index) {
        if (mConfigurations[index].mName == name) {
            mCurrentInfo.mConfiguration = mConfigurations[index];
            return;
        }
    }
    static_cast<void>(MakeErrorName());
}

// Reconstructed from eboot.elf at 0x2D5F70. The blend animation's entity
// is created empty; its keys are added once "anim" is ready.
bool PropertyBlenderCom::_OnResourcesLoaded() {
    mClock = TheTimeMgr->GetClock(mObject->mEntity);
    ResourcePtr<PropAnimResource> anim = mObject->mEntity->mResource->TryGetInline<PropAnimResource>(mAnim);
    if (anim == nullptr) {
        anim = Resource::GetOrLoad<PropAnimResource>(mAnim, false);
    }
    mAnimResource = static_cast<ResourcePtr<PropAnimResource>&&>(anim);
    mBlendAnim = new PropAnimResource();
    mBlendAnim->CreateEntity();
    mBlendAnim->SetUnits(0);
    return true;
}

// Reconstructed from eboot.elf at 0x2D6100. Both playbacks start on the
// animation's timeline; the blend's playback plays the blend animation.
bool PropertyBlenderCom::_AreResourcesReady(int pass) {
    if (pass != 0 || mAnimResource == nullptr || mAnimResource->Fail()) {
        return true;
    }
    mAnimResource->FixupStaleIds();
    // The binary move-assigns the keys (EASTL's swap, 0x2D7860).
    mBlendKeys = mBlendAnim->AddKeys(mAnimResource);
    mAnimResource->HarvestAllKeys(mSourceKeys, true);
    mDrivers.resize(mSourceKeys.size());
    for (unsigned long index = 0; index < mDrivers.size(); ++index) {
        mDrivers[index] = mSourceKeys[index]->CreatePropDriver();
    }
    AnimOptions options;
    options.mTimeline = mAnimResource->GetUnits();
    mPrevInfo.mPlayback = new PlaybackInfo(&static_cast<AnimResource&>(*mBlendAnim), 0, options, mClock);
    mCurrentInfo.mPlayback = new PlaybackInfo(&static_cast<AnimResource&>(*mAnimResource), 0, options, mClock);
    return true;
}

// Reconstructed from eboot.elf at 0x2D6350. A blend starts by setting the
// blend animation's first key to the properties' current values and holding
// both playbacks on their frames; the curve then moves the weight from the
// blend's playback to the configuration's.
void PropertyBlenderCom::_Poll() {
    if (mAnimResource == nullptr || mAnimResource->Fail() || !mEnabled) {
        return;
    }
    const TimeUnits units = mAnimResource->GetUnits();
    if (mPrevInfo.mConfiguration.mName != mCurrentInfo.mConfiguration.mName || mForceBlend) {
        mForceBlend = false;
        ExportBlendMessage(*this, PropBlendStart());
        _RemoveCurrentAndPrevInfos();
        Entity* const entity = mObject->mEntity;
        for (PropKeysBaseCom* keys : mBlendKeys) {
            keys->UpdateValueAtIndex(0, entity);
        }
        AnimOptions from;
        from.mTimeRangeBegin = 0.0F;
        from.mTimeRangeEnd = 0.0F;
        from.mUseFrameTime = true;
        mPrevInfo.mPlayback->SetOptions(from);
        AnimOptions to;
        to.mTimeRangeBegin = mCurrentInfo.mConfiguration.mFrame;
        to.mTimeRangeEnd = mCurrentInfo.mConfiguration.mFrame;
        to.mUseFrameTime = true;
        mCurrentInfo.mPlayback->SetOptions(to);
        for (unsigned long index = 0; index < mDrivers.size(); ++index) {
            mDrivers[index]->Add(mPrevInfo.mPlayback, mBlendKeys[index], mObject->mEntity);
            mDrivers[index]->Add(mCurrentInfo.mPlayback, mSourceKeys[index], mObject->mEntity);
        }
        mPrevInfo.mConfiguration.mName = mCurrentInfo.mConfiguration.mName;
        mElapsed = 0.0F;
        mPrevInfo.mPlaying = true;
        mCurrentInfo.mPlaying = true;
    }
    mElapsed += mClock->DeltaTime(units);
    if (mPrevInfo.mPlayback != nullptr && mCurrentInfo.mPlayback != nullptr) {
        if (mDuration == 0.0F) {
            mCurrentInfo.mPlayback->mWeight = 1.0F;
            mPrevInfo.mPlayback->mWeight = 0.0F;
        } else {
            // The elapsed time is clamped to the duration, which may be
            // negative.
            const float low = mDuration < 0.0F ? mDuration : 0.0F;
            const float high = mDuration > 0.0F ? mDuration : 0.0F;
            const float elapsed = mElapsed > high ? high : (mElapsed < low ? low : mElapsed);
            const float weight = mBlendCurve.Evaluate(elapsed / mDuration);
            mPrevInfo.mPlayback->mWeight = 1.0F - weight;
            mCurrentInfo.mPlayback->mWeight = weight;
        }
    }
    for (PropDriver* driver : mDrivers) {
        driver->Poll(mObject);
    }
    if (!(mCurrentInfo.mPlayback->mWeight < 1.0F) && mPrevInfo.mPlaying && mCurrentInfo.mPlaying) {
        ExportBlendMessage(*this, PropBlendDone());
        _RemoveCurrentAndPrevInfos();
    }
}

// Reconstructed from eboot.elf at 0x2D6BA0.
void PropertyBlenderCom::_RemoveCurrentAndPrevInfos() {
    if (!mPrevInfo.mPlaying || !mCurrentInfo.mPlaying) {
        return;
    }
    for (PropDriver* driver : mDrivers) {
        driver->Remove(mPrevInfo.mPlayback);
        driver->Remove(mCurrentInfo.mPlayback);
    }
    mPrevInfo.mPlaying = false;
    mCurrentInfo.mPlaying = false;
}

// Reconstructed from eboot.elf at 0x2D6C20.
Symbol PropertyBlenderCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2D6C30.
Symbol PropertyBlenderCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2D6C40.
int PropertyBlenderCom::CurrentRev() const {
    return const_cast<PropertyBlenderCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2D6C60.
bool PropertyBlenderCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2D6C90.
Component* PropertyBlenderCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2D6CA0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropertyBlenderCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropertyBlenderCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropertyBlenderCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2D6DB0.
PropRegistry& PropertyBlenderCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2D6DC0.
ComMetaData& PropertyBlenderCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2D7000. The curve shares the other's
// resources with a new evaluation state.
PropertyBlenderCom::PropertyBlenderCom(const PropertyBlenderCom& other)
    : Component(other),
      mAnimResource(),
      mSourceKeys(),
      mBlendAnim(),
      mBlendKeys(),
      mDrivers(),
      mCurrentInfo{Configuration(), nullptr, false},
      mPrevInfo{Configuration(), nullptr, false},
      mElapsed(0.0F),
      mEnabled(true),
      mForceBlend(false),
      mClock(nullptr),
      mAnim(other.mAnim),
      mBlendCurve(other.mBlendCurve),
      mDuration(other.mDuration),
      mConfigurations(other.mConfigurations) {}

// Reconstructed from eboot.elf at 0x2B7320. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropertyBlenderCom::_Create() {
    return new PropertyBlenderCom();
}

// Reconstructed from eboot.elf at 0x2AF8E0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropertyBlenderCom::Init() {
    InitComponentClass<PropertyBlenderCom>();
}
