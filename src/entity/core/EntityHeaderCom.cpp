// The entity header component: an object the map does not name (0x1D1E90 to
// 0x1D8D37). The registry (0x1D21D0), its handlers and the per-layer
// accessors (0x1D52E0-0x1D57CF) are not reconstructed.
#include "entity/core/EntityHeaderCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/core/TransEntityResource.h"

// The object's statics, in the order of its static initializer (0x1D8C80).
// The int it first sets (0x19E4A78) comes from a shared header and is not
// modelled.
Symbol EntityHeaderCom::sId("EntityHeader");
Symbol EntityHeaderCom::sClassName("EntityHeader");
PropRegistry EntityHeaderCom::sPropRegistry;
ComMetaData EntityHeaderCom::sMetaData;

// Reconstructed from eboot.elf at 0x114E20, in entity/Utl.o. Every root
// must hold an EntityHeaderCom.
bool IsStaticEntity(const Entity* entity) {
    for (;;) {
        if (entity->GetRoot()->GetExistingCom<EntityHeaderCom>()->mStatic) {
            return true;
        }
        if (entity->mParentObject == nullptr) {
            return false;
        }
        entity = entity->GetParent();
    }
}

// Reconstructed from eboot.elf at 0x1D1E90.
EntityHeaderCom::EntityHeaderCom()
    : mNumInstances(0),
      mNumMeshes(0),
      mNumParticles(0),
      mNumPropanims(0),
      mNumLights(0),
      mNumVerts(0),
      mNumFaces(0),
      mChangelist(0),
      mIconCamInitialized(false),
      mIconCamXfm(Transform::sID),
      mIconCamNear(0.1F),
      mIconCamFar(1000.0F),
      mInstancePollingMode(0),
      mCopyOnInstance(true),
      mDrivesParent(false),
      mStatic(false),
      mCapabilities(15),
      mShowEntity(),
      mIconData(),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x1D1F60. Every layer starts visible,
// selectable and unmodified.
EntityHeaderCom::RuntimeData::RuntimeData() : mNumModifiedLayers(0), mPollRequested(false) {
    mLayerVisible.resize(kMaxLayers);
    for (bool& visible : mLayerVisible) {
        visible = true;
    }
    mLayerSelectable.resize(kMaxLayers);
    for (bool& selectable : mLayerSelectable) {
        selectable = true;
    }
    mLayerModified.resize(kMaxLayers);
    for (bool& modified : mLayerModified) {
        modified = false;
    }
    mLayerChanges.resize(kMaxLayers);
    for (LayerChanges& changes : mLayerChanges) {
        changes.mCount = 0;
        changes.mDirty = false;
    }
}

// Inlined into _Imprint at 0x1D58C2: the properties up to "icon_data" are
// copied as bytes.
EntityHeaderCom::EntityHeaderCom(const EntityHeaderCom& other)
    : Component(other),
      mNumInstances(other.mNumInstances),
      mNumMeshes(other.mNumMeshes),
      mNumParticles(other.mNumParticles),
      mNumPropanims(other.mNumPropanims),
      mNumLights(other.mNumLights),
      mNumVerts(other.mNumVerts),
      mNumFaces(other.mNumFaces),
      mChangelist(other.mChangelist),
      mIconCamInitialized(other.mIconCamInitialized),
      mIconCamXfm(other.mIconCamXfm),
      mIconCamNear(other.mIconCamNear),
      mIconCamFar(other.mIconCamFar),
      mInstancePollingMode(other.mInstancePollingMode),
      mCopyOnInstance(other.mCopyOnInstance),
      mDrivesParent(other.mDrivesParent),
      mStatic(other.mStatic),
      mCapabilities(other.mCapabilities),
      mShowEntity(other.mShowEntity),
      mIconData(),
      mRuntime() {
    mIconData._Copy(other.mIconData);
}

// Reconstructed from eboot.elf at 0x1D51B0.
bool EntityHeaderCom::CopyOnInstance() const {
    return mCopyOnInstance;
}

// Reconstructed from eboot.elf at 0x1D51C0. A "static" entity's resource
// suppresses its poll and is asked to poll once.
bool EntityHeaderCom::_OnResourcesLoaded() {
    EntityResource* const resource = mObject->mEntity->mResource;
    if (resource == nullptr || !resource->IsA(TransEntityResource::Id())) {
        return true;
    }
    if (mIconData.mSize != 0) {
        resource->SetIconData(mIconData.mData, mIconData.mSize);
        mIconData.Resize(0);
    }
    static_cast<TransEntityResource*>(resource)->mSuppressPoll = mStatic;
    if (mStatic) {
        mRuntime.mPollRequested = true;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x1D5800.
Symbol EntityHeaderCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x1D5810.
Symbol EntityHeaderCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x1D5820.
int EntityHeaderCom::CurrentRev() const {
    return const_cast<EntityHeaderCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x1D5840.
bool EntityHeaderCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x1D5870.
Component* EntityHeaderCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x1D5880. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* EntityHeaderCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<EntityHeaderCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) EntityHeaderCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x1D5A20.
PropRegistry& EntityHeaderCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x1D5A30.
ComMetaData& EntityHeaderCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x1D5D00, the body slot 0 (0x1D50E0)
// jumps to. The members free the layer state and the icon data.
EntityHeaderCom::~EntityHeaderCom() {}
