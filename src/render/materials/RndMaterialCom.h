#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/resources/Resource.h"
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

    // Field names are not in the reference map; the members from 216 take
    // the names of the properties the material's registry (0x4F1C10) binds
    // to their offsets. The constructor is at 0x4F3290.
    // The members of the unmodelled classes between Component and
    // RndMaterialCom: a property array object at 24, a sub-object built at
    // 72, and a secondary base whose vtable pointer is at 160.
    unsigned char mBaseMembers[161];
    ResourcePath mShaderGraphFile;
    // Set to -1 and true by the constructor alongside mShaderGraphFile;
    // not decoded.
    unsigned char mShaderGraphState[8];
    // A sub-object with its own vtable pointer, built by the helper at
    // 0x6C9BA0; not decoded.
    unsigned char mUpdateLink[16];
    RndMaterialSharing mSharing;
    // The "bucket" property.
    std::int32_t mBucket;
    RndBlendMode mBlendMode;
    // "blend_factor", a Hmx::Color that starts white.
    float mBlendFactor[4];
    std::int32_t mCullMode;
    bool mReceiveAtmosphere;
    bool mReceiveDecals;
    bool mDepthPrepass;
    bool mForceOpaque;
    bool mSceneMask;
    bool mUnique;
    // Alignment padding; the constructor does not write it.
    unsigned char mPad[2];
    // Set by the "sharing_type" property's change handler (0x4F4EE0) and
    // cleared once the sharing is applied, by SetSharingType and by the
    // sharing resync at 0x4F38C0.
    bool mSharingDirty;
    // Set when the render state must be rebuilt.
    bool mRenderStateDirty;
};

static_assert(offsetof(RndMaterialCom, mShaderGraphFile) == 184);
static_assert(offsetof(RndMaterialCom, mUpdateLink) == 200);
static_assert(offsetof(RndMaterialCom, mSharing) == 216);
static_assert(offsetof(RndMaterialCom, mBucket) == 220);
static_assert(offsetof(RndMaterialCom, mBlendMode) == 224);
static_assert(offsetof(RndMaterialCom, mBlendFactor) == 228);
static_assert(offsetof(RndMaterialCom, mCullMode) == 244);
static_assert(offsetof(RndMaterialCom, mReceiveAtmosphere) == 248);
static_assert(offsetof(RndMaterialCom, mUnique) == 253);
static_assert(offsetof(RndMaterialCom, mSharingDirty) == 256);
static_assert(offsetof(RndMaterialCom, mRenderStateDirty) == 257);
