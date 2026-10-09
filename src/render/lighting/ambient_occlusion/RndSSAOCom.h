#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

class RndBufferCollection;
class RndCameraContext;
class RndContext;
class RndSceneDrawParams;
class RndTexture2D;
enum class RndQualityLevel : std::uint32_t;

// Screen-space ambient occlusion (render/RndSSAOCom.o, 0x4AC310-0x4ADB7F).
// Its class id is "SSAO". The light manager's "ambient_occlusion" object
// holds one, and RndLightMgrCom::GenerateAmbientOcclusion (0x486060) runs
// GenerateAO when the quality level enables it. The vtable at 0x1908190 has
// 41 slots. The object is 88 bytes.
class RndSSAOCom : public Component {
public:
    // One quality level's settings, the map's RndSSAOCom::QualitySettings.
    struct QualitySettings {
        // Inlined into the array's element constructor (0x4AC510).
        QualitySettings() : mEnabled(true) {}

        bool mEnabled;  // "enabled"
    };

    // The map's RndSSAOCom::RuntimeData; its constructor (0x4AC520) clears
    // the texture and its destructor (0x4AC530) is empty. Field name not
    // in the reference map.
    struct RuntimeData {
        RuntimeData() : mNoiseTexture(nullptr) {}

        // 256 by 256 random colors in [0.1, 1), "ssao_noise".
        RndTexture2D* mNoiseTexture;
    };

    RndSSAOCom();  // 0x4AC310
    // Inlined into _Imprint: the runtime data is not copied.
    RndSSAOCom(const RndSSAOCom& other)
        : Component(other),
          mRadius(other.mRadius),
          mAngleBias(other.mAngleBias),
          mIntensity(other.mIntensity) {
        mQualitySettings._Copy(other.mQualitySettings);
    }
    ~RndSSAOCom() override;  // slots 0-1: 0x4AC390, 0x4AC480

    Symbol GetId() const override;         // slot 4: 0x4AD340
    Symbol GetClassName() const override;  // slot 5: 0x4AD350
    int CurrentRev() const override;       // slot 7: 0x4AD360
    bool IsA(Symbol type) const override;  // slot 8: 0x4AD380
    Component* AsComponent() override;     // slot 9: 0x4AD3B0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x4AD3C0
    // Slot 19: one settings entry per quality level.
    void _PostCreate() override;                // 0x4ACEC0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x4AD530
    ComMetaData& _GetMetaData() override;       // slot 23: 0x4AD540
    // Slot 29: creates the noise texture. The map has
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0x4ACED0

    // Whether the quality level generates occlusion. Name not in the
    // reference map.
    bool IsEnabled(RndQualityLevel level) const;  // 0x4AD140
    // Generates the occlusion into the collection's frame-interval AO
    // buffer, limited to the scene mask when the draw uses one. Nothing is
    // drawn into cube targets.
    void GenerateAO(
        RndContext& context,
        RndBufferCollection& buffers,
        RndCameraContext& camera,
        const RndSceneDrawParams& params);  // 0x4AD150

    // Describes the class and registers "quality_settings", "radius",
    // "angleBias" and "intensity". Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x4AC540

    static Symbol sId;  // 0x1A89888, "SSAO"
    // Also "SSAO". Name not in the reference map.
    static Symbol sClassName;           // 0x1A89890
    static PropRegistry sPropRegistry;  // 0x1A898A0
    static ComMetaData sMetaData;       // 0x1A89940

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    // Indexed by RndQualityLevel.
    PropArray<QualitySettings> mQualitySettings;  // "quality_settings"
    // In pixels at 1080 lines.
    float mRadius;     // "radius"
    float mAngleBias;  // "angleBias"
    float mIntensity;  // "intensity"
    RuntimeData mRuntimeData;
};

static_assert(sizeof(RndSSAOCom::QualitySettings) == 1);
static_assert(offsetof(RndSSAOCom, mQualitySettings) == 0x18);
static_assert(offsetof(RndSSAOCom, mRadius) == 0x40);
static_assert(offsetof(RndSSAOCom, mAngleBias) == 0x44);
static_assert(offsetof(RndSSAOCom, mIntensity) == 0x48);
static_assert(offsetof(RndSSAOCom, mRuntimeData) == 0x50);
static_assert(sizeof(RndSSAOCom) == 0x58);
