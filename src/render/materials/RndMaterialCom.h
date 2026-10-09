#pragma once

#include <cstddef>
#include <cstdint>

#include "audio/core/resources/Resource.h"
#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

class GameObject;

// How a material blends into the target. Values confirmed from the material
// blend-mode metadata table. The map names the type
// (RndMaterialCom::SetBlendMode, PS4Context::_SetBlendModeImpl); the
// enumerator names are not in the reference map.
enum class RndBlendMode : std::int32_t {
    kSourceAlpha = 0,
    kSourceAlphaAdd = 1,
    kPremultipliedAlpha = 2,
    kScreen = 3,
    kDestination = 4,
    kSource = 5,
    kAdd = 6,
    kSubtract = 7,
    kMultiply = 8,
    kLighten = 9,
    kDarken = 10,
    kDecalLitSourceAlpha = 11,
};

// Whether a material is shared between the objects that use it. The map
// names the type (RndMaterialCom::SetSharingType); the enumerator names are
// not in the reference map.
enum class RndMaterialSharing : std::int32_t {
    kAutomatic = 0,
    kShared = 1,
    kUnique = 2,
};

// The material component: a shader graph and its render state. Only the
// setters the renderer's defaults call are declared; the small ones are
// reconstructed.
class RndMaterialCom : public Component {
public:
    // The map's signature is SetSharingType(ObjPtr const&,
    // RndMaterialSharing); this build passes the owning object.
    void SetSharingType(const GameObject& owner, RndMaterialSharing sharing);  // 0x4F3870
    void SetShaderGraphFile(const char* file);  // 0x4F3790
    void SetBlendMode(RndBlendMode mode);       // 0x4F3A00

    // The class's second symbol, "Material", stored by the static
    // initializer at 0x4F6574 beside sId (0x1A8B920); the component factory
    // is given it. Name not in the reference map.
    static Symbol sClassName;  // 0x1A8B928

    // Field names are not in the reference map.
    unsigned char mUnknown23[161];
    ResourcePath mShaderGraphFile;
    unsigned char mUnknown192[24];
    RndMaterialSharing mSharing;
    unsigned char mUnknown220[4];
    RndBlendMode mBlendMode;
    unsigned char mUnknown228[28];
    // Cleared by SetSharingType.
    bool mUnknown256;
    // Set when the render state must be rebuilt.
    bool mRenderStateDirty;
};

static_assert(offsetof(RndMaterialCom, mShaderGraphFile) == 184);
static_assert(offsetof(RndMaterialCom, mSharing) == 216);
static_assert(offsetof(RndMaterialCom, mBlendMode) == 224);
static_assert(offsetof(RndMaterialCom, mUnknown256) == 256);
static_assert(offsetof(RndMaterialCom, mRenderStateDirty) == 257);
