#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;
enum class RndQualityLevel : std::uint32_t;

// The scene's conservative morphological antialiasing
// (render/RndCMAACom.o, 0x44E9E0-0x45048F). Its class id is
// "AntiAliasing"; the scene component's "antialiasing" property names the
// object that holds it. The scene drawer runs Draw on the light
// accumulation after post-processing, through the four CMAA compute
// shaders, when the quality level enables it. The vtable at 0x19027F0 has
// 41 slots. The object is 72 bytes.
class RndCMAACom : public Component {
public:
    // One quality level's settings, the map's RndCMAACom::QualitySettings.
    struct QualitySettings {
        // Inlined into the array's element constructor (0x44EBA0).
        QualitySettings() : mEnabled(true) {}

        bool mEnabled;  // "enabled"
    };

    RndCMAACom();  // 0x44E9E0
    // Inlined into _Imprint: the settings are copied element by element.
    RndCMAACom(const RndCMAACom& other)
        : Component(other),
          mEdgeThreshold(other.mEdgeThreshold),
          mNonDominantEdgeThreshold(other.mNonDominantEdgeThreshold) {
        mQualitySettings._Copy(other.mQualitySettings);
    }
    ~RndCMAACom() override;  // slots 0-1: 0x44EA50, 0x44EB20

    Symbol GetId() const override;         // slot 4: 0x44FC60
    Symbol GetClassName() const override;  // slot 5: 0x44FC70
    int CurrentRev() const override;       // slot 7: 0x44FC80
    bool IsA(Symbol type) const override;  // slot 8: 0x44FCA0
    Component* AsComponent() override;     // slot 9: 0x44FCD0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x44FCE0
    // Slot 19: one settings entry per quality level.
    void _PostCreate() override;                // 0x44F410
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x44FE40
    ComMetaData& _GetMetaData() override;       // slot 23: 0x44FE50

    // Whether the quality level antialiases. Name not in the reference
    // map.
    bool IsEnabled(RndQualityLevel level) const;  // 0x44F420
    // Antialiases the target's light accumulation in place: detects the
    // edges, prunes them, fits the blend shapes and blends. The first
    // draw of a buffer collection clears its edge buffers; the collection's
    // CMAA state counts the draws and selects the edge buffer each pass
    // reads. Nothing is drawn into cube targets or without compute
    // support. The map's signature is Draw(RndContext&,
    // RndBufferCollection&).
    void Draw(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target);  // 0x44F430

    // Describes the class and registers "quality_settings",
    // "edge_threshold" and "non_dominant_edge_threshold". Not
    // reconstructed: the property metadata it fills is not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x44EBB0

    static Symbol sId;                  // 0x1A75358, "AntiAliasing"
    // Also "AntiAliasing". Name not in the reference map.
    static Symbol sClassName;           // 0x1A75360
    static PropRegistry sPropRegistry;  // 0x1A75370
    static ComMetaData sMetaData;       // 0x1A75410

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    // Indexed by RndQualityLevel.
    PropArray<QualitySettings> mQualitySettings;  // "quality_settings"
    // Lower values antialias more edges.
    float mEdgeThreshold;  // "edge_threshold"
    // Lower values antialias more of the unimportant edges.
    float mNonDominantEdgeThreshold;  // "non_dominant_edge_threshold"
};

static_assert(sizeof(RndCMAACom::QualitySettings) == 1);
static_assert(offsetof(RndCMAACom, mQualitySettings) == 0x18);
static_assert(offsetof(RndCMAACom, mEdgeThreshold) == 0x40);
static_assert(offsetof(RndCMAACom, mNonDominantEdgeThreshold) == 0x44);
static_assert(sizeof(RndCMAACom) == 0x48);
