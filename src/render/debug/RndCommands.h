#pragma once

// Console commands of the renderer. The map's build registers each handler as
// a script function taking the command's DataArray*; this build's handlers
// take no arguments.
class RndCommands {
public:
    // Name not in the reference map.
    using Handler = void (*)();

    // Registers every render command.
    static void Init();  // 0x6BB0E0

    // Not reconstructed yet. The map's signature is
    // _OnToggleOverlay(DataArray*).
    static void _OnToggleOverlay();
    // Not reconstructed yet. The map's signature is
    // _OnPrintOverlayHelp(DataArray*).
    static void _OnPrintOverlayHelp();
    // The map's signature is _OnReloadShaders(DataArray*).
    static void _OnReloadShaders();  // 0x6BA590
    // Not reconstructed yet. Name not in the reference map.
    static void _OnSetResolution();
    // Not reconstructed yet. Name not in the reference map.
    static void _OnSetQualityLevel();
    // The map's signature is _OnToggleVSync(DataArray*).
    static void _OnToggleVSync();  // 0x6BA880
    // The map's signature is _OnToggleSceneMask(DataArray*).
    static void _OnToggleSceneMask();  // 0x6BA8B0
    // Name not in the reference map.
    static void _OnToggleShadows();  // 0x6BA8E0
    // Name not in the reference map.
    static void _OnTogglePostProc();  // 0x6BA910
    // Name not in the reference map.
    static void _OnToggleToneMapping();  // 0x6BA940
    // Name not in the reference map.
    static void _OnToggleVScat();  // 0x6BA970
    // Not reconstructed yet. Name not in the reference map.
    static void _OnSetDrawnSceneRange();
    // Name not in the reference map.
    static void _OnToggleMultithreadedRendering();  // 0x6BAA40
    // The map's signature is _OnToggleAsyncCompute(DataArray*).
    static void _OnToggleAsyncCompute();  // 0x6BAA70
    // Name not in the reference map.
    static void _OnToggleAsyncCopy();  // 0x6BAAA0
    // Name not in the reference map.
    static void _OnToggleTiledLightInterpolation();  // 0x6BAAD0
    // Name not in the reference map.
    static void _OnTogglePartialFramerate();  // 0x6BAB00
    // Name not in the reference map.
    static void _OnToggleStereoOptimizations();  // 0x6BAB40
    // Name not in the reference map.
    static void _OnToggle64BitLightAccum();  // 0x6BAB70
    // Name not in the reference map.
    static void _OnToggleHdr();  // 0x6BABA0
    // Name not in the reference map.
    static void _OnTakeScreenshot();  // 0x6BABD0
    // Name not in the reference map.
    static void _OnCycleScreenshotResolution();  // 0x6BAC00
    // Not reconstructed yet. Name not in the reference map.
    static void _OnSetShadingMode();
    // Not reconstructed yet. The map's signature is
    // _OnSetBufferInspectionMode(DataArray*).
    static void _OnSetBufferInspectionMode();
};

// Registers a console command handler. Not reconstructed yet. Name not in the
// reference map.
void register_debug_command(const char* name, RndCommands::Handler handler);
