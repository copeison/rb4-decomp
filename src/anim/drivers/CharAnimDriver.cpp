// anim/CharAnimDriver.o (0x2DD2F0 to 0x2DF92F). The binary emits here the
// IK sort it uses (eastl quick_sort_impl and partial_sort over CharIkCom*,
// 0x2DF3E0 and 0x2DF580), the PropArray<CharBoneChannelMask> element
// operations (vtable 0x18F5210), AnimDriver's inline destructors (0x2DF140,
// 0x2DF1B0) and _OnStopAnim (0x2DF0D0), and the guarded PropArray<T>::sType
// copies (0x2DF0E0, 0x2DF110). The map's debug drawing (DebugDisplayAnims,
// _DrawPlaybackInfo(s)) and EditPoll are not in this build.
#include "anim/drivers/CharAnimDriver.h"

#include "anim/char/CharAnim.h"
#include "anim/char/CharAnimResource.h"
#include "anim/char/CharBoneChannelMask.h"
#include "anim/char/CharBoneStateSetFormat.h"
#include "anim/char/CharClipResource.h"
#include "anim/char/CharIkCom.h"
#include "anim/skeleton/CharSkeleton.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropUtl.h"
#include "render/skeletons/RndSkeletonPoseCom.h"
#include "utl/containers/Sort.h"

namespace {

// Reconstructed from eboot.elf at 0x2DDA20. Orders IK components by
// priority, lowest first. Name not in the reference map.
bool IkComPriorityLess(const CharIkCom* a, const CharIkCom* b) {
    return a->GetPriority() < b->GetPriority();
}

// The resource as a CharAnimResource, or null. Name not in the reference
// map.
CharAnimResource* AsCharAnim(Resource* resource) {
    return resource->IsA(CharAnimResource::Id()) ? static_cast<CharAnimResource*>(resource) : nullptr;
}

// The property the reference names on the component of the object the
// playback's animation names, read into `value` as `type`; false when the
// object, the component or the property is missing (a missing property or
// a failed read is reported, the message compiled out). Inlined into the
// property evaluations. Name not in the reference map.
bool ReadPlaybackProp(const PlaybackInfo& info, Entity* entity, const PropRef& ref, void* value, PropertyType type) {
    GameObject* const object = info.mAnim->GetObject(entity);
    if (object == nullptr || object->mComs.size() == 0) {
        return false;
    }
    Component* const com = object->FindCom(ref.mCom);
    if (com == nullptr) {
        return false;
    }
    void* storage = com;
    const PropInfo* const prop = com->_GetPropRegistry().FindProp(ref.mPath, &storage);
    if (prop != nullptr && _GetProp(*com, ref.mPath, *prop, storage, value, type)) {
        return true;
    }
    static_cast<void>(com->MakeErrorName());
    char text[512] = {};
    ref.mPath.ToString(text, sizeof(text));
    return false;
}

}  // namespace

// Reconstructed from eboot.elf at 0x2DD2F0. The pose component is set by
// _Init.
CharAnimDriver::CharAnimDriver()
    : mInitialized(false), mDisabled(false), mBuffer(), mSubTreeWeights(), mIkComs() {}

// Reconstructed from eboot.elf at 0x2DD380. The deleting destructor is at
// 0x2DD450.
CharAnimDriver::~CharAnimDriver() {}

// Reconstructed from eboot.elf at 0x2DD470.
void CharAnimDriver::SetSubTreeWeight(unsigned long subTree, float weight) {
    const unsigned long size = mSubTreeWeights.size();
    if (size <= subTree) {
        mSubTreeWeights.resize(subTree + 1);
        for (unsigned long i = size; i < subTree; ++i) {
            mSubTreeWeights[i] = 1.0f;
        }
    }
    mSubTreeWeights[subTree] = weight;
}

// Reconstructed from eboot.elf at 0x2DD560.
void CharAnimDriver::StopSubTree(unsigned long subTree) {
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end();) {
        if (it->mSubTreeIndex != subTree) {
            ++it;
            continue;
        }
        AnimInstID id = GetInstID(*it);
        ++it;
        StopAnim(id);
    }
}

// Reconstructed from eboot.elf at 0x2DD600.
void CharAnimDriver::StopAll() {
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end();) {
        AnimInstID id = GetInstID(*it);
        ++it;
        StopAnim(id);
    }
}

// Reconstructed from eboot.elf at 0x2DD6A0.
void CharAnimDriver::_CacheIkComs(GameObject* root) {
    mIkComs.clear();
    for (unsigned long i = 0; i < root->mComs.size(); ++i) {
        Component* const com = root->mComs[i].mCom;
        if (com != nullptr && com->IsA(CharIkCom::sClassName)) {
            mIkComs.push_back(static_cast<CharIkCom*>(com));
        }
    }
    eastl::sort(mIkComs.begin(), mIkComs.end(), IkComPriorityLess);
}

// Reconstructed from eboot.elf at 0x2DDA60. A playback with an
// auto-destruct mode of 3 or 4 that the layered playbacks after it in its
// slot hide completely is stopped after the blend.
void CharAnimDriver::_Poll(Entity* entity) {
    GameObject* const root = reinterpret_cast<GameObject*>(entity);
    if (mDisabled) {
        return;
    }
    if (!mInitialized) {
        _Init(root);
        if (!mInitialized) {
            return;
        }
    }
    mBuffer.BeginFrame();
    eastl::vector<AnimInstID> hidden;
    float* const weights =
        mPlaybacks.size() != 0 ? static_cast<float*>(__builtin_alloca(mPlaybacks.size() * sizeof(float))) : nullptr;
    for (auto first = mPlaybacks.begin(); first != mPlaybacks.end();) {
        const unsigned long subTree = first->mSubTreeIndex;
        auto last = first;
        unsigned long count = 0;
        while (last != mPlaybacks.end() && last->mSubTreeIndex == subTree) {
            ++last;
            ++count;
        }
        // The weights from the last playback of the slot back to the
        // first: what the layered playbacks after one leave of it.
        float visible = 1.0f;
        float* weight = weights + count - 1;
        auto it = last;
        do {
            it.mpNode = it.mpNode->mpPrev;
            *weight = visible * it->mWeight;
            if (visible == 0.0f && (it->mAutoDestruct == 3 || it->mAutoDestruct == 4)) {
                hidden.push_back(GetInstID(*it));
            }
            if (it->mBlendMode == AnimResource::kBlendLayered) {
                visible *= 1.0f - it->mWeight;
            }
            --weight;
        } while (it != first);

        const float slotWeight = subTree < mSubTreeWeights.size() ? mSubTreeWeights[subTree] : 1.0f;
        const float scale = (visible - 1.0f) * slotWeight + 1.0f;
        if (scale != 1.0f) {
            static_cast<const CharAnim*>(first->mAnim->GetCharData())->ScaleDown(scale, mBuffer);
        }
        weight = weights;
        for (it = first; it != last; ++it, ++weight) {
            const float frame = it->GetCurrentFrame();
            const float playbackWeight = slotWeight * *weight;
            if (playbackWeight != 0.0f) {
                const float delta = frame - it->mLastFrame;
                static_cast<const CharAnim*>(it->mAnim->GetCharData())
                    ->Apply(playbackWeight, frame, delta, it->mWrapped, AnimResource::kBlendAdditive, mBuffer);
            }
            it->SendEvents(root->mEntity, playbackWeight);
        }
        first = last;
    }
    for (AnimInstID& id : hidden) {
        StopAnim(id);
    }
    const CharSkeleton* const skeleton = mSkeletonCom->GetSkeleton();
    mBuffer.EndFrame();
    Transform deltaRoot = {{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, {0.0f, 0.0f, 0.0f}};
    mBuffer.GetDeltaRootXfm(deltaRoot);
    mSkeletonCom->UpdateTransforms(&mBuffer.mAbsolute, deltaRoot);
    for (unsigned long i = 0; i < mIkComs.size(); ++i) {
        mIkComs[i]->Apply(mSkeletonCom->mPose, skeleton);
    }
    mSkeletonCom->UpdateRenderTransforms();
}

// Reconstructed from eboot.elf at 0x2DE000. Every bone keeps its position,
// full rotation, scale and inverse parent scale.
void CharAnimDriver::_Init(GameObject* root) {
    RndSkeletonPoseCom* const com = root->GetCom<RndSkeletonPoseCom>();
    if (com == nullptr) {
        return;
    }
    mSkeletonCom = com;
    const CharSkeleton* const skeleton = com->GetSkeleton();
    if (skeleton == nullptr) {
        return;
    }
    const eastl::vector<Symbol> names = skeleton->mNames;
    CharBoneChannelMask mask;
    PropArray<CharBoneChannelMask> masks;
    masks.Resize(names.size());
    for (unsigned long i = 0; i < names.size(); ++i) {
        mask.mName = names[i];
        mask.SetSRTIPS();
        masks[i] = mask;
    }
    const CharBoneStateSetFormat format(masks);
    mBuffer.Init(format, skeleton, com->GetSkeletonPath());
    com->Init(&mBuffer.mAbsolute);
    _CacheIkComs(root);
    mEntity = root->mEntity;
    mInitialized = true;
}

// Reconstructed from eboot.elf at 0x2DE2E0.
AnimInstID CharAnimDriver::StartAnim(Resource* resource, const AnimOptions& options) {
    if (resource == nullptr || resource->Fail()) {
        return AnimInstID::NullID();
    }
    AnimResource* anim = nullptr;
    if (resource->IsA(CharAnimResource::Id())) {
        anim = static_cast<CharAnimResource*>(resource);
    } else if (resource->IsA(CharClipResource::Id())) {
        anim = static_cast<CharClipResource*>(resource);
    } else {
        return AnimInstID::NullID();
    }
    if (mPlaybacks.size() > 100) {
        return AnimInstID::NullID();
    }
    return GetInstID(*_StartAnim(anim, options));
}

// Reconstructed from eboot.elf at 0x2DE4A0.
void CharAnimDriver::Reset(GameObject* root) {
    StopAll();
    RndSkeletonPoseCom* const com = root->GetCom<RndSkeletonPoseCom>();
    if (com != nullptr) {
        com->mPose.Reset();
    }
    mInitialized = false;
}

// Reconstructed from eboot.elf at 0x2DE600. A layered playback scales down
// the value and the weight gathered before it.
float CharAnimDriver::EvaluateFloatProperty(const PropRef& ref, float& value, unsigned long subTree) {
    value = 0.0f;
    float total = 0.0f;
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end(); ++it) {
        if (subTree != static_cast<unsigned long>(-1) && it->mSubTreeIndex != subTree) {
            continue;
        }
        float propValue;
        if (!ReadPlaybackProp(*it, mEntity, ref, &propValue, kPropertyFloat)) {
            continue;
        }
        const float weight = it->mWeight;
        if (it->mBlendMode == AnimResource::kBlendLayered) {
            value *= 1.0f - weight;
            total *= 1.0f - weight;
        }
        value += propValue * weight;
        total += weight;
    }
    return total;
}

// Reconstructed from eboot.elf at 0x2DE990. A layered playback scales down
// the weight to beat.
CharAnimResource* CharAnimDriver::FindMajorityAnim(unsigned long subTree) const {
    CharAnimResource* majority = nullptr;
    float best = -1e30f;
    auto& playbacks = const_cast<CharAnimDriver*>(this)->mPlaybacks;
    for (auto it = playbacks.begin(); it != playbacks.end(); ++it) {
        Resource* const resource = it->mResource.Get();
        if (!resource->IsA(CharAnimResource::Id())) {
            continue;
        }
        if (subTree != static_cast<unsigned long>(-1) && it->mSubTreeIndex != subTree) {
            continue;
        }
        const float weight = it->mWeight;
        if (it->mBlendMode == AnimResource::kBlendLayered) {
            best *= 1.0f - weight;
        }
        const float larger = weight > best ? weight : best;
        if (larger != best) {
            best = larger;
            majority = resource != nullptr ? AsCharAnim(resource) : nullptr;
        }
    }
    return majority;
}

// Reconstructed from eboot.elf at 0x2DED30.
float CharAnimDriver::EvaluateBoolProperty(const PropRef& ref, float& value) {
    value = 0.0f;
    float total = 0.0f;
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end(); ++it) {
        bool propValue;
        if (!ReadPlaybackProp(*it, mEntity, ref, &propValue, kPropertyBool)) {
            continue;
        }
        const float weight = it->mWeight;
        if (it->mBlendMode == AnimResource::kBlendLayered) {
            value *= 1.0f - weight;
            total *= 1.0f - weight;
        }
        value += static_cast<float>(propValue) * weight;
        total += weight;
    }
    return total;
}
