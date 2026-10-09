#pragma once

#include <cstddef>

#include "math/geometry/Sphere.h"
#include "math/geometry/TruncatedRoundedCone.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"
#include "render/meshes/RndVertex.h"
#include "utl/containers/Vector.h"

class RndMesh;
class Transform;

// Procedural mesh builders. Each Create function makes a mesh through the
// factory, fills its vertices and faces, records its bounding sphere and,
// unless the parameters say otherwise, uploads it.
namespace RndMeshUtl {

// CreateMeshParams::mFlags bits. Names not in the reference map.
enum CreateMeshFlags : unsigned int {
    // Leave SyncStatic to the caller.
    kCreateMeshNoSync = 0x1,
    // Add a second copy of the vertices with flipped normals, and a second
    // set of faces.
    kCreateMeshDoubleSided = 0x2,
    // Alternate the diagonal that splits each quad.
    kCreateMeshAlternateDiagonals = 0x4,
    // CreateSphere: grow the tessellation so it circumscribes the sphere.
    kCreateMeshCircumscribe = 0x8,
    // Set the mesh's keep-faces flag.
    kCreateMeshKeepFaces = 0x10,
};

// The fields every builder reads. The constructor keeps the class out of the
// layout-POD category, so the derived parameter blocks start their fields in
// its tail padding at 0x24. Field names are not in the reference map.
struct CreateMeshParams {
    CreateMeshParams()
        : mName(nullptr),
          mVertexType(kVertexUnskinned),
          mFlags(0),
          mVertexUsageFlags(0),
          mFaceUsageFlags(0),
          mOffset{0.0F, 0.0F, 0.0F} {}

    // Null takes the builder's default name.
    const char* mName;
    RndVertexType mVertexType;
    unsigned int mFlags;
    unsigned int mVertexUsageFlags;
    unsigned int mFaceUsageFlags;
    // Added to every position.
    Vector3 mOffset;
};

static_assert(offsetof(CreateMeshParams, mVertexType) == 8);
static_assert(offsetof(CreateMeshParams, mFlags) == 12);
static_assert(offsetof(CreateMeshParams, mVertexUsageFlags) == 16);
static_assert(offsetof(CreateMeshParams, mFaceUsageFlags) == 20);
static_assert(offsetof(CreateMeshParams, mOffset) == 24);
static_assert(sizeof(CreateMeshParams) == 40);

// A grid centered on mOffset spanning mAxisU by mAxisV. Field names are not in
// the reference map.
struct CreateQuadParams : public CreateMeshParams {
    Vector3 mAxisU;
    Vector3 mAxisV;
    int mNumSegmentsU;
    int mNumSegmentsV;
};

static_assert(offsetof(CreateQuadParams, mAxisU) == 36);
static_assert(offsetof(CreateQuadParams, mAxisV) == 48);
static_assert(offsetof(CreateQuadParams, mNumSegmentsU) == 60);
static_assert(offsetof(CreateQuadParams, mNumSegmentsV) == 64);
static_assert(sizeof(CreateQuadParams) == 72);

// A box centered on mOffset whose edges are the three axes. Field names are
// not in the reference map.
struct CreateBoxParams : public CreateMeshParams {
    Vector3 mAxisX;
    Vector3 mAxisY;
    Vector3 mAxisZ;
    int mNumSegmentsX;
    int mNumSegmentsY;
    int mNumSegmentsZ;
};

static_assert(offsetof(CreateBoxParams, mAxisX) == 36);
static_assert(offsetof(CreateBoxParams, mAxisZ) == 60);
static_assert(offsetof(CreateBoxParams, mNumSegmentsX) == 72);
static_assert(offsetof(CreateBoxParams, mNumSegmentsZ) == 80);
static_assert(sizeof(CreateBoxParams) == 88);

// One point of the profile a radial surface sweeps around the z axis: x is
// the distance from the axis, z the height. Field names are not in the
// reference map.
struct ContourVertex {
    Vector3 mPos;
    Vector3 mNormal;
};

static_assert(sizeof(ContourVertex) == 24);

// Field names are not in the reference map.
struct CreateRadialSurfaceParams : public CreateMeshParams {
    CreateRadialSurfaceParams()
        : mSweepAngle(6.2831855F),
          mNumSegments(32),
          mUnknown30{},
          mUnknown40(0.0F),
          mUnknown44(1.0F) {}

    // An angle within 1e-4 of 2*pi wraps to 0, closing the seam.
    float mSweepAngle;
    unsigned long mNumSegments;
    // Not read by the builders reconstructed so far.
    unsigned int mUnknown30[4];
    float mUnknown40;
    float mUnknown44;
};

static_assert(offsetof(CreateRadialSurfaceParams, mSweepAngle) == 36);
static_assert(offsetof(CreateRadialSurfaceParams, mNumSegments) == 40);
static_assert(offsetof(CreateRadialSurfaceParams, mUnknown30) == 48);
static_assert(offsetof(CreateRadialSurfaceParams, mUnknown40) == 64);
static_assert(offsetof(CreateRadialSurfaceParams, mUnknown44) == 68);
static_assert(sizeof(CreateRadialSurfaceParams) == 72);

// Field names are not in the reference map.
struct CreateSphereParams : public CreateRadialSurfaceParams {
    float mRadius;
    // Rings from pole to pole.
    unsigned long mNumRings;
};

static_assert(offsetof(CreateSphereParams, mRadius) == 72);
static_assert(offsetof(CreateSphereParams, mNumRings) == 80);
static_assert(sizeof(CreateSphereParams) == 88);

// A capped cylinder along z, centered on the origin. Field names are not in
// the reference map.
struct CreateCylinderParams : public CreateRadialSurfaceParams {
    float mRadius;
    float mHeight;
    unsigned long mNumHeightSegments;
};

static_assert(offsetof(CreateCylinderParams, mRadius) == 72);
static_assert(offsetof(CreateCylinderParams, mHeight) == 76);
static_assert(offsetof(CreateCylinderParams, mNumHeightSegments) == 80);
static_assert(sizeof(CreateCylinderParams) == 88);

// Field names are not in the reference map.
struct CreateTruncatedRoundedConeParams : public CreateRadialSurfaceParams {
    CreateTruncatedRoundedConeParams()
        : mNumTopSegments(1),
          mNumSideSegments(1),
          mNumCapSegments(16) {}

    TruncatedRoundedCone mCone;
    // Rings across the flat top, along the side, and over the cap; the cap
    // uses half of mNumCapSegments.
    unsigned long mNumTopSegments;
    unsigned long mNumSideSegments;
    unsigned long mNumCapSegments;
};

static_assert(offsetof(CreateTruncatedRoundedConeParams, mCone) == 72);
static_assert(offsetof(CreateTruncatedRoundedConeParams, mNumTopSegments) == 104);
static_assert(offsetof(CreateTruncatedRoundedConeParams, mNumSideSegments) == 112);
static_assert(offsetof(CreateTruncatedRoundedConeParams, mNumCapSegments) == 120);
static_assert(sizeof(CreateTruncatedRoundedConeParams) == 128);

// A quad facing along one of the six axis directions, built by CreateQuad.
// Field names are not in the reference map.
struct CreateFacingQuadParams : public CreateMeshParams {
    // Selects the quad's axes, 0 to 5: (y, z), (-x, z), (x, y), (-y, z),
    // (x, z) and (x, -y), scaled by mWidth and mHeight. Other values build a
    // unit quad on (x, y).
    unsigned int mFacing;
    float mWidth;
    float mHeight;
    int mNumSegmentsU;
    int mNumSegmentsV;
};

static_assert(offsetof(CreateFacingQuadParams, mFacing) == 36);
static_assert(offsetof(CreateFacingQuadParams, mWidth) == 40);
static_assert(offsetof(CreateFacingQuadParams, mNumSegmentsU) == 48);
static_assert(offsetof(CreateFacingQuadParams, mNumSegmentsV) == 52);
static_assert(sizeof(CreateFacingQuadParams) == 56);

// A disc built as a fan of triangles around its center, facing along one of
// the six axis directions. Name not in the reference map, which has a
// CreateTriangleParams instead; field names are not in the reference map.
struct CreateTriangleFanParams : public CreateMeshParams {
    // Selects the disc's axes as in CreateFacingQuadParams, except that 5
    // uses (-x, y). Other values collapse the disc onto mOffset.
    unsigned int mFacing;
    unsigned long mNumSegments;
    float mRadius;
};

static_assert(offsetof(CreateTriangleFanParams, mFacing) == 36);
static_assert(offsetof(CreateTriangleFanParams, mNumSegments) == 40);
static_assert(offsetof(CreateTriangleFanParams, mRadius) == 48);
static_assert(sizeof(CreateTriangleFanParams) == 56);

// A capsule along z swept from a generated contour. Field names are not in
// the reference map.
struct CreateCapsuleParams : public CreateRadialSurfaceParams {
    float mRadius;
    float mLength;
    // The contour has mNumSideSegments + 2 * mNumCapSegments + 1 points.
    unsigned long mNumCapSegments;
    unsigned long mNumSideSegments;
};

static_assert(offsetof(CreateCapsuleParams, mRadius) == 72);
static_assert(offsetof(CreateCapsuleParams, mLength) == 76);
static_assert(offsetof(CreateCapsuleParams, mNumCapSegments) == 80);
static_assert(offsetof(CreateCapsuleParams, mNumSideSegments) == 88);
static_assert(sizeof(CreateCapsuleParams) == 96);

// Two cones along +z that share their apex at the origin, swept from a
// generated contour; its default mesh name is "NestedCone". The flatter
// cone (the smaller height over radius) is the outer surface and the
// steeper one the inner surface. Name not in the reference map; field
// names are not in the reference map.
struct CreateNestedConeParams : public CreateRadialSurfaceParams {
    // The base radius and height of each cone. Negative radii are clamped
    // to zero.
    Vector2 mRadii;
    Vector2 mHeights;
    // Rings along each cone's side.
    unsigned long mNumConeSegments;
    // Adds a ring of faces joining the two cones' base rims.
    bool mJoinRims;
};

static_assert(offsetof(CreateNestedConeParams, mRadii) == 72);
static_assert(offsetof(CreateNestedConeParams, mHeights) == 80);
static_assert(offsetof(CreateNestedConeParams, mNumConeSegments) == 88);
static_assert(offsetof(CreateNestedConeParams, mJoinRims) == 96);
static_assert(sizeof(CreateNestedConeParams) == 104);

// Scratch storage shared by the builders.
extern eastl::vector<ContourVertex> gTmpContour;  // 0x1AA6A58
extern eastl::vector<float> gTmpUVIntervals;      // 0x1AA6A78

// Bounds the transformed vertex positions with a BoundingHull and returns
// its sphere. The map has ComputeBoundingSphere(RndMesh&); this build takes
// a transform for the positions and returns the sphere.
Sphere ComputeBoundingSphere(const RndMesh& mesh, const Transform& xfm);  // 0x5D86E0

// The binary keeps out-of-line copies of these two; the builders inline them.
RndMesh* _CreateMeshPrelude(const CreateMeshParams& params, const char* defaultName);  // 0x5DB660
void _CreateMeshCoda(RndMesh& mesh, const CreateMeshParams& params);  // 0x5DB6B0

// Writes one quad's vertices and faces starting at the given indices, then
// advances them.
void _SetupQuadVertsAndFaces(
    RndMesh* mesh,
    const CreateQuadParams& params,
    unsigned long& vertex,
    unsigned long& face);  // 0x5DB930

RndMesh* CreateQuad(const CreateQuadParams& params);  // 0x5DB700
// Converts the facing to the quad's axes and calls CreateQuad.
RndMesh* CreateFacingQuad(const CreateFacingQuadParams& params);  // 0x5DC240
// Name not in the reference map, which has CreateTriangle instead.
RndMesh* CreateTriangleFan(const CreateTriangleFanParams& params);  // 0x5DC380
RndMesh* CreateBox(const CreateBoxParams& params);        // 0x5DCBA0
RndMesh* CreateSphere(const CreateSphereParams& params);  // 0x5DD0E0
RndMesh* CreateRadialSurface(
    const eastl::vector<ContourVertex>& contour,
    const CreateRadialSurfaceParams& params);  // 0x5DD5E0
RndMesh* CreateCylinder(const CreateCylinderParams& params);  // 0x5DDC00
// Generates the contour into gTmpContour and sweeps it with
// CreateRadialSurface.
RndMesh* CreateCapsule(const CreateCapsuleParams& params);  // 0x5DE890
void _GenerateCapsuleContour(
    const CreateCapsuleParams& params,
    eastl::vector<ContourVertex>& contour);  // 0x5DE910
// Generates the contour into gTmpContour and sweeps it with
// CreateRadialSurface. Name not in the reference map.
RndMesh* CreateNestedCone(const CreateNestedConeParams& params);  // 0x5E0240
// Name not in the reference map.
void _GenerateNestedConeContour(
    const CreateNestedConeParams& params,
    eastl::vector<ContourVertex>& contour);  // 0x5E02C0
void ReshapeRadialSurface(
    RndMesh& mesh,
    const eastl::vector<ContourVertex>& contour,
    const CreateRadialSurfaceParams& params);  // 0x5DF180
RndMesh* CreateTruncatedRoundedCone(
    const CreateTruncatedRoundedConeParams& params);  // 0x5E0C10
void _GenerateTruncatedRoundedConeContour(
    const CreateTruncatedRoundedConeParams& params,
    eastl::vector<ContourVertex>& contour);  // 0x5E0C90

}  // namespace RndMeshUtl
