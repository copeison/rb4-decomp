// ui/UISliderCom.o (0x8CDAF0 to 0x8CEF9F). The registration and the
// factory are emitted in ui/UI.o.
#include "ui/widgets/UISliderCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndDrawableEntityResource.h"

// The object's statics, in the order of its static initializer (0x8CEEE0).
// The int it first sets to -1 (0x1AC9E08) comes from a shared header and is
// not modelled.
Symbol UISliderCom::sId("UISliderCom");
Symbol UISliderCom::sClassName("UISliderCom");
PropRegistry UISliderCom::sPropRegistry;
ComMetaData UISliderCom::sMetaData;

// Reconstructed from eboot.elf at 0x8CDAF0.
UISliderCom::UISliderCom()
    : mCurrentStep(0),
      mCurrentPct(0.0F),
      mNumSteps(10),
      mDirection(kSliderHorizontal),
      mLocToken() {}

// Reconstructed from eboot.elf at 0x8CE520. The deleting destructor is at
// 0x8CEAF0.
UISliderCom::~UISliderCom() {}

// Reconstructed from eboot.elf at 0x8CDB40. "init_current_step" and
// "current_step" share the step's storage: the instancing object's editor
// sets the first and views the second, and neither is saved. Both are
// limited to the number of steps.
void UISliderCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Generic slider component - use to build a slider, knob, dial, etc.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mInstanceCom = true;
    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);
    metadata.AddExportedEvent(Symbol("slider_step_changed"), String("The slider's step has changed."));
    SetNonStaticTagFunctionsAllowed(nonStatic);

    UISliderCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    IntMetadata<unsigned int>& numSteps = TypeSpecificMetadata(
        registry.RegisterProp("num_steps", offsetof(UISliderCom, mNumSteps), kPropertyUInt, 4),
        prototype.mNumSteps);
    numSteps.mDefault = prototype.mNumSteps;
    static_cast<void>(Symbol("prop"));
    numSteps.mMin = 1U;
    numSteps.mHelp = String("Total number of steps in the slider");
    numSteps.mInstanceAccess = PropMetadata::kInstanceAccessSet;
    numSteps.mDefault = 10U;

    IntMetadata<int>& direction = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "direction", offsetof(UISliderCom, mDirection), kPropertyInt, 4).mMetadata.Get());
    direction.mDefault = prototype.mDirection;
    static_cast<void>(Symbol("prop"));
    direction.mHelp = String("How do we handle button presses, vertically or horizontally?");
    direction.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kSliderHorizontal, String("kSliderHorizontal"), String("")},
        AllowedValue<int>{kSliderVertical, String("kSliderVertical"), String("")}};
    direction.mInstanceAccess = PropMetadata::kInstanceAccessSet;

    SymbolMetadata& locToken = TypeSpecificMetadata(
        registry.RegisterProp("loc_token", offsetof(UISliderCom, mLocToken), kPropertySymbol, 8),
        prototype.mLocToken);
    locToken.mDefault = prototype.mLocToken;
    static_cast<void>(Symbol("prop"));
    locToken.mHelp = String("A generic token property so that slider interfaces can set some text.");
    locToken.mInstanceAccess = PropMetadata::kInstanceAccessSet;

    IntMetadata<unsigned int>& initStep = TypeSpecificMetadata(
        registry.RegisterProp("init_current_step", offsetof(UISliderCom, mCurrentStep), kPropertyUInt, 4),
        prototype.mCurrentStep);
    initStep.mDefault = prototype.mCurrentStep;
    static_cast<void>(Symbol("prop"));
    initStep.mHelp = String(
        "Used by the interface to SET the initial current step because current_step is only a viewable "
        "property.");
    initStep.mSaves = false;
    initStep.mHistory = false;
    initStep.mMin = 0U;
    initStep.mMax._Set([](const TagArgs& args) {
        return static_cast<const UISliderCom&>(args.mComponent).mNumSteps;
    });
    initStep.mInstanceAccess = PropMetadata::kInstanceAccessSet;

    IntMetadata<unsigned int>& currentStep = TypeSpecificMetadata(
        registry.RegisterProp("current_step", offsetof(UISliderCom, mCurrentStep), kPropertyUInt, 4),
        prototype.mCurrentStep);
    currentStep.mDefault = prototype.mCurrentStep;
    static_cast<void>(Symbol("prop"));
    currentStep.mHelp = String("The current step (not saved, used for animating/reading).");
    currentStep.mSaves = false;
    currentStep.mHistory = false;
    currentStep.mMin = 0U;
    currentStep.mMax._Set([](const TagArgs& args) {
        return static_cast<const UISliderCom&>(args.mComponent).mNumSteps;
    });
    currentStep.mInstanceAccess = PropMetadata::kInstanceAccessView;

    FloatMetadata& currentPct = TypeSpecificMetadata(
        registry.RegisterProp("current_pct", offsetof(UISliderCom, mCurrentPct), kPropertyFloat, 4),
        prototype.mCurrentPct);
    currentPct.mDefault = prototype.mCurrentPct;
    static_cast<void>(Symbol("prop"));
    currentPct.mHelp = String(
        "The current step in a 0-1 percentage (not saved, used for animating/reading).");
    currentPct.mSaves = false;
    currentPct.mHistory = false;
    currentPct.mReadOnly = true;
    currentPct.mInstanceAccess = PropMetadata::kInstanceAccessView;
}

// Reconstructed from eboot.elf at 0x8CE530. _UpdateCurrentPercent is
// inlined.
void UISliderCom::SetCurrentStep(unsigned int step) {
    mCurrentStep = step;
    _UpdateCurrentPercent();
}

// Reconstructed from eboot.elf at 0x8CE560.
void UISliderCom::_UpdateCurrentPercent() {
    float percent = 0.0F;
    if (mNumSteps != 1) {
        percent = static_cast<float>(mCurrentStep) / static_cast<float>(mNumSteps - 1);
    }
    mCurrentPct = percent;
}

// Reconstructed from eboot.elf at 0x8CE590.
void UISliderCom::_EditPoll() {
    _UpdateCurrentPercent();
}

// Reconstructed from eboot.elf at 0x8CE5C0.
void UISliderCom::_Poll() {
    _UpdateCurrentPercent();
}

// Reconstructed from eboot.elf at 0x8CEB10.
Symbol UISliderCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8CEB20.
Symbol UISliderCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8CEB30.
int UISliderCom::CurrentRev() const {
    return const_cast<UISliderCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8CEB50.
bool UISliderCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8CEB80.
Component* UISliderCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8CEB90. The copy goes at the buffer's
// next 8-byte boundary.
char* UISliderCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UISliderCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UISliderCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8CECC0.
PropRegistry& UISliderCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8CECD0.
ComMetaData& UISliderCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x895200. The binary emits the factory
// in ui/UI.o.
Component* UISliderCom::_Create() {
    return new UISliderCom();
}

// Reconstructed from eboot.elf at 0x893660. The binary emits the
// registration in ui/UI.o.
void UISliderCom::Init() {
    InitComponentClass<UISliderCom>();
}
