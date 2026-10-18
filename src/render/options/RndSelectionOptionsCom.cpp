// render/RndSelectionOptionsCom.o (0x46ACD0 to 0x46B87B).
#include "render/options/RndSelectionOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x46B7B0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndSelectionOptionsCom::sId("SelectionOptions");
Symbol RndSelectionOptionsCom::sClassName("SelectionOptions");
PropRegistry RndSelectionOptionsCom::sPropRegistry;
ComMetaData RndSelectionOptionsCom::sMetaData;
RndSelectionOptionsCom* theRndSelectionOpts;

// Reconstructed from eboot.elf at 0x46ACD0.
void RndSelectionOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndSelectionOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x46ACE0.
bool RndSelectionOptionsCom::_OnResourcesLoaded() {
    theRndSelectionOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x46ACF0.
RndSelectionOptionsCom::RndSelectionOptionsCom()
    : mShowWireframe(true),
      mShowBoundingSpheres(false),
      mShowDistances(false),
      mDistanceLinesColor(Hmx::Color::GetCyan()),
      mOccludedDistanceLinesAlpha(0.25F) {}

// Reconstructed from eboot.elf at 0x46AD80. The deleting destructor is at
// 0x46AD90.
RndSelectionOptionsCom::~RndSelectionOptionsCom() {}

// Reconstructed from eboot.elf at 0x46ADB0. The distance line settings are read-only
// while the lines are off.
void RndSelectionOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndOptionsCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Options governing the appearance of the current selection";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    RndSelectionOptionsCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    BoolMetadata& showWireframe = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_wireframe",
            offsetof(RndSelectionOptionsCom, mShowWireframe),
            kPropertyBool,
            1),
        prototype.mShowWireframe);
    showWireframe.mDefault = prototype.mShowWireframe;
    static_cast<void>(Symbol("prop"));
    showWireframe.mHelp = String("Should selected objects be redrawn in wireframe?");
    BoolMetadata& showBoundingSpheres = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_bounding_spheres",
            offsetof(RndSelectionOptionsCom, mShowBoundingSpheres),
            kPropertyBool,
            1),
        prototype.mShowBoundingSpheres);
    showBoundingSpheres.mDefault = prototype.mShowBoundingSpheres;
    static_cast<void>(Symbol("prop"));
    showBoundingSpheres.mHelp = String("Should the bounding spheres for selected objects be drawn?");
    BoolMetadata& showDistances = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_distances",
            offsetof(RndSelectionOptionsCom, mShowDistances),
            kPropertyBool,
            1),
        prototype.mShowDistances);
    showDistances.mDefault = prototype.mShowDistances;
    static_cast<void>(Symbol("prop"));
    showDistances.mHelp = String("Should we draw distance lines between selected objects?");
    ColorMetadata& lineColor = TypeSpecificMetadata(
        registry.RegisterProp(
            "distance_lines_color",
            offsetof(RndSelectionOptionsCom, mDistanceLinesColor),
            kPropertyColor,
            16),
        prototype.mDistanceLinesColor);
    lineColor.mDefault = prototype.mDistanceLinesColor;
    static_cast<void>(Symbol("prop"));
    lineColor.mHelp = String("Color to draw the distance lines");
    lineColor.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSelectionOptionsCom&>(args.mComponent).mShowDistances;
    });
    FloatMetadata& occludedAlpha = TypeSpecificMetadata(
        registry.RegisterProp(
            "occluded_distance_lines_alpha",
            offsetof(RndSelectionOptionsCom, mOccludedDistanceLinesAlpha),
            kPropertyFloat,
            4),
        prototype.mOccludedDistanceLinesAlpha);
    occludedAlpha.mDefault = prototype.mOccludedDistanceLinesAlpha;
    static_cast<void>(Symbol("prop"));
    occludedAlpha.mHelp = String("Alpha to draw the occluded distance lines");
    occludedAlpha.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSelectionOptionsCom&>(args.mComponent).mShowDistances;
    });
    occludedAlpha.mMin = 0.0F;
    occludedAlpha.mMax = 1.0F;
    occludedAlpha.mSlider = true;
}

// Reconstructed from eboot.elf at 0x46B3C0.
Symbol RndSelectionOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x46B3D0.
Symbol RndSelectionOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x46B3E0.
int RndSelectionOptionsCom::CurrentRev() const {
    return const_cast<RndSelectionOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x46B400.
bool RndSelectionOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x46B430.
Component* RndSelectionOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x46B440. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndSelectionOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSelectionOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSelectionOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x46B590.
PropRegistry& RndSelectionOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x46B5A0.
ComMetaData& RndSelectionOptionsCom::_GetMetaData() {
    return sMetaData;
}
