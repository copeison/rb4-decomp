#pragma once

#include <cstddef>
#include <cstdint>

class RndComputeBuffer;
class RndTextureArray2D;

// The scene's light manager: its tiled-light buffers and the spot shadow
// depth array. The functions follow RndLightGlobals in the binary, where the
// map places RndLightMgrCom.o, and the map's RndLightMgrCom::_InitBuffers()
// matches the buffer setup. Only the members these functions use are
// recovered; the leading bytes include the component base. Field names are
// not in the reference map.
class RndLightMgrCom {
public:
    // A spot shadow configuration: the number of shadow layers and the
    // index of their resolution. Name not in the reference map.
    struct SpotShadowConfig {
        std::uint64_t mNumLayers;
        std::uint32_t mResolutionIndex;
        std::uint32_t mUnknown12;
    };

    // Creates the tiled-light buffers when tiled lighting is enabled, then
    // the spot shadow depth array.
    void _InitBuffers();  // 0x48A400
    // Recreates the spot shadow depth array for the active configuration.
    // Name not in the reference map.
    void _SyncSpotShadowDepthTexArray();  // 0x48AB30
    // The tiled-light portion of 0x480AD0. Name not in the reference map.
    void _TerminateBuffers();

    unsigned char mUnknown0[152];
    SpotShadowConfig* mSpotShadowConfigs;
    unsigned char mUnknown160[44];
    std::int32_t mSpotShadowConfigIndex;
    unsigned char mUnknown208[840];
    bool mSpotShadowDepthActive;
    RndTextureArray2D* mSpotShadowDepthTexArray;
    unsigned char mUnknown1064[624];
    RndComputeBuffer* mPointLightBuffer;
    unsigned char mUnknown1696[16];
    RndComputeBuffer* mSpotLightBuffer;
    unsigned char mUnknown1720[16];
    RndComputeBuffer* mDirectionalLightBuffer;
    unsigned char mUnknown1744[16];
    RndComputeBuffer* mLightProbeBuffer;
    RndComputeBuffer* mSliceZeroLightIdBuffer;
};

static_assert(sizeof(RndLightMgrCom::SpotShadowConfig) == 16);
static_assert(offsetof(RndLightMgrCom, mSpotShadowConfigs) == 152);
static_assert(offsetof(RndLightMgrCom, mSpotShadowConfigIndex) == 204);
static_assert(offsetof(RndLightMgrCom, mSpotShadowDepthActive) == 1048);
static_assert(offsetof(RndLightMgrCom, mSpotShadowDepthTexArray) == 1056);
static_assert(offsetof(RndLightMgrCom, mPointLightBuffer) == 1688);
static_assert(offsetof(RndLightMgrCom, mSpotLightBuffer) == 1712);
static_assert(offsetof(RndLightMgrCom, mDirectionalLightBuffer) == 1736);
static_assert(offsetof(RndLightMgrCom, mLightProbeBuffer) == 1760);
static_assert(offsetof(RndLightMgrCom, mSliceZeroLightIdBuffer) == 1768);
