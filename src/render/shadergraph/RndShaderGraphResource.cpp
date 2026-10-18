// render/RndShaderGraphResource.o: only GetShaderGraph is reconstructed,
// with the registry getter of the materials' dynamic properties.
#include "render/shadergraph/RndShaderGraphResource.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "os/memory/MemMgr.h"
#include "render/shadergraph/RndShaderGraph.h"

namespace {

// The FNV-1a offset basis the registry's layout hash starts from. Name not
// in the reference map.
constexpr unsigned int kFnvOffsetBasis = 0x811C9DC5U;

}  // namespace

// Reconstructed from eboot.elf at 0x50B110.
RndShaderGraph* RndShaderGraphResource::GetShaderGraph() {
    return mEntity->GetRoot()->GetCom<RndShaderGraph>();
}

// Reconstructed from eboot.elf at 0x4F60C0. The binary emits it among the
// material objects (render/RndMaterialCom.o follows); the template lives in
// entity/core/DynamicCom.h, so the specialization is defined here. A stale
// registry is rebuilt in the "metadata" heap.
template <>
PropRegistry& DynamicPropRegistry<RndShaderGraphResource>::Get(
    Symbol className) {
    if (mDirty) {
        static const long sMetadataHeap = MemFindHeap("metadata");
        MemPushHeap(sMetadataHeap);
        if (!mDirty) {
            mRegistry.~PropRegistry();
            mDirty = true;
        }
        mClassName = className;
        new (&mRegistry) PropRegistry();
        mCrc = kFnvOffsetBasis;
        mDirty = false;
        _Build(*this);
        MemPopHeap();
    }
    return mRegistry;
}
