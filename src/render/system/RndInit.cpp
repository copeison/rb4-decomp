#include "render/system/RndInit.h"

#include "os/debug/Debug.h"
#include "render/debug/RndBufferInspection.h"
#include "render/debug/RndDebugFont.h"
#include "render/debug/RndOverlayMgr.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndEditorDrawUtl.h"
#include "render/drawing/RndTexturedQuadCom.h"
#include "render/particles/RndParticleBouncePlaneEditCom.h"
#include "render/queries/RndOcclusionQueryMgr.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

// Reconstructed from eboot.elf at 0x402C30.
void Rnd::Init(const RndInitParams& params) {
    RndInitParams initParams = params;
    RndDevice& device = *PlatformCreateDevice();

    (void)PlatformGfxApi();
    (void)RndGfxApiForPlatform(kPlatformPS4);
    device.Init(initParams);
    device.mDefaults.Init(initParams);
    device.mLighting.LoadResources(initParams);
    RndBufferInspection::Init();
    RndDebugFont::Init();
    RndDrawUtl::Init();
    RndEditorDrawUtl::Init();
    RndOcclusionQueryMgr::Init();
    RndOverlayMgr::Init();
    RndParticleBouncePlaneEditCom::StaticInit();
    RndTexturedQuadCom::PostInit();
    RndDebugFont::InitExtendedFonts();
    TheDebug.AddExitCallback(Terminate);
}

// Reconstructed from eboot.elf at 0x402D30.
void Rnd::Terminate() {
    // Guards against reentry from the exit callbacks. Name not in the
    // reference map.
    static bool sTerminating = false;  // 0x1A71914

    RndDevice* device = TheRndDevice();
    if (device == nullptr || sTerminating) {
        return;
    }

    sTerminating = true;
    RndBufferInspection::Terminate();
    RndDebugFont::Terminate();
    RndDrawUtl::Terminate();
    RndEditorDrawUtl::Terminate();
    RndOcclusionQueryMgr::Terminate();
    RndParticleBouncePlaneEditCom::StaticTerminate();
    RndTexturedQuadCom::Terminate();
    RndDebugFont::TerminateExtendedFonts();
    device->Terminate();
    delete device;
    gRndDevice = nullptr;
    sTerminating = false;
}
