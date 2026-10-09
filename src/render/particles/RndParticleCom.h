#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "math/vector/Vector4.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "render/particles/RndParticleCollection.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class Rand;
class RndDrawNodeCom;
class RndMesh;
class RndMeshResource;
class RndParticleBuffer;
class RndParticleExtCom;
class TransCom;

// The particle system component (render/RndParticleCom.o, 0x601880 to
// 0x61E1F5). Its class id is "PSys". The emitter spawns particles at a rate,
// in bursts and by the distance it moves; each particle lives in the
// component's RndParticleCollection and is driven by waveforms over the
// system's duration and over its own life, by affectors and colliders on
// the entity, and by the extensions on the object. Sprites draw through one
// RndParticleBuffer instance, meshes as one draw instance per particle. The
// vtable at 0x192BD08 has 46 slots. The object is 3440 bytes.
//
// The waveform properties are Waveform<float> and Waveform<Hmx::Color>
// (entity/Waveform.o, not modelled): 40 bytes each, a reference to the
// WaveformResource, the property's own WaveformEvalData, the Rand it draws
// from (+16) and an animation override. They are kept as opaque storage
// here, so the functions that build, copy or evaluate them are declared
// only. Field names are not in the reference map; the properties are named
// after the registry (_Init, 0x603850).
class RndParticleCom : public RndDrawInstanceCom {
public:
    // How the quads turn, from the particle_alignment property. Enumerator
    // names are not in the reference map; they follow the property's
    // labels.
    enum ParticleAlignment : int {
        kCameraAligned = 0,    // Face the camera.
        kCameraXYAligned = 1,  // Face the camera in XY, keeping Z straight up.
        kXAxisAligned = 2,     // Face world +X.
        kYAxisAligned = 3,     // Face world -Y.
        kZAxisAligned = 4,     // Face world +Z.
    };
    // "particle_type". Enumerator names are not in the reference map; they
    // follow the property's labels.
    enum ParticleType : int {
        kSprite = 0,  // One RndParticleBuffer instance.
        kMesh = 1,    // One mesh instance per particle.
    };

    // The size of an opaque Waveform<T> property. Name not in the reference
    // map.
    static constexpr unsigned long kWaveformSize = 40;

    // The "emitter" properties (+192). The map names the type.
    struct EmitterParams {
        // Not reconstructed: it builds the waveforms from the class's
        // defaults (sDefaultPitch, sDefaultYaw).
        EmitterParams();  // 0x601FC0
        EmitterParams(const EmitterParams& other);  // 0x619580
        // Inlined into ~RndParticleCom (0x601B90); releases the waveforms.
        ~EmitterParams();

        // "duration": the system's cycle, which the over-duration waveforms
        // span. 30 by default.
        float mDuration;
        // "emit_can_kill_oldest": kill the oldest particle to make room.
        bool mEmitCanKillOldest;
        // "never_kill": particles never die of age.
        bool mNeverKill;
        // "min_box_extents" and "max_box_extents": the emission box.
        Vector3 mMinBoxExtents;
        Vector3 mMaxBoxExtents;
        // "pitch" and "yaw": the emission direction over the duration, in
        // degrees (Waveform<float>).
        alignas(8) unsigned char mPitch[kWaveformSize];
        alignas(8) unsigned char mYaw[kWaveformSize];
        // "birth_momentum" and "birth_momentum_amount": new particles
        // inherit this much of the emitter's speed.
        bool mBirthMomentum;
        float mBirthMomentumAmount;
        // "subsamples": steps the rate emission splits a frame into.
        int mSubsamples;
        // "local_space": the particles live in the object's space.
        bool mLocalSpace;
    };

    // The "particle" properties (+320). The map names the type.
    struct ParticleParams {
        // Not reconstructed: it builds the waveforms from the class's
        // defaults.
        ParticleParams();  // 0x602200
        ParticleParams(const ParticleParams& other);  // 0x619720
        ~ParticleParams();  // 0x6183C0

        // "particle_type", a ParticleType.
        int mParticleType;
        // "mesh_resource_path": the mesh a kMesh system draws.
        ResourcePath mMeshResourcePath;
        // "max_count": the most particles alive at once, 100 by default.
        int mMaxCount;
        // The Waveform<float> properties and their multipliers.
        alignas(8) unsigned char mEmitRate[kWaveformSize];  // "emit_rate"
        float mEmitRateMultiplier;       // "emit_rate_multiplier", 1
        float mEmitDistancePerParticle;  // "emit_distance_per_particle", 0
        alignas(8) unsigned char mLifespan[kWaveformSize];  // "lifespan"
        float mLifespanMultiplier;  // "lifespan_multiplier", 1
        alignas(8) unsigned char mSpeed[kWaveformSize];  // "speed"
        // "speed_multiplier_over_life".
        alignas(8) unsigned char mSpeedMultiplierOverLife[kWaveformSize];
        float mDrag;  // "drag", 0
        alignas(8) unsigned char mInitialSize[kWaveformSize];      // "initial_size"
        alignas(8) unsigned char mXScaleOverLife[kWaveformSize];   // "x_scale_over_life"
        alignas(8) unsigned char mYScaleOverLife[kWaveformSize];   // "y_scale_over_life"
        alignas(8) unsigned char mZScaleOverLife[kWaveformSize];   // "z_scale_over_life"
        bool mUniformScale;  // "uniform_scale", true
        alignas(8) unsigned char mXPivotOverLife[kWaveformSize];   // "x_pivot_over_life"
        alignas(8) unsigned char mYPivotOverLife[kWaveformSize];   // "y_pivot_over_life"
        alignas(8) unsigned char mZPivotOverLife[kWaveformSize];   // "z_pivot_over_life"
        bool mOffsetThenScale;  // "offset_then_scale", true
        // "force_direction": the constant acceleration, zero by default.
        Vector3 mForceDirection;
    };

    // The "color" properties (+832). The map names the type.
    struct ColorParams {
        // Not reconstructed: it builds the waveforms from sDefaultColor and
        // sDefaultAlpha.
        ColorParams();  // 0x602D70
        ColorParams(const ColorParams& other);  // 0x61A020
        // Inlined into ~RndParticleCom (0x601B90).
        ~ColorParams();

        // "color" (Waveform<Hmx::Color>) and "alpha" (Waveform<float>).
        alignas(8) unsigned char mColor[kWaveformSize];
        alignas(8) unsigned char mAlpha[kWaveformSize];
    };

    // "initial_rotation". Enumerator names are not in the reference map;
    // they follow the property's labels.
    enum InitialRotation : int {
        kUnrotated = 0,
        kInheritFromEmitter = 1,
        kRandomRotation = 2,
    };

    // The "spin" properties (+912). The map names the type.
    struct SpinParams {
        // Not reconstructed: it builds the waveforms from the class's
        // defaults.
        SpinParams();  // 0x602EC0
        SpinParams(const SpinParams& other);  // 0x61A1A0
        ~SpinParams();  // 0x6182F0

        // "initial_rotation", an InitialRotation.
        int mInitialRotation;
        // "initial_x_rotation_angle", "initial_y_rotation_angle" and
        // "initial_rotation_angle" (also "initial_z_rotation_angle"), in
        // degrees.
        alignas(8) unsigned char mInitialXRotationAngle[kWaveformSize];
        alignas(8) unsigned char mInitialYRotationAngle[kWaveformSize];
        alignas(8) unsigned char mInitialRotationAngle[kWaveformSize];
        // A mesh's orientation, edited as "x_orientation", "y_orientation"
        // and "z_orientation" in degrees; the identity by default.
        Hmx::Matrix3 mOrientation;
        // "x_rpm_over_life", "y_rpm_over_life" and "rpm_over_life" (also
        // "z_rpm_over_life").
        alignas(8) unsigned char mXRpmOverLife[kWaveformSize];
        alignas(8) unsigned char mYRpmOverLife[kWaveformSize];
        alignas(8) unsigned char mRpmOverLife[kWaveformSize];
        // "x_rpm_drag", "y_rpm_drag" and "rpm_drag" (also "z_rpm_drag").
        float mXRpmDrag;
        float mYRpmDrag;
        float mRpmDrag;
    };

    // "extra_data_type_0" to "extra_data_type_2". Enumerator names are not in
    // the reference map; they follow the property's labels.
    enum ExtraDataType : int {
        kExtraDataDisabled = 0,
        kExtraDataOverLife = 1,
        kExtraDataOnEmission = 2,
    };

    // The "particle_extra_data" properties (+1216): three data channels
    // copied into the particles' vertices or instances. Name not in the
    // reference map.
    struct ExtraDataParams {
        // Not reconstructed: it builds the waveforms from
        // sDefaultExtraData.
        ExtraDataParams();  // 0x6033F0
        // Inlined into RndParticleCom's copy constructor (0x619130).
        ExtraDataParams(const ExtraDataParams& other);
        // Inlined into ~RndParticleCom (0x601B90).
        ~ExtraDataParams();

        int mExtraDataType0;
        int mExtraDataType1;
        int mExtraDataType2;
        // "extra_data_0" to "extra_data_2" (Waveform<float>).
        alignas(8) unsigned char mExtraData0[kWaveformSize];
        alignas(8) unsigned char mExtraData1[kWaveformSize];
        alignas(8) unsigned char mExtraData2[kWaveformSize];
    };

    // The "render" properties (+1352). The map names the type.
    struct RenderParams {
        // Out of line in the map's build; inlined here.
        RenderParams() : mParticleAlignment(kCameraAligned), mSortMethod(0) {}

        // "particle_alignment", a ParticleAlignment.
        int mParticleAlignment;
        // "sort_method": 0 back to front, 1 oldest first, 2 newest first.
        int mSortMethod;
    };

    // "per_particle_collision_shape". Enumerator names are not in the
    // reference map; they follow the property's labels.
    enum CollisionShape : int {
        kCollidePoint = 0,
        kCollideSphere = 1,
    };

    // The "collision" properties (+1368). Name not in the reference map.
    struct CollisionParams {
        CollisionParams() : mPerParticleCollisionShape(kCollidePoint), mElasticity(0.75F) {}

        // "per_particle_collision_shape", a CollisionShape.
        int mPerParticleCollisionShape;
        // "elasticity": 1 for a full bounce, 0 to stick.
        float mElasticity;
        // "colliders": the RndParticleColliderCom objects of the entity.
        PropArray<GameObjectId> mColliders;
    };

    // The members from 1456 that are not properties. The map names the
    // type; the constructor is also emitted on its own at 0x6036F0.
    struct RuntimeData {
        RuntimeData();  // 0x6036F0
        // Inlined into ~RndParticleCom (0x601B90).
        ~RuntimeData();

        // Set when "particle_type" changes (its handler at 0x618540); not
        // read in this object, so the name is weak.
        bool mParticleTypeChanged;
        // The RndParticleExtCom components on the object, which add and
        // remove themselves.
        eastl::vector<RndParticleExtCom*> mExtensions;
        // The mesh resource and its mesh for a kMesh system; the mesh is the
        // shared box (sDefaultMesh) when no path is set.
        ResourcePtr<RndMeshResource> mMeshResource;
        RndMesh* mMesh;
        // The clock "time_units" reads, the object's TransCom and DrawNode.
        TimeMgr::Clock* mClock;
        TransCom* mTrans;
        RndDrawNodeCom* mDrawNode;
        // The system's time: the sum of the clock's deltas.
        float mTime;
        // The emitter's position at the end of the last poll, for the
        // distance emission.
        Vector3 mLastPosition;
        // The emitter's transform at the end of the last rate emission.
        Transform mLastXfm;
        // "num_active": the live particles.
        int mNumActive;
        // The fractional particles the rate emission still owes.
        float mRateRemainder;
        // The distance the emitter moved since the last distance emission.
        float mDistanceTravelled;
        // "burst_count" "min" and "max": particles to fire this frame, a
        // random count between the two; both reset once fired.
        int mBurstCountMin;
        int mBurstCountMax;
        RndParticleCollection mParticles;
        // The generator the waveforms draw from; seeded with the low bits of
        // this structure's address.
        Rand* mRand;
        // The sprite buffer of a kSprite system.
        RndParticleBuffer* mBuffer;
        // The transform the buffer draws the particles with: the world
        // transform in local space, otherwise the identity.
        Transform mParticleXfm;
        // Whether the object has an RndParticleVelocityAlignCom.
        bool mVelocityAligned;
        // The transforms of the particles queued this frame, and per-new-
        // particle data the initialization fills (_InitParticles).
        eastl::vector<Transform> mNewParticleXfms;
        eastl::vector<Vector4> mNewParticleData;
    };

    RndParticleCom();  // 0x601940
    // Copies the properties; the runtime data starts afresh. _Imprint uses
    // it. Not reconstructed: the waveforms' copies are not modelled, nor is
    // the base's copy, which is inlined.
    RndParticleCom(const RndParticleCom& other);  // 0x619130
    // Slots 0-1: 0x601B90, 0x601E30.
    ~RndParticleCom() override;

    Symbol GetId() const override;         // slot 4: 0x618140
    Symbol GetClassName() const override;  // slot 5: 0x618150
    int CurrentRev() const override;       // slot 7: 0x618160
    bool IsA(Symbol type) const override;  // slot 8: 0x618180
    Component* AsComponent() override;     // slot 9: 0x6181B0
    // Slot 10. The map's _Imprint(char*, Component*&, bool). The copy
    // constructor it calls is not reconstructed.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6181C0
    // Slot 18 at 0x60D6F0: the base's order, and the system follows the
    // colliders.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6182D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6182E0
    // Slot 27 at 0x60D530: the colliders' objects poll first.
    void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) override;
    // Slot 29 at 0x60D7C0: the map's _LoadResources(ObjPtr const&). Caches
    // the related components and the clock, loads the mesh and allocates
    // the particles.
    bool _OnResourcesLoaded() override;
    // Slot 31 at 0x60F550: starts without particles.
    void _Enter() override;
    // Slot 33 at 0x60F5E0: the map's _Poll(ObjPtr const&).
    void _Poll() override;
    // Slot 41 at 0x60DE00: one sprite instance, or one mesh instance per
    // particle, at the levels in "lods".
    unsigned long _GetNumDrawInstancesImpl(RndSceneLod lod) const override;
    // Slot 42 at 0x60DE30: the lit default material for meshes, the
    // particle material for sprites.
    RndMaterialCom* _GetDefaultMaterial() const override;
    // Slot 43 at 0x60DE60.
    void _InitDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) override;
    // Slot 44: fills the sprite instance with the buffer, or one instance per
    // live particle with the mesh and the particle's transform, color and
    // extra data. Not reconstructed: the instance transforms are built from
    // the waveform-driven attributes with SIMD code that is not decoded yet.
    void _SyncDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) override;  // 0x60DE70
    // Slot 45 at 0x60F540: false. Name not in the reference map.
    bool _DrawsGeometry() const override;

    // Creates and destroys the shared box mesh. Rnd::Init and
    // Rnd::Terminate call them. Names not in the reference map.
    static void StaticInit();       // 0x601880
    static void StaticTerminate();  // 0x601910

    // Recreates the sprite buffer for "max_count" and grows the collection.
    // The map's _AllocateBuffers() has no argument; this build passes the
    // object's name.
    void _AllocateBuffers(const char* name);  // 0x60DCD0
    // Points every waveform at the system's generator, or at gRand without
    // one. Not reconstructed: it writes into the opaque waveforms.
    void _InstallRand();  // 0x60DD40
    // Kills every particle. Name not in the reference map; inlined into
    // _Enter.
    void _KillAllParticles();  // 0x60F5A0
    // Kills every particle and frees the chunks; inlined into _Poll.
    void _DeallocAllParticles();  // 0x60FA20
    // Ages, moves and spawns the particles: `emitXfm` is where new particles
    // start, `simXfm` the transform into the particles' space for the
    // affectors and colliders.
    void _UpdateParticles(const Transform& emitXfm, const Transform& simXfm);  // 0x60FA70
    // Fits the draw node's sphere to the live particles.
    void _UpdateBoundingSphere();  // 0x60FDF0
    // Kills the particles past their death time unless "never_kill". The
    // map's _UpdateDeath(); this copy also frees the unused chunks and is
    // not called in this build (_UpdateParticles inlines it).
    void _UpdateDeath();  // 0x610070
    // Accelerates the particles by "force_direction" and runs the enabled
    // affectors (slot 41).
    void _UpdateForce(const Transform& simXfm);  // 0x610230
    // Applies "drag" and "speed_multiplier_over_life", moves the particles and
    // runs the colliders. Not reconstructed: the multiplier is evaluated per
    // particle through Waveform<float> (0x2D7C60).
    void _UpdatePositions(const Transform& simXfm);  // 0x610490
    // Runs the affectors' slot 42 and frees the unused chunks. Not called in
    // this build (_UpdateParticles inlines it). Name not in the reference
    // map.
    void _UpdateAffectors(const Transform& simXfm);  // 0x6108A0
    // Turns the particles by their RPM over life. Not reconstructed:
    // waveform-driven SIMD code.
    void _UpdateSpin();  // 0x6109D0
    // Sets the particles' pivots over life. Not reconstructed: waveform-
    // driven SIMD code.
    void _UpdatePivot();  // 0x6113D0
    // Sets the particles' color, alpha and scale over life. Not
    // reconstructed: waveform-driven SIMD code.
    void _UpdateColorSize();  // 0x611640
    // Evaluates the "Over Life" extra data channels. Not reconstructed:
    // waveform-driven SIMD code. Name not in the reference map.
    void _UpdateExtraData();  // 0x611B60
    // Queues the frame's births, makes room by killing the oldest particles
    // when allowed, creates the queued particles and initializes them.
    void _SpawnParticles(const Transform& emitXfm);  // 0x611F50
    // Queues a particle for every "emit_distance_per_particle" the emitter
    // moved, along the path.
    void _UpdateDistanceEmission(const Transform& emitXfm);  // 0x612080
    // Queues the "emit_rate" births, split into "subsamples" steps along the
    // emitter's estimated path. Not reconstructed: it evaluates the rate,
    // pitch, yaw and speed waveforms.
    void _UpdateRateEmission(const Transform& emitXfm);  // 0x6122C0
    // Kills one particle.
    void _KillParticle(unsigned long particle);  // 0x612890
    // Initializes the particles created from the queue: position, velocity,
    // life, size, rotation and the per-particle waveform data. Not
    // reconstructed: waveform-driven SIMD code.
    void _InitParticles();  // 0x6128C0
    // Queues one birth; false when the system is full.
    bool _QueueNewParticle(const Transform& xfm);  // 0x617630
    // Queues one step's births: the rate over the step, the remainder and
    // the burst. Not reconstructed: it evaluates the "emit_rate" waveform.
    // Name weakly supported: the map's _UpdateBirth(Transform const&).
    void _UpdateBirth(const Transform& xfm);  // 0x617A40
    // Queues up to `count` births; returns how many were queued.
    unsigned long _QueueNewParticles(unsigned long count, const Transform& xfm);  // 0x617BC0
    // Moves the queued births onto the emitter's shape: the object's
    // RndParticleEmitterCom when it has one, otherwise a random point of the
    // emission box. Name not in the reference map.
    void _PlaceNewParticles();  // 0x617D20
    // The "gather_all" actions: fill "colliders" and "affectors" with every
    // object of the entity that has one. Names not in the reference map.
    void GatherColliders();  // 0x617F30
    void GatherAffectors();  // 0x618040

    // Registers the class. Not reconstructed: the registration helpers it
    // inlines are not modelled.
    static void Init();  // 0x3FA5F0
    // The class factory, emitted with Init.
    static Component* _Create();  // 0x405B30
    // Registers the class description, the defaults of the waveforms and the
    // properties. Not reconstructed: the waveform resources and the property
    // metadata's attributes are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x603850

    // The object's statics, in the order of its static initializer
    // (0x61DF50). The three ints it first sets (0x1AA8B58: -1, the invalid
    // GameObjectId; 0x1AA8B5C: 8; 0x1AA8B60: 4) come from a shared header
    // and are not modelled.
    static Symbol sId;  // 0x1AA8B68, "PSys"
    // The class's second symbol, also "PSys". Name not in the reference map.
    static Symbol sClassName;           // 0x1AA8B70
    static PropRegistry sPropRegistry;  // 0x1AA8B80
    static ComMetaData sMetaData;       // 0x1AA8C20
    // The default waveforms _Init builds, which the parameter constructors
    // copy. In the binary they are ResourcePtr<WaveformResource<float>>
    // (ResourcePtr<WaveformResource<Hmx::Color>> for sDefaultColor), whose
    // destructors the initializer registers; WaveformResource is not
    // modelled, so they are typed by the base. Names not in the reference
    // map.
    static ResourcePtr<Resource> sDefaultPitch;                  // 0x1AA8DC8
    static ResourcePtr<Resource> sDefaultYaw;                    // 0x1AA8DD0
    static ResourcePtr<Resource> sDefaultEmitRate;               // 0x1AA8DD8
    static ResourcePtr<Resource> sDefaultSpeedMultiplierOverLife;  // 0x1AA8DE0
    static ResourcePtr<Resource> sDefaultLifespan;               // 0x1AA8DE8
    static ResourcePtr<Resource> sDefaultSpeed;                  // 0x1AA8DF0
    static ResourcePtr<Resource> sDefaultInitialSize;            // 0x1AA8DF8
    static ResourcePtr<Resource> sDefaultPivot;                  // 0x1AA8E00
    static ResourcePtr<Resource> sDefaultZPivot;                 // 0x1AA8E08
    static ResourcePtr<Resource> sDefaultScaleOverLife;          // 0x1AA8E10
    static ResourcePtr<Resource> sDefaultColor;                  // 0x1AA8E18
    static ResourcePtr<Resource> sDefaultAlpha;                  // 0x1AA8E20
    static ResourcePtr<Resource> sDefaultXRotationAngle;         // 0x1AA8E28
    static ResourcePtr<Resource> sDefaultYRotationAngle;         // 0x1AA8E30
    static ResourcePtr<Resource> sDefaultRotationAngle;          // 0x1AA8E38
    static ResourcePtr<Resource> sDefaultExtraData;              // 0x1AA8E40
    // The unit box StaticInit creates, drawn by a kMesh system without a
    // mesh path. Name not in the reference map.
    static RndMesh* sDefaultMesh;  // 0x1AA8E48

    EmitterParams mEmitter;
    ParticleParams mParticle;
    ColorParams mColor;
    SpinParams mSpin;
    ExtraDataParams mExtraData;
    RenderParams mRender;
    // "time_units": the clock's units the system runs on.
    TimeUnits mTimeUnits;
    // "sim_when_hidden": keep simulating while the draw node is hidden.
    bool mSimWhenHidden;
    CollisionParams mCollision;
    // "affectors": the RndParticleAffectorCom objects of the entity.
    PropArray<GameObjectId> mAffectors;
    RuntimeData mRuntime;
};

static_assert(sizeof(RndParticleCom::EmitterParams) == 0x80);
static_assert(offsetof(RndParticleCom::EmitterParams, mNeverKill) == 0x5);
static_assert(offsetof(RndParticleCom::EmitterParams, mMinBoxExtents) == 0x8);
static_assert(offsetof(RndParticleCom::EmitterParams, mMaxBoxExtents) == 0x14);
static_assert(offsetof(RndParticleCom::EmitterParams, mPitch) == 0x20);
static_assert(offsetof(RndParticleCom::EmitterParams, mYaw) == 0x48);
static_assert(offsetof(RndParticleCom::EmitterParams, mBirthMomentum) == 0x70);
static_assert(offsetof(RndParticleCom::EmitterParams, mSubsamples) == 0x78);
static_assert(offsetof(RndParticleCom::EmitterParams, mLocalSpace) == 0x7C);
static_assert(sizeof(RndParticleCom::ParticleParams) == 0x200);
static_assert(offsetof(RndParticleCom::ParticleParams, mMeshResourcePath) == 0x8);
static_assert(offsetof(RndParticleCom::ParticleParams, mMaxCount) == 0x10);
static_assert(offsetof(RndParticleCom::ParticleParams, mEmitRate) == 0x18);
static_assert(offsetof(RndParticleCom::ParticleParams, mEmitRateMultiplier) == 0x40);
static_assert(offsetof(RndParticleCom::ParticleParams, mEmitDistancePerParticle) == 0x44);
static_assert(offsetof(RndParticleCom::ParticleParams, mLifespan) == 0x48);
static_assert(offsetof(RndParticleCom::ParticleParams, mLifespanMultiplier) == 0x70);
static_assert(offsetof(RndParticleCom::ParticleParams, mSpeed) == 0x78);
static_assert(offsetof(RndParticleCom::ParticleParams, mSpeedMultiplierOverLife) == 0xA0);
static_assert(offsetof(RndParticleCom::ParticleParams, mDrag) == 0xC8);
static_assert(offsetof(RndParticleCom::ParticleParams, mInitialSize) == 0xD0);
static_assert(offsetof(RndParticleCom::ParticleParams, mZScaleOverLife) == 0x148);
static_assert(offsetof(RndParticleCom::ParticleParams, mUniformScale) == 0x170);
static_assert(offsetof(RndParticleCom::ParticleParams, mXPivotOverLife) == 0x178);
static_assert(offsetof(RndParticleCom::ParticleParams, mZPivotOverLife) == 0x1C8);
static_assert(offsetof(RndParticleCom::ParticleParams, mOffsetThenScale) == 0x1F0);
static_assert(offsetof(RndParticleCom::ParticleParams, mForceDirection) == 0x1F4);
static_assert(sizeof(RndParticleCom::ColorParams) == 0x50);
static_assert(sizeof(RndParticleCom::SpinParams) == 0x130);
static_assert(offsetof(RndParticleCom::SpinParams, mInitialXRotationAngle) == 0x8);
static_assert(offsetof(RndParticleCom::SpinParams, mInitialRotationAngle) == 0x58);
static_assert(offsetof(RndParticleCom::SpinParams, mOrientation) == 0x80);
static_assert(offsetof(RndParticleCom::SpinParams, mXRpmOverLife) == 0xA8);
static_assert(offsetof(RndParticleCom::SpinParams, mRpmOverLife) == 0xF8);
static_assert(offsetof(RndParticleCom::SpinParams, mXRpmDrag) == 0x120);
static_assert(offsetof(RndParticleCom::SpinParams, mRpmDrag) == 0x128);
static_assert(sizeof(RndParticleCom::ExtraDataParams) == 0x88);
static_assert(offsetof(RndParticleCom::ExtraDataParams, mExtraData0) == 0x10);
static_assert(offsetof(RndParticleCom::ExtraDataParams, mExtraData2) == 0x60);
static_assert(sizeof(RndParticleCom::RenderParams) == 8);
static_assert(sizeof(RndParticleCom::CollisionParams) == 0x30);
static_assert(offsetof(RndParticleCom::CollisionParams, mColliders) == 0x8);
static_assert(offsetof(RndParticleCom::RuntimeData, mExtensions) == 0x8);
static_assert(offsetof(RndParticleCom::RuntimeData, mMeshResource) == 0x28);
static_assert(offsetof(RndParticleCom::RuntimeData, mClock) == 0x38);
static_assert(offsetof(RndParticleCom::RuntimeData, mDrawNode) == 0x48);
static_assert(offsetof(RndParticleCom::RuntimeData, mTime) == 0x50);
static_assert(offsetof(RndParticleCom::RuntimeData, mLastPosition) == 0x54);
static_assert(offsetof(RndParticleCom::RuntimeData, mLastXfm) == 0x60);
static_assert(offsetof(RndParticleCom::RuntimeData, mNumActive) == 0x90);
static_assert(offsetof(RndParticleCom::RuntimeData, mBurstCountMin) == 0x9C);
static_assert(offsetof(RndParticleCom::RuntimeData, mParticles) == 0xA8);
static_assert(offsetof(RndParticleCom::RuntimeData, mRand) == 0x738);
static_assert(offsetof(RndParticleCom::RuntimeData, mParticleXfm) == 0x748);
static_assert(offsetof(RndParticleCom::RuntimeData, mVelocityAligned) == 0x778);
static_assert(offsetof(RndParticleCom::RuntimeData, mNewParticleXfms) == 0x780);
static_assert(offsetof(RndParticleCom::RuntimeData, mNewParticleData) == 0x7A0);
static_assert(sizeof(RndParticleCom::RuntimeData) == 0x7C0);
static_assert(offsetof(RndParticleCom, mEmitter) == 0xC0);
static_assert(offsetof(RndParticleCom, mParticle) == 0x140);
static_assert(offsetof(RndParticleCom, mColor) == 0x340);
static_assert(offsetof(RndParticleCom, mSpin) == 0x390);
static_assert(offsetof(RndParticleCom, mExtraData) == 0x4C0);
static_assert(offsetof(RndParticleCom, mRender) == 0x548);
static_assert(offsetof(RndParticleCom, mTimeUnits) == 0x550);
static_assert(offsetof(RndParticleCom, mSimWhenHidden) == 0x554);
static_assert(offsetof(RndParticleCom, mCollision) == 0x558);
static_assert(offsetof(RndParticleCom, mAffectors) == 0x588);
static_assert(offsetof(RndParticleCom, mRuntime) == 0x5B0);
static_assert(sizeof(RndParticleCom) == 0xD70);
