// anim/AnimInit.o (part): Anim::Init. The object also holds each anim
// class's Init and factory; the reconstructed ones are defined with their
// classes (AnimCom, AnimDriverCom, CharSkeletonResource).
#include "anim/core/AnimInit.h"

#include "anim/char/CharAnimCom.h"
#include "anim/char/CharAnimRawResource.h"
#include "anim/char/CharAnimResource.h"
#include "anim/char/CharBoneMaskCom.h"
#include "anim/char/CharBoneMaskResource.h"
#include "anim/char/CharClipCom.h"
#include "anim/char/CharClipResource.h"
#include "anim/char/CharIkBoneInterpCom.h"
#include "anim/char/CharIkCom.h"
#include "anim/char/CharIkDriverCom.h"
#include "anim/char/CharLipsyncResource.h"
#include "anim/core/AnimResource.h"
#include "anim/drivers/AnimCom.h"
#include "anim/drivers/AnimDriverCom.h"
#include "anim/props/PropAnimCom.h"
#include "anim/props/PropAnimResource.h"
#include "anim/props/PropKeysBoolCom.h"
#include "anim/props/PropKeysColorCom.h"
#include "anim/props/PropKeysFloatCom.h"
#include "anim/props/PropKeysIntCom.h"
#include "anim/props/PropKeysObjCom.h"
#include "anim/props/PropKeysQuatCom.h"
#include "anim/props/PropKeysRotationCom.h"
#include "anim/props/PropKeysStrCom.h"
#include "anim/props/PropKeysSymCom.h"
#include "anim/props/PropKeysTransformSRTCom.h"
#include "anim/props/PropKeysVec3Com.h"
#include "anim/props/PropKeysWaveformCom.h"
#include "anim/props/PropertyBlenderCom.h"
#include "anim/skeleton/CharSkeletonResource.h"
#include "entity/core/ComMetaData.h"
#include "math/color/Color.h"

namespace Anim {

// Reconstructed from eboot.elf at 0x2A7080. The struct registration is
// RegisterPropStruct<AnimResource::Event> (0x2A7180).
void Init() {
    RegisterPropStruct<AnimResource::Event>(Symbol("AnimResource::Event"));
    AnimCom::Init();
    AnimDriverCom::Init();
    PropAnimResource::Init();
    PropAnimCom::Init();
    PropKeysBoolCom::Init();
    PropKeysIntCom<signed char>::Init();
    PropKeysIntCom<short>::Init();
    PropKeysIntCom<int>::Init();
    PropKeysIntCom<unsigned char>::Init();
    PropKeysIntCom<unsigned short>::Init();
    PropKeysIntCom<unsigned int>::Init();
    PropKeysIntCom<long>::Init();
    PropKeysIntCom<unsigned long>::Init();
    PropKeysFloatCom::Init();
    PropKeysVec3Com::Init();
    PropKeysRotationCom::Init();
    PropKeysTransformSRTCom::Init();
    PropKeysQuatCom::Init();
    PropKeysObjCom::Init();
    PropKeysSymCom::Init();
    PropKeysStrCom::Init();
    PropKeysColorCom::Init();
    PropKeysWaveformCom<float>::Init();
    PropKeysWaveformCom<Hmx::Color>::Init();
    CharAnimResource::Init();
    CharAnimRawResource::Init();
    CharSkeletonResource::Init();
    CharBoneMaskResource::Init();
    CharLipsyncResource::Init();
    CharAnimCom::Init();
    CharBoneMaskCom::Init();
    CharIkCom::Init();
    CharIkDriverCom::Init();
    CharIkBoneInterp::Init();
    PropertyBlenderCom::Init();
    CharClipResource::Init();
    CharClipCom::Init();
}

}  // namespace Anim
