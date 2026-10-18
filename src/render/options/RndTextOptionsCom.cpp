// render/RndTextOptionsCom.o (0x67C560 to 0x67D42B).
#include "render/options/RndTextOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/fonts/RndTextUtl.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x67D360).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndTextOptionsCom::sId("TextOptions");
Symbol RndTextOptionsCom::sClassName("TextOptions");
PropRegistry RndTextOptionsCom::sPropRegistry;
ComMetaData RndTextOptionsCom::sMetaData;
RndTextOptionsCom* theRndTextOpts;

// Reconstructed from eboot.elf at 0x67C560.
void RndTextOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndTextOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x67C570.
bool RndTextOptionsCom::_OnResourcesLoaded() {
    theRndTextOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x67C580.
RndTextOptionsCom::RndTextOptionsCom()
    : mShowSelectedTextBounds(true),
      mShowSelectedTextGlyphBounds(false),
      mTextBoundsColor(Hmx::Color::GetCyan()),
      mDisplayMode(0),
      mAlignment(1),
      mJustification(1),
      mStyleSize(0) {}

// Reconstructed from eboot.elf at 0x67C610. The deleting destructor is at
// 0x67C620.
RndTextOptionsCom::~RndTextOptionsCom() {}

// Reconstructed from eboot.elf at 0x67C640. The "Text" and "FontFamily" structs group
// the component's own members.
void RndTextOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndOptionsCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Options governing the appearance of text";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    RndTextOptionsCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    PropRegistry& text = static_cast<GroupMetadata&>(
        *registry.RegisterProp("Text", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& bounds = TypeSpecificMetadata(
        text.RegisterProp(
            "show_selected_text_bounds",
            offsetof(RndTextOptionsCom, mShowSelectedTextBounds),
            kPropertyBool,
            1),
        prototype.mShowSelectedTextBounds);
    bounds.mDefault = prototype.mShowSelectedTextBounds;
    static_cast<void>(Symbol("prop"));
    bounds.mHelp = String("Should we draw the bounding box for selected text?");
    BoolMetadata& glyphBounds = TypeSpecificMetadata(
        text.RegisterProp(
            "show_selected_text_glyph_bounds",
            offsetof(RndTextOptionsCom, mShowSelectedTextGlyphBounds),
            kPropertyBool,
            1),
        prototype.mShowSelectedTextGlyphBounds);
    glyphBounds.mDefault = prototype.mShowSelectedTextGlyphBounds;
    static_cast<void>(Symbol("prop"));
    glyphBounds.mHelp = String(
        "Should we draw the bounding box of the actual text glyphs for selected text?");
    ColorMetadata& boundsColor = TypeSpecificMetadata(
        text.RegisterProp(
            "text_bounds_color",
            offsetof(RndTextOptionsCom, mTextBoundsColor),
            kPropertyColor,
            16),
        prototype.mTextBoundsColor);
    boundsColor.mDefault = prototype.mTextBoundsColor;
    static_cast<void>(Symbol("prop"));
    boundsColor.mHelp = String("Color of the text bounding box");
    PropRegistry& family = static_cast<GroupMetadata&>(
        *registry.RegisterProp("FontFamily", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    IntMetadata<int>& displayMode = static_cast<IntMetadata<int>&>(*family.RegisterProp(
        "display_mode",
        offsetof(RndTextOptionsCom, mDisplayMode), kPropertyInt, 4).mMetadata.Get());
    displayMode.mDefault = prototype.mDisplayMode;
    static_cast<void>(Symbol("prop"));
    displayMode.mHelp = String("What text should we display for the font family?");
    displayMode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("Spaces between styles"), String("")},
        AllowedValue<int>{1, String("Newlines between styles"), String("")}};
    IntMetadata<int>& alignment = static_cast<IntMetadata<int>&>(*family.RegisterProp(
        "alignment",
        offsetof(RndTextOptionsCom, mAlignment), kPropertyInt, 4).mMetadata.Get());
    alignment.mDefault = prototype.mAlignment;
    static_cast<void>(Symbol("prop"));
    alignment.mHelp = String("Vertical text alignment");
    alignment.mAllowedValues = RndTextUtl::GetAlignmentAllowedValues();
    IntMetadata<int>& justification = static_cast<IntMetadata<int>&>(*family.RegisterProp(
        "justification",
        offsetof(RndTextOptionsCom, mJustification), kPropertyInt, 4).mMetadata.Get());
    justification.mDefault = prototype.mJustification;
    static_cast<void>(Symbol("prop"));
    justification.mHelp = String("Horizontal text justification");
    justification.mAllowedValues = RndTextUtl::GetJustificationAllowedValues();
    IntMetadata<int>& styleSize = static_cast<IntMetadata<int>&>(*family.RegisterProp(
        "style_size",
        offsetof(RndTextOptionsCom, mStyleSize), kPropertyInt, 4).mMetadata.Get());
    styleSize.mDefault = prototype.mStyleSize;
    static_cast<void>(Symbol("prop"));
    styleSize.mHelp = String("Which style size should we display?");
    {
        eastl::vector<AllowedValue<int>> sizes;
        sizes.reserve(kNumFontStyleSizes);
        for (int size = 0; size != kNumFontStyleSizes; ++size) {
            const char* const name = RndTextUtl::GetFontStyleSizeName(static_cast<RndFontStyleSize>(size));
            sizes.push_back(AllowedValue<int>{size, String(name), String("")});
        }
        styleSize.mAllowedValues = static_cast<eastl::vector<AllowedValue<int>>&&>(sizes);
    }
}

// Reconstructed from eboot.elf at 0x67D170.
Symbol RndTextOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x67D180.
Symbol RndTextOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x67D190.
int RndTextOptionsCom::CurrentRev() const {
    return const_cast<RndTextOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x67D1B0.
bool RndTextOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x67D1E0.
Component* RndTextOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x67D1F0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndTextOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndTextOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndTextOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x67D340.
PropRegistry& RndTextOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x67D350.
ComMetaData& RndTextOptionsCom::_GetMetaData() {
    return sMetaData;
}
