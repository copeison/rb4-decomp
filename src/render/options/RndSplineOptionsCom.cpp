// render/RndSplineOptionsCom.o (0x659180 to 0x65A5BB).
#include "render/options/RndSplineOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/drawing/RndEditorDrawUtl.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x65A4F0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndSplineOptionsCom::sId("SplineOptions");
Symbol RndSplineOptionsCom::sClassName("SplineOptions");
PropRegistry RndSplineOptionsCom::sPropRegistry;
ComMetaData RndSplineOptionsCom::sMetaData;
RndSplineOptionsCom* theRndSplineOpts;

// Reconstructed from eboot.elf at 0x659180.
void RndSplineOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndSplineOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x659190.
bool RndSplineOptionsCom::_OnResourcesLoaded() {
    theRndSplineOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x6591A0.
RndSplineOptionsCom::RndSplineOptionsCom()
    : mShowSplines(true),
      mShowSelectedSplineControlPoints(true),
      mShowSelectedSplineControlPointsIndices(false),
      mShowSplineHulls(1),
      mShowSelectedSplineContourPoints(true),
      mShowSelectedSplineContourPointsIndices(false),
      mDepthTestSplines(true) {}

// Reconstructed from eboot.elf at 0x6591E0. The deleting destructor is at
// 0x6591F0.
RndSplineOptionsCom::~RndSplineOptionsCom() {}

// Reconstructed from eboot.elf at 0x659210. Everything but "show_splines" is read-only
// while splines are hidden.
void RndSplineOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndOptionsCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Options governing the appearance of splines";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    RndSplineOptionsCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    BoolMetadata& showSplines = TypeSpecificMetadata(
        registry.RegisterProp("show_splines", offsetof(RndSplineOptionsCom, mShowSplines), kPropertyBool, 1),
        prototype.mShowSplines);
    showSplines.mDefault = prototype.mShowSplines;
    static_cast<void>(Symbol("prop"));
    showSplines.mHelp = String("Should splines be drawn?");
    BoolMetadata& controlPoints = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_spline_control_points",
            offsetof(RndSplineOptionsCom, mShowSelectedSplineControlPoints),
            kPropertyBool,
            1),
        prototype.mShowSelectedSplineControlPoints);
    controlPoints.mDefault = prototype.mShowSelectedSplineControlPoints;
    static_cast<void>(Symbol("prop"));
    controlPoints.mHelp = String("Should the control points of selected splines be drawn?");
    controlPoints.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSplineOptionsCom&>(args.mComponent).mShowSplines;
    });
    BoolMetadata& controlPointIndices = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_spline_control_points_indices",
            offsetof(RndSplineOptionsCom, mShowSelectedSplineControlPointsIndices),
            kPropertyBool,
            1),
        prototype.mShowSelectedSplineControlPointsIndices);
    controlPointIndices.mDefault = prototype.mShowSelectedSplineControlPointsIndices;
    static_cast<void>(Symbol("prop"));
    controlPointIndices.mHelp = String("Should the control point indices of selected splines be drawn?");
    controlPointIndices.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSplineOptionsCom&>(args.mComponent).mShowSplines;
    });
    IntMetadata<int>& hulls = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "show_spline_hulls",
        offsetof(RndSplineOptionsCom, mShowSplineHulls), kPropertyInt, 4).mMetadata.Get());
    hulls.mDefault = prototype.mShowSplineHulls;
    static_cast<void>(Symbol("prop"));
    hulls.mHelp = String("Should the hulls of splines be drawn?");
    hulls.mAllowedValues = RndEditorDrawUtl::GetDrawWhenAllowedValues();
    hulls.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSplineOptionsCom&>(args.mComponent).mShowSplines;
    });
    BoolMetadata& contourPoints = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_spline_contour_points",
            offsetof(RndSplineOptionsCom, mShowSelectedSplineContourPoints),
            kPropertyBool,
            1),
        prototype.mShowSelectedSplineContourPoints);
    contourPoints.mDefault = prototype.mShowSelectedSplineContourPoints;
    static_cast<void>(Symbol("prop"));
    contourPoints.mHelp = String("Should the contour points of selected splines be drawn?");
    contourPoints.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSplineOptionsCom&>(args.mComponent).mShowSplines;
    });
    BoolMetadata& contourPointIndices = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_spline_contour_points_indices",
            offsetof(RndSplineOptionsCom, mShowSelectedSplineContourPointsIndices),
            kPropertyBool,
            1),
        prototype.mShowSelectedSplineContourPointsIndices);
    contourPointIndices.mDefault = prototype.mShowSelectedSplineContourPointsIndices;
    static_cast<void>(Symbol("prop"));
    contourPointIndices.mHelp = String("Should the contour point indices of selected splines be drawn?");
    contourPointIndices.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSplineOptionsCom&>(args.mComponent).mShowSplines;
    });
    BoolMetadata& depthTest = TypeSpecificMetadata(
        registry.RegisterProp(
            "depth_test_splines",
            offsetof(RndSplineOptionsCom, mDepthTestSplines),
            kPropertyBool,
            1),
        prototype.mDepthTestSplines);
    depthTest.mDefault = prototype.mDepthTestSplines;
    static_cast<void>(Symbol("prop"));
    depthTest.mHelp = String("Should splines test/write depth?");
    depthTest.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndSplineOptionsCom&>(args.mComponent).mShowSplines;
    });
}

// Reconstructed from eboot.elf at 0x659D00.
Symbol RndSplineOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x659D10.
Symbol RndSplineOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x659D20.
int RndSplineOptionsCom::CurrentRev() const {
    return const_cast<RndSplineOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x659D40.
bool RndSplineOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x659D70.
Component* RndSplineOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x659D80. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndSplineOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSplineOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSplineOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x659ED0.
PropRegistry& RndSplineOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x659EE0.
ComMetaData& RndSplineOptionsCom::_GetMetaData() {
    return sMetaData;
}
