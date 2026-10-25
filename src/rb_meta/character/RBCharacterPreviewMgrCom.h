#pragma once

// The character preview scene's manager (rb_meta/RBCharacterPreviewMgrCom.o).
// Only what the reconstructed code uses is declared; the class is not
// reconstructed.
class RBCharacterPreviewMgrCom {
public:
    static void ShowPreviewScene();  // 0xB09F30
    static void HidePreviewScene();  // 0xB09FD0

    // The preview's scenes: "ui/overshell/overshell_band.scene" and
    // "ui/overshell/overshell_band_backplates.scene".
    static const char* kCharPreviewScenePath;           // 0x19BE270
    static const char* kCharPreviewBackplateScenePath;  // 0x19BE278
};
