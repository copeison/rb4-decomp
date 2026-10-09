// render/RndParticleCom.o (0x601880 to 0x61E1F5). The object also emits the
// extension list's insert and erase (0x601E50, 0x601F60), the copies of the
// parameter structures (0x619580-0x61A66B), the per-chunk waveform helpers
// (0x614AF0, 0x615970), Waveform<float>::ValueRange (0x6178D0),
// WaveformResource<float>::ValueRange (0x61DD30), the resize of the
// new-particle data (0x61DE40) and the registry's accessors and lambdas
// (0x618540-0x61A1A0, 0x61A670-0x61DD2F); they are not reconstructed.
#include "render/particles/RndParticleCom.h"

#include <cfloat>
#include <cmath>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransCom.h"
#include "math/geometry/Sphere.h"
#include "math/random/Rand.h"
#include "render/buffers/RndParticleBuffer.h"
#include "render/defaults/RndDefaults.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshResource.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/particles/RndParticleAffectorCom.h"
#include "render/particles/RndParticleColliderCom.h"
#include "render/particles/RndParticleEmitterCom.h"
#include "render/particles/RndParticleExtCom.h"
#include "render/particles/RndParticleVelocityAlignCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/system/RndDevice.h"

// The object's statics, in the order of its static initializer (0x61DF50).
Symbol RndParticleCom::sId("PSys");
Symbol RndParticleCom::sClassName("PSys");
PropRegistry RndParticleCom::sPropRegistry;
ComMetaData RndParticleCom::sMetaData;
ResourcePtr<Resource> RndParticleCom::sDefaultPitch;
ResourcePtr<Resource> RndParticleCom::sDefaultYaw;
ResourcePtr<Resource> RndParticleCom::sDefaultEmitRate;
ResourcePtr<Resource> RndParticleCom::sDefaultSpeedMultiplierOverLife;
ResourcePtr<Resource> RndParticleCom::sDefaultLifespan;
ResourcePtr<Resource> RndParticleCom::sDefaultSpeed;
ResourcePtr<Resource> RndParticleCom::sDefaultInitialSize;
ResourcePtr<Resource> RndParticleCom::sDefaultPivot;
ResourcePtr<Resource> RndParticleCom::sDefaultZPivot;
ResourcePtr<Resource> RndParticleCom::sDefaultScaleOverLife;
ResourcePtr<Resource> RndParticleCom::sDefaultColor;
ResourcePtr<Resource> RndParticleCom::sDefaultAlpha;
ResourcePtr<Resource> RndParticleCom::sDefaultXRotationAngle;
ResourcePtr<Resource> RndParticleCom::sDefaultYRotationAngle;
ResourcePtr<Resource> RndParticleCom::sDefaultRotationAngle;
ResourcePtr<Resource> RndParticleCom::sDefaultExtraData;
RndMesh* RndParticleCom::sDefaultMesh;

namespace {

// The invalid GameObjectId, the shared header's -1 (0x1AA8B58). Name not in
// the reference map.
constexpr unsigned int kInvalidObjectId = 0xFFFFFFFF;

// The component with base class T on the object with the id, or null. The
// lookup is inlined into each loop over "affectors". Name not in the
// reference map.
template <typename T>
T* FindBaseCom(const Entity* entity, GameObjectId id) {
    if (id.mId == kInvalidObjectId) {
        return nullptr;
    }
    const GameObject* object = entity->GetObject(id);
    if (object == nullptr) {
        return nullptr;
    }
    return object->GetBaseCom<T>();
}

}  // namespace

// Reconstructed from eboot.elf at 0x601880. A unit box on the axes.
void RndParticleCom::StaticInit() {
    RndMeshUtl::CreateBoxParams params;
    params.mAxisX = {1.0F, 0.0F, 0.0F};
    params.mAxisY = {0.0F, 1.0F, 0.0F};
    params.mAxisZ = {0.0F, 0.0F, 1.0F};
    params.mNumSegmentsX = 1;
    params.mNumSegmentsY = 1;
    params.mNumSegmentsZ = 1;
    sDefaultMesh = RndMeshUtl::CreateBox(params);
}

// Reconstructed from eboot.elf at 0x601910.
void RndParticleCom::StaticTerminate() {
    delete sDefaultMesh;
    sDefaultMesh = nullptr;
}

// Reconstructed from eboot.elf at 0x601940.
RndParticleCom::RndParticleCom()
    : RndDrawInstanceCom(),
      mEmitter(),
      mParticle(),
      mColor(),
      mSpin(),
      mExtraData(),
      mRender(),
      mTimeUnits(kTimeUnitsSeconds),
      mSimWhenHidden(false),
      mCollision(),
      mAffectors(),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x601B90. The members release their
// resources; the base leaves the scene drawer's list.
RndParticleCom::~RndParticleCom() {}

// Reconstructed from eboot.elf at 0x6036F0. The cached components and the
// mesh are left for _OnResourcesLoaded to set.
RndParticleCom::RuntimeData::RuntimeData()
    : mParticleTypeChanged(false),
      mExtensions(),
      mMeshResource(),
      mClock(nullptr),
      mTime(0.0F),
      mLastPosition(Vector3::sZero),
      mNumActive(0),
      mRateRemainder(0.0F),
      mDistanceTravelled(0.0F),
      mBurstCountMin(0),
      mBurstCountMax(0),
      mParticles(),
      mRand(nullptr),
      mBuffer(nullptr),
      mVelocityAligned(false),
      mNewParticleXfms(),
      mNewParticleData() {
    mLastXfm.m.x = {1.0F, 0.0F, 0.0F};
    mLastXfm.m.y = {0.0F, 1.0F, 0.0F};
    mLastXfm.m.z = {0.0F, 0.0F, 1.0F};
    mLastXfm.v = {0.0F, 0.0F, 0.0F};
    mParticleXfm.m.x = {1.0F, 0.0F, 0.0F};
    mParticleXfm.m.y = {0.0F, 1.0F, 0.0F};
    mParticleXfm.m.z = {0.0F, 0.0F, 1.0F};
    mParticleXfm.v = {0.0F, 0.0F, 0.0F};
    // The seed is the structure's address, truncated to an int.
    mRand = new Rand(static_cast<long>(static_cast<int>(reinterpret_cast<long>(this))));
}

// Inlined into ~RndParticleCom (0x601B90).
RndParticleCom::RuntimeData::~RuntimeData() {
    delete mRand;
    mRand = nullptr;
    delete mBuffer;
    mBuffer = nullptr;
}

// Reconstructed from eboot.elf at 0x60D530.
void RndParticleCom::_GetPollDeps(
    eastl::vector<GameObjectId>& before,
    eastl::vector<GameObjectId>& after) {
    static_cast<void>(after);
    const PropArray<GameObjectId>& colliders = mCollision.mColliders;
    before.reserve(before.size() + colliders.mSize);
    for (const GameObjectId& id : colliders) {
        before.push_back(id);
    }
}

// Reconstructed from eboot.elf at 0x60D6F0.
void RndParticleCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    RndDrawInstanceCom::_GetComponentOrderDeps(follows, precedes);
    follows.push_back(RndParticleColliderCom::sClassName);
}

// Reconstructed from eboot.elf at 0x60D7C0. A mesh path is looked up first
// among the resources inlined into the entity's layers, then loaded. The
// binary inlines _AllocateBuffers and _InstallRand.
bool RndParticleCom::_OnResourcesLoaded() {
    if (!RndDrawInstanceCom::_OnResourcesLoaded()) {
        return false;
    }
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    mRuntime.mClock = TheTimeMgr->GetClock(mObject->mEntity);

    RndMesh* mesh = nullptr;
    if (mParticle.mParticleType != kMesh) {
        mRuntime.mMeshResource = nullptr;
    } else if (mParticle.mMeshResourcePath == ResourcePath()) {
        mRuntime.mMeshResource = nullptr;
        mesh = sDefaultMesh;
    } else {
        const ResourcePath& path = mParticle.mMeshResourcePath;
        Resource* inlined = nullptr;
        for (const EntityResource::LayerInfo& layer : mObject->mEntity->mResource->mLayers) {
            for (Resource* resource : layer.mInlineResources) {
                if (resource->mPath == path) {
                    inlined = resource;
                    break;
                }
            }
            if (inlined != nullptr) {
                break;
            }
        }
        if (inlined != nullptr) {
            mRuntime.mMeshResource =
                ResourcePtr<RndMeshResource>(static_cast<RndMeshResource*>(inlined));
        } else {
            mRuntime.mMeshResource = Resource::GetOrLoad<RndMeshResource>(path, false);
        }
        RndMeshResource* resource = mRuntime.mMeshResource;
        if (resource != nullptr && !resource->Fail()) {
            mesh = resource->mMesh;
        }
    }
    mRuntime.mMesh = mesh;

    _AllocateBuffers(mObject->mName.Str());
    _InstallRand();
    mRuntime.mNewParticleXfms.reserve(mParticle.mMaxCount);
    mRuntime.mNewParticleData.reserve(mParticle.mMaxCount);
    return true;
}

// Reconstructed from eboot.elf at 0x60DCD0.
void RndParticleCom::_AllocateBuffers(const char* name) {
    delete mRuntime.mBuffer;
    mRuntime.mBuffer = nullptr;
    mRuntime.mParticles.SetMaxCount(mParticle.mMaxCount);
    if (mParticle.mParticleType == kSprite) {
        mRuntime.mBuffer = RndParticleBuffer::New(mParticle.mMaxCount, name);
    }
}

// Reconstructed from eboot.elf at 0x60DE00.
unsigned long RndParticleCom::_GetNumDrawInstancesImpl(RndSceneLod lod) const {
    if ((mLods >> (lod & 31) & 1) == 0) {
        return 0;
    }
    if (mParticle.mParticleType == kMesh) {
        return mRuntime.mParticles.mMaxCount;
    }
    return 1;
}

// Reconstructed from eboot.elf at 0x60DE30.
RndMaterialCom* RndParticleCom::_GetDefaultMaterial() const {
    const RndDefaults& defaults = gRndDevice->mDefaults;
    return mParticle.mParticleType == kMesh ? defaults.mLitMaterial : defaults.mParticleMaterial;
}

// Reconstructed from eboot.elf at 0x60DE60.
void RndParticleCom::_InitDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) {
    _SyncInstanceStateFlags(instances);
}

// Reconstructed from eboot.elf at 0x60F540.
bool RndParticleCom::_DrawsGeometry() const {
    return false;
}

// Reconstructed from eboot.elf at 0x60F550. The binary inlines
// _KillAllParticles.
void RndParticleCom::_Enter() {
    _KillAllParticles();
    RndDrawInstanceCom::_Enter();
}

// Reconstructed from eboot.elf at 0x60F5A0.
void RndParticleCom::_KillAllParticles() {
    while (mRuntime.mParticles.mNumParticles != 0) {
        mRuntime.mParticles.DeleteParticle(0);
        --mRuntime.mNumActive;
    }
}

// Reconstructed from eboot.elf at 0x60F5E0. A hidden system that does not
// "sim_when_hidden" empties its sphere and its particles. Otherwise the
// system advances its time and runs in the object's space ("local_space")
// or in world space, then hands the buffer what it draws and polls its
// extensions. The buffer gets last frame's velocity alignment.
void RndParticleCom::_Poll() {
    if (!mSimWhenHidden && (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) != 0) {
        mRuntime.mDrawNode->SetLocalSphere(Sphere::sZero);
        if (mRuntime.mParticles.mNumChunks * 16 != 0) {
            _DeallocAllParticles();
        }
        RndDrawInstanceCom::_Poll();
        return;
    }

    mRuntime.mTime += mRuntime.mClock->DeltaTime(mTimeUnits);
    Transform emitXfm;
    Transform simXfm;
    if (mEmitter.mLocalSpace) {
        emitXfm = Transform::sID;
        simXfm = RndDrawInstanceCom::mRuntime.mDrawNode->mRuntime.mInvWorldXfm;
        mRuntime.mParticleXfm = mRuntime.mTrans->mWorldXfm;
    } else {
        emitXfm = mRuntime.mTrans->mWorldXfm;
        simXfm = Transform::sID;
        mRuntime.mParticleXfm = Transform::sID;
    }
    _UpdateParticles(emitXfm, simXfm);
    _UpdateBoundingSphere();

    RndParticleBuffer* buffer = mRuntime.mBuffer;
    if (buffer != nullptr) {
        buffer->mParticles = &mRuntime.mParticles;
        buffer->mXfm = &mRuntime.mParticleXfm;
        buffer->mAlignment = mRender.mParticleAlignment;
        buffer->mSortMode = mRender.mSortMethod;
        buffer->mVelocityAligned = mRuntime.mVelocityAligned;
        buffer->mWorldSpace = mParticle.mOffsetThenScale;
        buffer->mHasRotation = mExtraData.mExtraDataType0 != kExtraDataDisabled ||
                               mExtraData.mExtraDataType1 != kExtraDataDisabled ||
                               mExtraData.mExtraDataType2 != kExtraDataDisabled;
    }
    mRuntime.mVelocityAligned = mObject->GetCom<RndParticleVelocityAlignCom>() != nullptr;
    mRuntime.mLastPosition = emitXfm.v;
    for (RndParticleExtCom* extension : mRuntime.mExtensions) {
        extension->_PollExtension();
    }
    RndDrawInstanceCom::_Poll();
}

// Reconstructed from eboot.elf at 0x60FA20.
void RndParticleCom::_DeallocAllParticles() {
    _KillAllParticles();
    mRuntime.mParticles.DeleteAllParticles();
}

// Reconstructed from eboot.elf at 0x60FA70. Nothing happens on a frame
// without time. Particles die past their death time, or when the clock went
// back before their birth, unless "never_kill"; the binary inlines
// _UpdateDeath and _UpdateAffectors without their chunk frees. The life
// fraction is computed four particles at a time with a refined reciprocal.
// Sprites aligned to their velocity keep their rotation.
void RndParticleCom::_UpdateParticles(const Transform& emitXfm, const Transform& simXfm) {
    const float dt = mRuntime.mClock->DeltaTime(mTimeUnits);
    if (!(std::fabs(dt) > 1.0e-4F)) {
        return;
    }
    RndParticleCollection& particles = mRuntime.mParticles;
    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        while (!mEmitter.mNeverKill) {
            const float time = mRuntime.mTime;
            if (!(time >= particles.mDeathTimes[i]) &&
                time >= particles.mBirthTimes[i]) {
                break;
            }
            particles.DeleteParticle(i);
            --mRuntime.mNumActive;
            if (i >= particles.mNumParticles) {
                break;
            }
        }
    }
    particles.FreeUnusedChunks();

    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        const float birth = particles.mBirthTimes[i];
        const float death = particles.mDeathTimes[i];
        float fraction = (mRuntime.mTime - birth) / (death - birth);
        fraction = fraction < 1.0F ? fraction : 1.0F;
        fraction = fraction > 0.0F ? fraction : 0.0F;
        particles.mLifeFractions[i] = fraction;
    }

    _UpdateForce(simXfm);
    _UpdatePositions(simXfm);
    const Entity* entity = mObject->mEntity;
    for (const GameObjectId& id : mAffectors) {
        RndParticleAffectorCom* affector = FindBaseCom<RndParticleAffectorCom>(entity, id);
        if (affector != nullptr && affector->mEnabled) {
            affector->_PostAffectParticles(*this, simXfm);
        }
    }
    particles.FreeUnusedChunks();

    if (!mRuntime.mVelocityAligned || mParticle.mParticleType == kMesh) {
        _UpdateSpin();
    }
    _UpdatePivot();
    _UpdateColorSize();
    _UpdateExtraData();
    _SpawnParticles(emitXfm);
}

// Reconstructed from eboot.elf at 0x60FDF0. The sphere encloses each live
// particle's box of its largest size; without particles it is a point at
// the particle transform's position. A world-space system's sphere moves
// into the object's space.
void RndParticleCom::_UpdateBoundingSphere() {
    Sphere sphere;
    const int numActive = mRuntime.mNumActive;
    if (numActive <= 0) {
        sphere.center = mRuntime.mParticleXfm.v;
        sphere.radius = 0.0F;
    } else {
        const RndParticleCollection& particles = mRuntime.mParticles;
        Vector3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        for (unsigned long i = 0; i < static_cast<unsigned long>(numActive); ++i) {
            float extent = particles.mSizeY[i];
            extent = std::fmax(extent, particles.mSizeX[i]);
            extent = std::fmax(extent, particles.mSizeZ[i]);
            const float x = particles.mPosX[i];
            const float y = particles.mPosY[i];
            const float z = particles.mPosZ[i];
            min.x = std::fmin(x - extent, min.x);
            min.y = std::fmin(y - extent, min.y);
            min.z = std::fmin(z - extent, min.z);
            max.x = std::fmax(extent + x, max.x);
            max.y = std::fmax(extent + y, max.y);
            max.z = std::fmax(extent + z, max.z);
        }
        sphere.SetFromBox(min, max);
    }
    if (!mEmitter.mLocalSpace) {
        mRuntime.mDrawNode->SetLocalSphereFromWorld(sphere);
    } else {
        mRuntime.mDrawNode->SetLocalSphere(sphere);
    }
}

// Reconstructed from eboot.elf at 0x610070.
void RndParticleCom::_UpdateDeath() {
    RndParticleCollection& particles = mRuntime.mParticles;
    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        while (!mEmitter.mNeverKill) {
            const float time = mRuntime.mTime;
            if (!(time >= particles.mDeathTimes[i]) &&
                time >= particles.mBirthTimes[i]) {
                break;
            }
            particles.DeleteParticle(i);
            --mRuntime.mNumActive;
            if (i >= particles.mNumParticles) {
                break;
            }
        }
    }
    particles.FreeUnusedChunks();
}

// Reconstructed from eboot.elf at 0x610230. The binary adds the force to
// four particles at a time. Affectors whose strength is within 1e-4 of zero
// are skipped.
void RndParticleCom::_UpdateForce(const Transform& simXfm) {
    const float dt = mRuntime.mClock->DeltaTime(mTimeUnits);
    RndParticleCollection& particles = mRuntime.mParticles;
    const Vector3& force = mParticle.mForceDirection;
    const float dx = dt * force.x;
    const float dy = dt * force.y;
    const float dz = dt * force.z;
    for (unsigned long i = 0; i < particles.mNumParticles; ++i) {
        particles.mVelX[i] += dx;
        particles.mVelY[i] += dy;
        particles.mVelZ[i] += dz;
    }
    const Entity* entity = mObject->mEntity;
    for (const GameObjectId& id : mAffectors) {
        RndParticleAffectorCom* affector = FindBaseCom<RndParticleAffectorCom>(entity, id);
        if (affector != nullptr && affector->mEnabled &&
            std::fabs(affector->mForceStrength) > 1.0e-4F) {
            affector->_AffectParticles(particles, simXfm, dt);
        }
    }
}

// Reconstructed from eboot.elf at 0x6108A0.
void RndParticleCom::_UpdateAffectors(const Transform& simXfm) {
    const Entity* entity = mObject->mEntity;
    for (const GameObjectId& id : mAffectors) {
        RndParticleAffectorCom* affector = FindBaseCom<RndParticleAffectorCom>(entity, id);
        if (affector != nullptr && affector->mEnabled) {
            affector->_PostAffectParticles(*this, simXfm);
        }
    }
    mRuntime.mParticles.FreeUnusedChunks();
}

// Reconstructed from eboot.elf at 0x611F50. When the oldest may die, the
// oldest live particles make room for the whole queue. A queue the
// collection cannot hold is cut where creation failed.
void RndParticleCom::_SpawnParticles(const Transform& emitXfm) {
    _UpdateDistanceEmission(emitXfm);
    _UpdateRateEmission(emitXfm);
    RndParticleCollection& particles = mRuntime.mParticles;
    eastl::vector<Transform>& births = mRuntime.mNewParticleXfms;
    if (mEmitter.mEmitCanKillOldest) {
        int excess = static_cast<int>(births.size()) + mRuntime.mNumActive - mParticle.mMaxCount;
        for (; excess > 0; --excess) {
            particles.DeleteParticle(particles.mFirstParticle);
            --mRuntime.mNumActive;
        }
    }
    for (Transform* birth = births.begin(); birth != births.end(); ++birth) {
        if (particles.CreateParticle() == static_cast<unsigned long>(-1)) {
            births.erase(birth, births.end());
            break;
        }
        ++mRuntime.mNumActive;
    }
    _InitParticles();
}

// Reconstructed from eboot.elf at 0x612080. Births are spaced
// "emit_distance_per_particle" apart along the straight path from the last
// position; a jump of more than 128 spacings emits along it in 128 steps,
// and an absurd one (a step over 1e6) is dropped. The binary measures the
// path with a refined reciprocal square root.
void RndParticleCom::_UpdateDistanceEmission(const Transform& emitXfm) {
    const float spacing = mParticle.mEmitDistancePerParticle;
    if (!(spacing > 0.0F)) {
        return;
    }
    const float rateMultiplier = mParticle.mEmitRateMultiplier;
    if (!(std::fabs(rateMultiplier) > 1.0e-4F)) {
        return;
    }
    const Vector3& last = mRuntime.mLastPosition;
    const Vector3 delta = {emitXfm.v.x - last.x, emitXfm.v.y - last.y, emitXfm.v.z - last.z};
    const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (!std::isfinite(distance)) {
        return;
    }
    const float previous = mRuntime.mDistanceTravelled;
    float travelled = previous + distance;
    mRuntime.mDistanceTravelled = travelled;
    float step;
    if ((travelled - spacing) / spacing > 128.0F) {
        step = distance * (1.0F / 128.0F);
        if (step > 1.0e6F) {
            mRuntime.mDistanceTravelled = 0.0F;
            return;
        }
    } else {
        step = spacing / rateMultiplier;
    }

    Transform birth = emitXfm;
    float target = spacing;
    if (!(target > travelled)) {
        do {
            const float span = travelled - previous;
            if (span == 0.0F) {
                break;
            }
            const float t = (target - previous) / span;
            birth.v.x = delta.x * t + last.x;
            birth.v.y = delta.y * t + last.y;
            birth.v.z = delta.z * t + last.z;
            if (!_QueueNewParticle(birth)) {
                break;
            }
            target += step;
            travelled -= step;
        } while (target <= mRuntime.mDistanceTravelled);
    }
    mRuntime.mDistanceTravelled = travelled;
}

// Reconstructed from eboot.elf at 0x612890.
void RndParticleCom::_KillParticle(unsigned long particle) {
    mRuntime.mParticles.DeleteParticle(particle);
    --mRuntime.mNumActive;
}

// Reconstructed from eboot.elf at 0x617630. A full queue that may kill the
// oldest drops its first birth.
bool RndParticleCom::_QueueNewParticle(const Transform& xfm) {
    eastl::vector<Transform>& births = mRuntime.mNewParticleXfms;
    const long maxCount = mParticle.mMaxCount;
    if (mEmitter.mEmitCanKillOldest) {
        if (static_cast<long>(births.size()) == maxCount) {
            births.erase(births.begin());
        }
    } else if (births.size() >= static_cast<unsigned long>(maxCount - mRuntime.mNumActive)) {
        return false;
    }
    births.push_back(xfm);
    return true;
}

// Reconstructed from eboot.elf at 0x617BC0. The queue grows once for the
// whole count.
unsigned long RndParticleCom::_QueueNewParticles(unsigned long count, const Transform& xfm) {
    int room = mParticle.mMaxCount;
    if (!mEmitter.mEmitCanKillOldest) {
        room -= mRuntime.mNumActive;
    }
    if (static_cast<unsigned long>(static_cast<long>(room)) < count) {
        count = room;
    }
    eastl::vector<Transform>& births = mRuntime.mNewParticleXfms;
    births.reserve(count + births.size());
    unsigned long queued = 0;
    for (; queued < count; ++queued) {
        if (!_QueueNewParticle(xfm)) {
            break;
        }
    }
    return queued;
}

// Reconstructed from eboot.elf at 0x617D20. Each queued birth moves by a
// random point of the box, in the birth transform's axes.
void RndParticleCom::_PlaceNewParticles() {
    RndParticleEmitterCom* emitter = mObject->GetBaseCom<RndParticleEmitterCom>();
    if (emitter != nullptr) {
        emitter->_PlaceNewParticles(mRuntime.mRand, mRuntime.mNewParticleXfms);
        return;
    }
    Rand& rand = *mRuntime.mRand;
    const Vector3& min = mEmitter.mMinBoxExtents;
    const Vector3& max = mEmitter.mMaxBoxExtents;
    for (Transform& birth : mRuntime.mNewParticleXfms) {
        const float x = rand.Float() * (max.x - min.x) + min.x;
        const float y = rand.Float() * (max.y - min.y) + min.y;
        const float z = rand.Float() * (max.z - min.z) + min.z;
        const Hmx::Matrix3& m = birth.m;
        birth.v.x = x * m.x.x + y * m.y.x + birth.v.x + z * m.z.x;
        birth.v.y = x * m.x.y + y * m.y.y + birth.v.y + z * m.z.y;
        birth.v.z = x * m.x.z + y * m.y.z + birth.v.z + z * m.z.z;
    }
}

// Reconstructed from eboot.elf at 0x617F30.
void RndParticleCom::GatherColliders() {
    PropArray<GameObjectId>& colliders = mCollision.mColliders;
    colliders.Resize(0);
    const Entity* entity = mObject->mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object->GetCom<RndParticleColliderCom>() != nullptr) {
            const GameObjectId id = object->mId;
            colliders._Insert(colliders.mSize, &id);
        }
    }
}

// Reconstructed from eboot.elf at 0x618040.
void RndParticleCom::GatherAffectors() {
    mAffectors.Resize(0);
    const Entity* entity = mObject->mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object->GetBaseCom<RndParticleAffectorCom>() != nullptr) {
            const GameObjectId id = object->mId;
            mAffectors._Insert(mAffectors.mSize, &id);
        }
    }
}

// Reconstructed from eboot.elf at 0x618140.
Symbol RndParticleCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x618150.
Symbol RndParticleCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x618160.
int RndParticleCom::CurrentRev() const {
    return const_cast<RndParticleCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x618180.
bool RndParticleCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6181B0.
Component* RndParticleCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6181C0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndParticleCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndParticleCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndParticleCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6182D0.
PropRegistry& RndParticleCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6182E0.
ComMetaData& RndParticleCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x405B30. The binary emits the factory
// with the class's Init (0x3FA5F0), before the renderer's components; the
// placement factory at 0x405B60 constructs in given storage.
Component* RndParticleCom::_Create() {
    return new RndParticleCom();
}
