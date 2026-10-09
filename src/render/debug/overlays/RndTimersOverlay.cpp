#include "render/debug/overlays/RndTimersOverlay.h"

namespace {

// Background colors. Names not in the reference map.
const Hmx::Color sUnknown152Color(0.0F, 0.5F, 1.0F, 0.5F);  // 0x1AB1F10
const Hmx::Color sOverBudgetColor(1.0F, 0.0F, 0.0F, 0.8F);  // 0x1AB1F20

}  // namespace

// Reconstructed from eboot.elf at 0x6E7850.
RndTimersOverlay::RndTimersOverlay(const char* name, unsigned int flags)
    : RndOverlayTextBase(name, flags | kFlagKeyboard | kFlagHasHelp | kFlagStripedLines),
      mThreadList(this),
      mUnknown152(false),
      mOverBudget(0.0F) {}

// Reconstructed from eboot.elf at 0x6E78C0 (deleting variant at 0x6E78F0).
RndTimersOverlay::~RndTimersOverlay() {}

// Reconstructed from eboot.elf at 0x6E8400.
void RndTimersOverlay::PrintHelp(TextStream& stream) {
    stream << "Keyboard controls:\n"
              "  Up/Down: navigate tree control\n"
              "  Left:    collapse tree control\n"
              "  Right:   expand tree control\n"
              "  D:       cycle display type\n"
              "  S:       cycle sorting method\n";
}

// Reconstructed from eboot.elf at 0x6E8740. Blends every channel towards
// red by the over-budget amount.
Hmx::Color RndTimersOverlay::_GetBackgroundColor() const {
    float amount = mOverBudget;
    Hmx::Color color;
    if (mUnknown152) {
        amount *= 0.5F;
        color = sUnknown152Color;
    } else {
        color = RndOverlayTextBase::_GetBackgroundColor();
    }
    if (amount > 0.0F) {
        color.red += (sOverBudgetColor.red - color.red) * amount;
        color.green += (sOverBudgetColor.green - color.green) * amount;
        color.blue += (sOverBudgetColor.blue - color.blue) * amount;
        color.alpha += (sOverBudgetColor.alpha - color.alpha) * amount;
    }
    return color;
}
