// render/RndLightEnvironCom.o (0x478B70 to 0x47A3BB). The registry builder
// (_Init, 0x479650) and the property callbacks after the identity slots are
// not reconstructed.
#include "render/lighting/lights/RndLightEnvironCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "math/color/Color.h"
#include "render/context/RndContext.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/RndLightUtl.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/materials/RndMaterialCom.h"
#include "render/scene/RndSceneCom.h"

namespace {

// The id of no object. The binary reads it from a shared header's global
// (0x1A87B54), which the object's static initializer sets to -1. Name not
// in the reference map.
constexpr unsigned int kNoObject = 0xFFFFFFFFU;

// The debug-misc modes that change how the subtracting lights show. Names
// not in the reference map; the meaning of the modes is not decoded.
constexpr int kDebugMiscHideNegativeLights = 15;
constexpr int kDebugMiscAddNegativeLights = 16;

// The stencil reference of an environment: 0 for the unindexed default,
// otherwise its index plus 2. Inlined into DrawDeferredLightNoCompute. Name
// not in the reference map.
unsigned char EnvironStencil(int index) {
    return index == -1 ? 0 : static_cast<unsigned char>(index + 2);
}

// The light manager of the scene whose root object holds this object's
// entity, or null. Inlined into _Enter and _Exit. Name not in the reference
// map.
RndLightMgrCom* FindLightMgr(const GameObject* object) {
    RndSceneCom* scene = object->mEntity->GetRoot()->GetCom<RndSceneCom>();
    return scene != nullptr ? scene->GetLightMgr() : nullptr;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x47A2F0).
// The three ints it first sets (0x1A87B54) come from a shared header and are
// not modelled.
Symbol RndLightEnvironCom::sId("LightEnviron");
Symbol RndLightEnvironCom::sClassName("LightEnviron");
PropRegistry RndLightEnvironCom::sPropRegistry;
ComMetaData RndLightEnvironCom::sMetaData;

// Reconstructed from eboot.elf at 0x478B70. The run-time state's
// constructor is inlined.
RndLightEnvironCom::RndLightEnvironCom() {}

// Reconstructed from eboot.elf at 0x478D90. The deleting destructor is at
// 0x478DC0.
RndLightEnvironCom::~RndLightEnvironCom() {}

// Reconstructed from eboot.elf at 0x478E00.
void RndLightEnvironCom::SetEnvironIndex(int index) {
    const int previous = mRuntime.mEnvironIndex;
    if (previous == index) {
        return;
    }
    if (previous != kNoEnvironIndex) {
        for (RndLightCom* light : mRuntime.mLights) {
            light->mRuntime.mEnvironBits &= ~(1U << previous);
        }
    }
    mRuntime.mEnvironIndex = index;
    if (index != kNoEnvironIndex) {
        for (RndLightCom* light : mRuntime.mLights) {
            light->mRuntime.mEnvironBits |= 1U << index;
        }
    }
}

// Reconstructed from eboot.elf at 0x478EC0. A light of the environment's own
// entity is a sibling; any other is nested, with its entity kept beside it.
void RndLightEnvironCom::LightAdded(RndLightCom* light) {
    mRuntime.mLights.push_back(light);
    GameObject* lightObject = light->mObject;
    if (mObject->mEntity == lightObject->mEntity) {
        GameObjectId id = lightObject->mId;
        mRuntime.mSiblingLights._Insert(mRuntime.mSiblingLights.size(), &id);
    } else {
        GameObjectId id = lightObject->mId;
        mRuntime.mNestedLights._Insert(mRuntime.mNestedLights.size(), &id);
        mRuntime.mNestedLightEntities.push_back(light->mObject->mEntity);
    }
    light->mRuntime.mEnvironBits |= 1U << (mRuntime.mEnvironIndex & 31);
}

// Reconstructed from eboot.elf at 0x4790E0. As in the binary, a light that
// is not in the list removes the last one, and a missing sibling removes
// the element before the array.
void RndLightEnvironCom::LightRemoved(RndLightCom* light) {
    eastl::vector<RndLightCom*>& lights = mRuntime.mLights;
    RndLightCom** position = lights.mpBegin;
    while (position != lights.mpEnd && *position != light) {
        ++position;
    }
    if (position == lights.mpEnd) {
        position = lights.mpEnd - 1;
    }
    lights.erase(position);

    GameObject* lightObject = light->mObject;
    if (mObject->mEntity == lightObject->mEntity) {
        PropArray<GameObjectId>& siblings = mRuntime.mSiblingLights;
        unsigned long index = static_cast<unsigned long>(-1);
        for (unsigned long i = 0; i < siblings.size(); ++i) {
            if (siblings[i].mId == lightObject->mId.mId) {
                index = i;
                break;
            }
        }
        siblings.Destruct(1, siblings.ElementAt(index));
        const unsigned int size = siblings.size();
        if (static_cast<unsigned int>(index) < size - 1) {
            siblings.Move(
                size - 1 - index,
                siblings.ElementAt(index),
                siblings.ElementAt(index + 1));
        }
        --siblings.mSize;
    } else {
        PropArray<GameObjectId>& nested = mRuntime.mNestedLights;
        Entity* entity = lightObject->mEntity;
        unsigned long index = 0;
        while (index < nested.size() &&
               !(nested[index].mId == lightObject->mId.mId &&
                 mRuntime.mNestedLightEntities[index] == entity)) {
            ++index;
        }
        nested.Destruct(1, nested.ElementAt(index));
        const unsigned int size = nested.size();
        if (static_cast<unsigned int>(index) < size - 1) {
            nested.Move(
                size - 1 - index,
                nested.ElementAt(index),
                nested.ElementAt(index + 1));
        }
        --nested.mSize;
        mRuntime.mNestedLightEntities.erase(mRuntime.mNestedLightEntities.mpBegin + index);
    }
    light->mRuntime.mEnvironBits &= ~(1U << (mRuntime.mEnvironIndex & 31));
}

// Reconstructed from eboot.elf at 0x479300.
void RndLightEnvironCom::ClearCulledLights() {
    mRuntime.mCulledLights.mpEnd = mRuntime.mCulledLights.mpBegin;
    mRuntime.mCulledNegativeLights.mpEnd = mRuntime.mCulledNegativeLights.mpBegin;
    mRuntime.mHasCulledDirectionalLight = false;
}

// Reconstructed from eboot.elf at 0x479330. Only the first directional light
// draws with the probe; the others, and the subtracting lights, get no
// probe. An unindexed environment draws nothing.
void RndLightEnvironCom::DrawDeferredLightNoCompute(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    const RndLightProbeParams& probe) {
    static Symbol name;
    if (name == Symbol()) {
        name = Symbol("Deferred Lighting");
    }
    RndScopedGpuStatBlock stat(ctx, name.Str());
    if (mRuntime.mEnvironIndex == kNoEnvironIndex) {
        return;
    }
    ctx._SetStencilModeImpl(2, EnvironStencil(mRuntime.mEnvironIndex), 1, 0);
    RndLightProbeParams noProbe;
    noProbe.mProbe = nullptr;
    noProbe.mStateIndices[0] = static_cast<unsigned long>(-1);
    noProbe.mStateIndices[1] = static_cast<unsigned long>(-1);
    noProbe.mStateBlend = 0.0F;

    ctx.mBlendMode = RndBlendMode::kAdd;
    ctx._SetBlendModeImpl(RndBlendMode::kAdd, Hmx::Color::GetWhite());
    bool probeUsed = false;
    for (RndLightCom* light : mRuntime.mCulledLights) {
        if (!probeUsed && probe.mProbe != nullptr &&
            light->_GetTypeImpl() == kRndLightDirectional) {
            light->_DrawDeferredNoComputeImpl(ctx, buffers, target, probe);
            probeUsed = true;
        } else {
            light->_DrawDeferredNoComputeImpl(ctx, buffers, target, noProbe);
        }
    }

    bool addNegativeLights = false;
    if (ctx.mShadingMode == kShadingModeDebugMisc) {
        if (ctx.mDebugMiscMode == kDebugMiscHideNegativeLights) {
            return;
        }
        addNegativeLights = ctx.mDebugMiscMode == kDebugMiscAddNegativeLights;
    }
    const RndBlendMode mode =
        addNegativeLights ? RndBlendMode::kAdd : RndBlendMode::kMultiply;
    ctx.mBlendMode = mode;
    ctx._SetBlendModeImpl(mode, Hmx::Color::GetWhite());
    for (RndLightCom* light : mRuntime.mCulledNegativeLights) {
        light->_DrawDeferredNoComputeImpl(ctx, buffers, target, noProbe);
    }
}

// Reconstructed from eboot.elf at 0x479B80.
void RndLightEnvironCom::_Enter() {
    RndLightMgrCom* lightMgr = FindLightMgr(mObject);
    if (lightMgr == nullptr) {
        return;
    }
    mRuntime.mRegistered = true;
    lightMgr->AddEnviron(this);
}

// Reconstructed from eboot.elf at 0x479C70. Only an object or component
// exit leaves the light manager; the binary does not check for a missing
// manager there.
void RndLightEnvironCom::_Exit(DestroyType type) {
    if ((type == kDestroyObject || type == kDestroyComponent) && mRuntime.mRegistered) {
        RndSceneCom* scene = mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
        RndLightMgrCom* lightMgr = scene->GetLightMgr();
        lightMgr->RemoveEnviron(this);
        if (lightMgr->GetDefaultEnviron() == this) {
            lightMgr->SetDefaultEnvironId(GameObjectId{kNoObject});
        }
    }
    mRuntime.mRegistered = false;
    mRuntime.mLights.mpEnd = mRuntime.mLights.mpBegin;
    mRuntime.mSiblingLights.Resize(0);
    mRuntime.mNestedLights.Resize(0);
    mRuntime.mNestedLightEntities.mpEnd = mRuntime.mNestedLightEntities.mpBegin;
}

// Reconstructed from eboot.elf at 0x479D50.
void RndLightEnvironCom::_EditEnter() {
    RndLightMgrCom* lightMgr = FindLightMgr(mObject);
    if (lightMgr == nullptr) {
        return;
    }
    mRuntime.mRegistered = true;
    lightMgr->AddEnviron(this);
}

// Reconstructed from eboot.elf at 0x479DD0.
void RndLightEnvironCom::_EditExit(DestroyType type) {
    _Exit(type);
}

// Reconstructed from eboot.elf at 0x479DE0.
Symbol RndLightEnvironCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x479DF0.
Symbol RndLightEnvironCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x479E00.
int RndLightEnvironCom::CurrentRev() const {
    return const_cast<RndLightEnvironCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x479E20.
bool RndLightEnvironCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x479E50.
Component* RndLightEnvironCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x479E60. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndLightEnvironCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightEnvironCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightEnvironCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x47A080.
PropRegistry& RndLightEnvironCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x47A090.
ComMetaData& RndLightEnvironCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x47A0A0. The members are destroyed in
// reverse order, as the compiler does.
RndLightEnvironCom::RuntimeData::~RuntimeData() {}

// Reconstructed from eboot.elf at 0x404390. The binary emits the factory
// with the other render factories.
Component* RndLightEnvironCom::_Create() {
    return new RndLightEnvironCom();
}
