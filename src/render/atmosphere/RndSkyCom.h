#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

class RndBufferCollection;
class RndContext;
class RndTextureBase;
struct RndSceneBatchContext;

// The material-based sky, the scene's background (0x453030-0x453BFF; the
// reference map has no such object, so the class and file names are
// inferred from the class id "Sky"). It needs a RndMaterialCom on its
// object: each frame the material is drawn into the buffer collection's
// atmosphere texture of the chosen resolution, which the fog then fades
// pixels into. The scene component's "sky" property names the object; a
// sky created on the scene's root object becomes the scene's sky when it
// has none. The vtable at 0x1902EF0 has 41 slots. The object is 32 bytes.
// Name not in the reference map.
class RndSkyCom : public Component {
public:
    // The "texture_resolution" choices, relative to the full screen; they
    // index RndBufferCollection::mAtmosphere. Names not in the reference
    // map; they follow the property's labels.
    enum TextureResolution : int {
        kResolutionFull = 0,
        kResolutionHalf = 1,
        kResolutionQuarter = 2,
        kResolutionEighth = 3,
    };

    // Inlined into the class's factory and _Init.
    RndSkyCom() : mTextureResolution(kResolutionHalf) {}
    // Inlined into _Imprint.
    RndSkyCom(const RndSkyCom& other)
        : Component(other), mTextureResolution(other.mTextureResolution) {}
    ~RndSkyCom() override;  // slots 0-1: 0x453030, 0x453040

    Symbol GetId() const override;         // slot 4: 0x453960
    Symbol GetClassName() const override;  // slot 5: 0x453970
    int CurrentRev() const override;       // slot 7: 0x453980
    bool IsA(Symbol type) const override;  // slot 8: 0x4539A0
    Component* AsComponent() override;     // slot 9: 0x4539D0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x4539E0
    // Slot 19: a sky on the scene's root object becomes the scene's sky
    // unless the scene has one.
    void _PostCreate() override;                // 0x4538D0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x453B10
    ComMetaData& _GetMetaData() override;       // slot 23: 0x453B20

    // The collection's atmosphere texture of the sky's resolution.
    RndTextureBase* GetAtmosphereTexture(
        const RndBufferCollection& buffers) const;  // 0x453630
    // Draws the sky's material over the whole atmosphere texture of the
    // batch's collection ("Update Sky Texture"). Name not in the reference
    // map.
    void UpdateTexture(
        RndContext& context,
        const RndSceneBatchContext& batch);  // 0x453640

    // Describes the class, requires RndMaterialCom and registers
    // "texture_resolution". Not reconstructed: the property metadata it
    // fills is not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x453060

    static Symbol sId;  // 0x1A75B20, "Sky"
    // Also "Sky". Name not in the reference map.
    static Symbol sClassName;           // 0x1A75B28
    static PropRegistry sPropRegistry;  // 0x1A75B30
    static ComMetaData sMetaData;       // 0x1A75BD0

    // Field name not in the reference map.
    TextureResolution mTextureResolution;  // "texture_resolution"
};

static_assert(offsetof(RndSkyCom, mTextureResolution) == 0x18);
static_assert(sizeof(RndSkyCom) == 0x20);
