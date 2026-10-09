#include "render/buffers/RndParticleBuffer.h"

#include <algorithm>
#include <cmath>

#include "math/color/Color.h"
#include "math/matrix/Matrix3.h"
#include "math/scalar/Trig.h"
#include "math/transform/Transform.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/meshes/RndVertex.h"
#include "render/particles/RndParticleCollection.h"
#include "render/system/RndFactory.h"

namespace {

constexpr float kHalfPi = 1.5707964F;

// The blend modes up to kScreen draw the particles in sorted order.
constexpr int kLastSortedBlendMode = 3;

// Each quad's corners, in vertex order, as signs along the right and up
// axes, and their texture coordinates.
struct QuadCorner {
    float mRight;
    float mUp;
    float mU;
    float mV;
};

constexpr QuadCorner kQuadCorners[4] = {
    {1.0F, 1.0F, 1.0F, 0.0F},
    {-1.0F, 1.0F, 0.0F, 0.0F},
    {-1.0F, -1.0F, 0.0F, 1.0F},
    {1.0F, -1.0F, 1.0F, 1.0F},
};

float ParticleFloat(
    const RndParticleCollection& particles,
    RndParticleCollection::Attribute attribute,
    unsigned long particle) {
    return particles[attribute].Get<float>(particle);
}

// The rotation that keeps world Z up and turns the X axis across the
// camera's view. Inlined into _FillVertexBuffer at 0x6EC5B2. Name not in the
// reference map.
Hmx::Matrix3 MakeUprightFacing(const Hmx::Matrix3& cameraRotation) {
    Hmx::Matrix3 rotation;
    rotation.y = {0.0F, 1.0F, 0.0F};
    rotation.z = Vector3::sZ;
    const Vector3& up = rotation.z;
    const Vector3& view = cameraRotation.y;
    const Vector3 across = {
        up.z * view.y - view.z * up.y,
        view.z * up.x - view.x * up.z,
        view.x * up.y - view.y * up.x,
    };
    const float length = std::sqrt(across.y * across.y + across.z * across.z + across.x * across.x);
    float scale = 0.0F;
    if (length != 0.0F) {
        scale = 1.0F / length;
    }
    rotation.x = {scale * across.x, scale * across.y, scale * across.z};
    const Vector3& x = rotation.x;
    rotation.y = {
        x.z * up.y - x.y * up.z,
        x.x * up.z - x.z * up.x,
        x.y * up.x - x.x * up.y,
    };
    return rotation;
}

// Turns a vector by a rotation, as a row vector times the matrix. Inlined
// into _ComputeParticleBasis. Name not in the reference map.
Vector3 Rotate(const Vector3& v, const Hmx::Matrix3& m) {
    return {
        v.y * m.y.x + v.x * m.x.x + v.z * m.z.x,
        v.y * m.y.y + v.x * m.x.y + v.z * m.z.y,
        v.y * m.y.z + v.x * m.x.z + v.z * m.z.z,
    };
}

}  // namespace

// Reconstructed from eboot.elf at 0x6EAFD0.
RndParticleBuffer* RndParticleBuffer::New(
    unsigned long numParticles,
    const char* name) {
    return TheRndFactory()->CreateParticleBuffer(
        static_cast<unsigned int>(numParticles), name);
}

// Reconstructed from eboot.elf at 0x6EB000.
RndParticleBuffer::RndParticleBuffer(unsigned long numParticles, const char* name)
    : mCapacity(numParticles),
      mNumActive(0),
      mParticles(nullptr),
      mXfm(nullptr),
      mAlignment(-1),
      mSortMode(-1),
      mVelocityAligned(false),
      mWorldSpace(true),
      mHasRotation(false),
      mName(name) {}

// Reconstructed from eboot.elf at 0x6EB6B0. The alignment picks the quad's
// local right and up axes. A velocity-aligned quad turns its up axis toward
// the particle's velocity within that plane; otherwise the quad spins by
// the particle's rotation about the axis it faces. Both results are then
// turned by the system's rotation.
void RndParticleBuffer::_ComputeParticleBasis(
    unsigned long particle,
    Vector3& right,
    Vector3& up,
    const Hmx::Matrix3& rotation,
    RndParticleCom::ParticleAlignment alignment,
    bool velocityAligned,
    const RndParticleCollection& particles) {
    Vector3 rightAxis = {0.0F, 0.0F, 0.0F};
    Vector3 upAxis = {0.0F, 0.0F, 0.0F};
    switch (alignment) {
    case RndParticleCom::kCameraAligned:
    case RndParticleCom::kCameraXYAligned:
    case RndParticleCom::kYAxisAligned:
        rightAxis = {1.0F, 0.0F, 0.0F};
        upAxis = {0.0F, 0.0F, 1.0F};
        break;
    case RndParticleCom::kXAxisAligned:
        rightAxis = {0.0F, 1.0F, 0.0F};
        upAxis = {0.0F, 0.0F, 1.0F};
        break;
    case RndParticleCom::kZAxisAligned:
        rightAxis = {1.0F, 0.0F, 0.0F};
        upAxis = {0.0F, 1.0F, 0.0F};
        break;
    }

    if (velocityAligned) {
        const Vector3 velocity = {
            ParticleFloat(particles, RndParticleCollection::kVelocityX, particle),
            ParticleFloat(particles, RndParticleCollection::kVelocityY, particle),
            ParticleFloat(particles, RndParticleCollection::kVelocityZ, particle),
        };
        const float length = std::sqrt(
            velocity.y * velocity.y + velocity.x * velocity.x + velocity.z * velocity.z);
        float scale = 0.0F;
        if (length != 0.0F) {
            scale = 1.0F / length;
        }
        const Vector3 direction = {
            scale * velocity.x, scale * velocity.y, scale * velocity.z};
        const Vector3 worldRight = Rotate(rightAxis, rotation);
        const Vector3 worldUp = Rotate(upAxis, rotation);
        const float along = worldRight.y * direction.y + worldRight.x * direction.x +
            worldRight.z * direction.z;
        const float across = worldUp.y * direction.y + worldUp.x * direction.x +
            worldUp.z * direction.z;
        up = {
            across * worldUp.x + along * worldRight.x,
            across * worldUp.y + along * worldRight.y,
            across * worldUp.z + along * worldRight.z,
        };
        right = {
            across * worldRight.x - along * worldUp.x,
            across * worldRight.y - along * worldUp.y,
            across * worldRight.z - along * worldUp.z,
        };
        return;
    }

    const float angle = ParticleFloat(particles, RndParticleCollection::kRotation, particle);
    Hmx::Matrix3 spin = Hmx::Matrix3::sID;
    switch (alignment) {
    case RndParticleCom::kCameraAligned:
    case RndParticleCom::kCameraXYAligned:
    case RndParticleCom::kYAxisAligned: {
        // About Y, against the rotation.
        const float cosine = Sine(-angle + kHalfPi);
        const float sine = Sine(-angle);
        spin.x = {cosine, 0.0F, -sine};
        spin.y = {0.0F, 1.0F, 0.0F};
        spin.z = {sine, 0.0F, cosine};
        break;
    }
    case RndParticleCom::kXAxisAligned: {
        const float cosine = Sine(angle + kHalfPi);
        const float sine = Sine(angle);
        spin.x = {1.0F, 0.0F, 0.0F};
        spin.y = {0.0F, cosine, sine};
        spin.z = {0.0F, -sine, cosine};
        break;
    }
    case RndParticleCom::kZAxisAligned: {
        const float cosine = Sine(angle + kHalfPi);
        const float sine = Sine(angle);
        spin.x = {cosine, sine, 0.0F};
        spin.y = {-sine, cosine, 0.0F};
        spin.z = {0.0F, 0.0F, 1.0F};
        break;
    }
    }
    right = Rotate(Rotate(rightAxis, spin), rotation);
    up = Rotate(Rotate(upAxis, spin), rotation);
}

// Reconstructed from eboot.elf at 0x6EBD70. Each particle's position moves
// through the system's transform; its quad spans its size along the right and
// up axes and is shifted by its pivot.
void RndParticleBuffer::_FillVertexBuffer(
    RndContext& context,
    const RndCameraContext& camera,
    void* vertices) {
    const RndParticleCollection& particles = *mParticles;
    const Transform& xfm = *mXfm;
    const bool worldSpace = mWorldSpace;

    Hmx::Matrix3 rotation;
    switch (mAlignment) {
    case RndParticleCom::kCameraAligned:
        rotation = camera.mPrimaryView.mWorldXfm.m;
        break;
    case RndParticleCom::kCameraXYAligned:
        rotation = MakeUprightFacing(camera.mPrimaryView.mWorldXfm.m);
        break;
    default:
        rotation = Hmx::Matrix3::sID;
        break;
    }

    mNumActive = particles.mNumParticles;
    const bool sorted = static_cast<int>(context.mBlendMode) <= kLastSortedBlendMode;
    if (sorted) {
        context._ReserveParticleSorts(mNumActive);
        _SortParticles(context.mParticleSorts, mSortMode, particles, camera);
    }

    auto* vertex = static_cast<RndVertexParticle*>(vertices);
    for (unsigned long i = 0; i < mNumActive; ++i) {
        const unsigned long particle = sorted ? context.mParticleSorts[i].mIndex : i;
        Vector3 right = {};
        Vector3 up = {};
        _ComputeParticleBasis(
            particle,
            right,
            up,
            rotation,
            static_cast<RndParticleCom::ParticleAlignment>(mAlignment),
            mVelocityAligned,
            particles);

        const Hmx::Color& color =
            particles[RndParticleCollection::kColor].Get<Hmx::Color>(particle);
        const Vector3 position = {
            ParticleFloat(particles, RndParticleCollection::kPositionX, particle),
            ParticleFloat(particles, RndParticleCollection::kPositionY, particle),
            ParticleFloat(particles, RndParticleCollection::kPositionZ, particle),
        };
        const Hmx::Matrix3& m = xfm.m;
        Vector3 center = {
            position.y * m.y.x + position.x * m.x.x + position.z * m.z.x + xfm.v.x,
            position.y * m.y.y + position.x * m.x.y + position.z * m.z.y + xfm.v.y,
            position.y * m.y.z + position.x * m.x.z + position.z * m.z.z + xfm.v.z,
        };
        const float sizeX = ParticleFloat(particles, RndParticleCollection::kSizeX, particle);
        const float sizeY = ParticleFloat(particles, RndParticleCollection::kSizeY, particle);
        const float pivotX =
            ParticleFloat(particles, RndParticleCollection::kPivotX, particle) * 2.0F - 1.0F;
        const float pivotY =
            ParticleFloat(particles, RndParticleCollection::kPivotY, particle) * 2.0F - 1.0F;
        float data[3] = {0.0F, 0.0F, 0.0F};
        if (mHasRotation) {
            data[0] = ParticleFloat(particles, RndParticleCollection::kData0, particle);
            data[1] = ParticleFloat(particles, RndParticleCollection::kData1, particle);
            data[2] = ParticleFloat(particles, RndParticleCollection::kData2, particle);
        }

        if (worldSpace) {
            center.x = center.x + right.x * pivotX + up.x * pivotY;
            center.y = center.y + right.y * pivotX + up.y * pivotY;
            center.z = center.z + right.z * pivotX + up.z * pivotY;
        }
        const Vector3 scaledRight = {right.x * sizeX, right.y * sizeX, right.z * sizeX};
        const Vector3 scaledUp = {up.x * sizeY, up.y * sizeY, up.z * sizeY};
        const float rightPivot = sizeX * pivotX;
        const float upPivot = sizeY * pivotY;

        for (const QuadCorner& corner : kQuadCorners) {
            float pos[3] = {
                center.x + corner.mRight * scaledRight.x + corner.mUp * scaledUp.x,
                center.y + corner.mRight * scaledRight.y + corner.mUp * scaledUp.y,
                center.z + corner.mRight * scaledRight.z + corner.mUp * scaledUp.z,
            };
            if (!worldSpace) {
                pos[0] = pos[0] + rightPivot * right.x + upPivot * up.x;
                pos[1] = pos[1] + rightPivot * right.y + upPivot * up.y;
                pos[2] = pos[2] + rightPivot * right.z + upPivot * up.z;
            }
            vertex->mPos[0] = pos[0];
            vertex->mPos[1] = pos[1];
            vertex->mPos[2] = pos[2];
            vertex->mColor[0] = color.red;
            vertex->mColor[1] = color.green;
            vertex->mColor[2] = color.blue;
            vertex->mColor[3] = color.alpha;
            vertex->mTex[0] = corner.mU;
            vertex->mTex[1] = corner.mV;
            vertex->mParticleData[0] = data[0];
            vertex->mParticleData[1] = data[1];
            vertex->mParticleData[2] = data[2];
            vertex->mParticleData[3] = 0.0F;
            ++vertex;
        }
    }
}

// Reconstructed from eboot.elf at 0x6EC700. The keys sort ascending: the
// negated view depth draws back to front, the negated age oldest first and
// the age newest first. Positions are taken in the system's space. The
// binary sorts with an EASTL sort instantiation (0x6ECBA0: introsort with a
// 28-element insertion-sort threshold).
void RndParticleBuffer::_SortParticles(
    eastl::vector<ParticleIndexDepth>& sorts,
    int sortMode,
    const RndParticleCollection& particles,
    const RndCameraContext& camera) {
    sorts.clear();
    switch (sortMode) {
    case 0: {
        const Vector3& view = camera.mPrimaryView.mWorldXfm.m.y;
        for (unsigned long i = 0; i < mNumActive; ++i) {
            const float depth =
                view.x * ParticleFloat(particles, RndParticleCollection::kPositionX, i) +
                view.y * ParticleFloat(particles, RndParticleCollection::kPositionY, i) +
                view.z * ParticleFloat(particles, RndParticleCollection::kPositionZ, i);
            sorts.push_back({-depth, i});
        }
        break;
    }
    case 1:
        for (unsigned long i = 0; i < mNumActive; ++i) {
            sorts.push_back(
                {-ParticleFloat(particles, RndParticleCollection::kAge, i), i});
        }
        break;
    case 2:
        for (unsigned long i = 0; i < mNumActive; ++i) {
            sorts.push_back({ParticleFloat(particles, RndParticleCollection::kAge, i), i});
        }
        break;
    }
    std::sort(sorts.begin(), sorts.end(), ParticleDepthSort);
}

// Reconstructed from eboot.elf at 0x6ECB60.
void RndParticleBuffer::_UpdateStats(RndContext&) {}
