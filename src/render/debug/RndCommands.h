#pragma once

#include "utl/data/DataFunc.h"

class RndWindow;

// Console commands of the renderer, registered as script functions. Each
// takes the command's arguments and returns 0.
class RndCommands {
public:
    // Registers every render command.
    static void Init();  // 0x6BB0E0

    // Shows or hides the overlay named by the argument.
    static DataNode _OnToggleOverlay(DataArray* args);  // 0x6BA3D0
    // Prints the help of the named overlay to TheDebug; "all" or no
    // argument prints every overlay that has help.
    static DataNode _OnPrintOverlayHelp(DataArray* args);  // 0x6BA430
    static DataNode _OnReloadShaders(DataArray* args);  // 0x6BA590
    // Overrides the output resolution with a supported one: a width and a
    // height, a "WxH" string, or a 16:9 height. No argument clears the
    // override. Name not in the reference map.
    static DataNode _OnSetResolution(DataArray* args);
    // Sets the quality level by name; an unknown name leaves it unchanged.
    // Name not in the reference map.
    static DataNode _OnSetQualityLevel(DataArray* args);  // 0x6BA730
    static DataNode _OnToggleVSync(DataArray* args);  // 0x6BA880
    static DataNode _OnToggleSceneMask(DataArray* args);  // 0x6BA8B0
    // Name not in the reference map.
    static DataNode _OnToggleShadows(DataArray* args);  // 0x6BA8E0
    // Name not in the reference map.
    static DataNode _OnTogglePostProc(DataArray* args);  // 0x6BA910
    // Name not in the reference map.
    static DataNode _OnToggleToneMapping(DataArray* args);  // 0x6BA940
    // Name not in the reference map.
    static DataNode _OnToggleVScat(DataArray* args);  // 0x6BA970
    // Limits drawing to a range of scenes; no argument draws them all.
    // Name not in the reference map.
    static DataNode _OnSetDrawnSceneRange(DataArray* args);
    // Name not in the reference map.
    static DataNode _OnToggleMultithreadedRendering(DataArray* args);  // 0x6BAA40
    static DataNode _OnToggleAsyncCompute(DataArray* args);  // 0x6BAA70
    // Name not in the reference map.
    static DataNode _OnToggleAsyncCopy(DataArray* args);  // 0x6BAAA0
    // Name not in the reference map.
    static DataNode _OnToggleTiledLightInterpolation(DataArray* args);  // 0x6BAAD0
    // Name not in the reference map.
    static DataNode _OnTogglePartialFramerate(DataArray* args);  // 0x6BAB00
    // Name not in the reference map.
    static DataNode _OnToggleStereoOptimizations(DataArray* args);  // 0x6BAB40
    // Name not in the reference map.
    static DataNode _OnToggle64BitLightAccum(DataArray* args);  // 0x6BAB70
    // Name not in the reference map.
    static DataNode _OnToggleHdr(DataArray* args);  // 0x6BABA0
    // Name not in the reference map.
    static DataNode _OnTakeScreenshot(DataArray* args);  // 0x6BABD0
    // Name not in the reference map.
    static DataNode _OnCycleScreenshotResolution(DataArray* args);  // 0x6BAC00
    // Set the main window's mode by name or number; "help" lists the
    // names. Name of the first not in the reference map.
    static DataNode _OnSetShadingMode(DataArray* args);  // 0x6BAC70
    static DataNode _OnSetBufferInspectionMode(DataArray* args);  // 0x6BAF40

private:
    // Apply a mode argument to a window, returning false for an unknown
    // name. Inlined into the handlers in the map's build; names not in the
    // reference map.
    static bool _SetShadingMode(RndWindow& window, DataArray* args);  // 0x6BACB0
    static bool _SetBufferInspectionMode(RndWindow& window, DataArray* args);  // 0x6BAF80
};
