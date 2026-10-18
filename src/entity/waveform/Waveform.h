#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "math/random/Rand.h"
#include "utl/containers/Vector.h"

class Component;
class EntityResource;

// A waveform's random evaluation state: the blend between its two curves
// at the start, and how far the blend moves over the waveform's span
// (Waveform<float>::Evaluate, 0x2D7C60, reads the second for the blend
// modes that move). A new waveform draws both from its generator. The map
// names the type; the field names are not in the reference map.
struct WaveformEvalData {
    float mBlend;
    float mBlendRate;
};

static_assert(sizeof(WaveformEvalData) == 8);

// The curves a waveform evaluates (entity/Waveform.o; 248 bytes): a type,
// how the curves blend, the two key tracks, their baked samples and the
// range of values. Only the layout the reconstructed code reads is
// modelled; the methods are not reconstructed. A default-constructed
// Waveform<T> starts with sDefault (0x19C5390 for float); WaveformInit builds
// the resources at 0x19E4A00 and 0x19E4A08, which the map does not name.
// Field names are not in the reference map.
template <class T>
class WaveformResource : public Resource {
public:
    ResourceMetaData* GetMetaData() const override;
    Symbol GetId() const override;
    bool IsA(Symbol type) const override;
    bool Load(BinStream& stream, bool cached) override;
    void Save(BinStream& stream, bool cached) override;
    bool Fail() const override;

    static ResourcePtr<WaveformResource> sDefault;

    // The map's WaveformType: 0 evaluates the first key of the first track,
    // 1 blends the first keys of both tracks, 2 samples the first baked
    // curve and 3 blends both baked curves. -1 until set. It lives in the
    // tail padding of Resource.
    int mType;
    // The map's WaveformBlendMode (0 to 5): how the eval data blends the
    // two curves.
    int mBlendMode;
    // The two TKeyTrack<T> (entity/Key.o), 56 bytes each; not modelled.
    alignas(8) unsigned char mTracks[2][56];
    // The tracks baked into samples.
    eastl::vector<T> mSamples[2];
    // Set once the samples are baked.
    bool mBaked;
    // The smallest and largest value, starting at 1e30 and -1e30.
    float mValueRange[2];
};

// A guarded template static, null until set: 0x19C5390 (guard 0x19C5398)
// for float.
template <class T>
ResourcePtr<WaveformResource<T>> WaveformResource<T>::sDefault;

// A property that varies over time or life through a WaveformResource<T>
// (entity/Waveform.o; 40 bytes): the resource, the property's own
// evaluation state, the generator it draws from, and a second resource
// that modulates the first. The map names the type; the field names are
// not in the reference map, and the evidence for the last two is weak.
template <class T>
class Waveform {
public:
    // The resource, usually the owner's default for the property, with an
    // evaluation state drawn from gRand. Inlined into every owner's
    // constructor, for example RndVolumetricScatteringCom's at 0x453C00.
    explicit Waveform(WaveformResource<T>* resource)
        : mResource(resource), mRand(nullptr), mOverride(), mOverrideData{0, 0} {
        DrawEvalData();
    }
    // Shares the other waveform's resources and draws a new evaluation
    // state. Inlined into the owners' copies, for example at 0x456770.
    Waveform(const Waveform& other)
        : mResource(other.mResource.Get()),
          mRand(nullptr),
          mOverride(other.mOverride.Get()),
          mOverrideData{other.mOverrideData[0], other.mOverrideData[1]} {
        DrawEvalData();
    }
    // The owner's default resource, usually the property's (0x19C5390 for
    // float, a guarded template static), with an evaluation state drawn
    // from gRand. Inlined into the owners' constructors, for example
    // RndParticleCom's EmitterParams at 0x601FC0.
    Waveform() : mResource(WaveformResource<T>::sDefault.Get()), mRand(nullptr), mOverride(), mOverrideData{0, 0} {
        DrawEvalData();
    }
    Waveform& operator=(const Waveform&) = delete;

    // Takes the resource and gRand as the generator and drops the
    // modulating resource, keeping the evaluation state. Inlined into the
    // owners' constructors, for example at 0x601FC0. Name not in the
    // reference map.
    void SetResource(WaveformResource<T>* resource) {
        mResource = resource;
        mRand = &gRand;
        mOverride = nullptr;
        mOverrideData[0] = 0;
        mOverrideData[1] = 0;
    }

    // Draws the blend and its rate from gRand, which becomes the
    // generator. Inlined. Name not in the reference map.
    void DrawEvalData() {
        const float blend = gRand.Float();
        mEvalData.mBlend = blend;
        mEvalData.mBlendRate = gRand.Float() - blend;
        mRand = &gRand;
    }

    ResourcePtr<WaveformResource<T>> mResource;
    WaveformEvalData mEvalData;
    Rand* mRand;
    ResourcePtr<WaveformResource<T>> mOverride;
    int mOverrideData[2];
};

static_assert(offsetof(WaveformResource<float>, mType) == 44);
static_assert(offsetof(WaveformResource<float>, mTracks) == 56);
static_assert(offsetof(WaveformResource<float>, mSamples) == 168);
static_assert(offsetof(WaveformResource<float>, mBaked) == 232);
static_assert(offsetof(WaveformResource<float>, mValueRange) == 236);
static_assert(sizeof(WaveformResource<float>) == 248);
static_assert(offsetof(Waveform<float>, mEvalData) == 8);
static_assert(offsetof(Waveform<float>, mRand) == 16);
static_assert(offsetof(Waveform<float>, mOverride) == 24);
static_assert(sizeof(Waveform<float>) == 40);

// Builds the default waveform resources and registers the key types the
// waveforms use. The map's WaveformInit(). Not reconstructed.
void WaveformInit();  // 0x1C8B70
// Loads the resources of the component's waveform properties (property
// types 18 and 19), recursing into arrays and structs; called by
// Component::LoadResources unless it is quiet. The map's signature is
// LoadWaveformResources(ObjPtr const&, Component&). Not reconstructed.
void LoadWaveformResources(Component* component);  // 0x1C9A90
// Uninlines the layer's waveform resources and inlines those its objects'
// waveform properties use. The map's InlineWaveformResources(
// EntityResource&, unsigned long). Not reconstructed.
void InlineWaveformResources(EntityResource& resource, unsigned long layer);  // 0x1C9C00
