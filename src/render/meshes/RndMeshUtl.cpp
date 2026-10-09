#include "render/meshes/RndMeshUtl.h"

#include <cmath>
#include <cstdint>

#include "math/color/Color.h"
#include "math/geometry/Geo.h"
#include "math/scalar/Trig.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector4.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndVertexInterpreter.h"

namespace RndMeshUtl {

eastl::vector<ContourVertex> gTmpContour;
eastl::vector<float> gTmpUVIntervals;

}  // namespace RndMeshUtl

namespace {

using AttributeInfo = RndVertexInterpreter::AttributeInfo;

constexpr float kPi = 3.1415927F;
constexpr float kHalfPi = 1.5707964F;
constexpr float kTwoPi = 6.2831855F;
constexpr float kEpsilon = 1.0e-4F;

Vector3 Negate(const Vector3& v) {
    return {-v.x, -v.y, -v.z};
}

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// A zero vector stays zero.
Vector3 Normalize(const Vector3& v) {
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    const float scale = length != 0.0F ? 1.0F / length : 0.0F;
    return {scale * v.x, scale * v.y, scale * v.z};
}

bool SamePoint(const Vector3& a, const Vector3& b) {
    return std::fabs(a.x - b.x) <= kEpsilon && std::fabs(a.y - b.y) <= kEpsilon &&
           std::fabs(a.z - b.z) <= kEpsilon;
}

bool OnAxis(const Vector3& p) {
    return std::fabs(p.x) <= kEpsilon && std::fabs(p.y) <= kEpsilon;
}

unsigned int DoubleSidedShift(const RndMeshUtl::CreateMeshParams& params) {
    return (params.mFlags >> 1) & 1;
}

const AttributeInfo& Attribute(const RndVertexInterpreter* interp, unsigned int index) {
    return interp->mAttributes[index];
}

bool HasAttribute(const RndVertexInterpreter* interp, unsigned int index) {
    return interp->mAttributes[index].mOffset != -1;
}

void* AttributeData(void* vertex, const AttributeInfo& info) {
    return static_cast<char*>(vertex) + info.mOffset;
}

template <class T>
void SetAttribute(
    const RndVertexInterpreter* interp,
    unsigned int index,
    void* vertex,
    const T& value) {
    const auto& info = Attribute(interp, index);
    RndVertexInterpreter::_AttributeValueHelper<T>::Set(
        info, value, AttributeData(vertex, info));
}

// The attributes past the position and frame that every builder writes the
// same way: opaque white, the UV in every UV set, and a single full weight on
// bone 0. The color is written through the Vector4 writer.
void SetDefaultAttributes(
    const RndVertexInterpreter* interp,
    void* vertex,
    const Vector2& uv) {
    using Interp = RndVertexInterpreter;
    if (HasAttribute(interp, Interp::kColorAttribute)) {
        SetAttribute(
            interp,
            Interp::kColorAttribute,
            vertex,
            reinterpret_cast<const Vector4&>(Hmx::Color::GetWhite()));
    }
    for (unsigned long set = 0; set < interp->GetNumUVs(); ++set) {
        SetAttribute(
            interp, Interp::kFirstUVAttribute + static_cast<unsigned int>(set), vertex, uv);
    }
    if (HasAttribute(interp, Interp::kWeightsAttribute) &&
        HasAttribute(interp, Interp::kBonesAttribute)) {
        const Vector4 weights = {1.0F, 0.0F, 0.0F, 0.0F};
        SetAttribute(interp, Interp::kWeightsAttribute, vertex, weights);
        *static_cast<std::uint32_t*>(
            AttributeData(vertex, Attribute(interp, Interp::kBonesAttribute))) = 0;
    }
}

void SetFace(RndMesh::Face& face, unsigned int a, unsigned int b, unsigned int c) {
    face.mIdx[0] = a;
    face.mIdx[1] = b;
    face.mIdx[2] = c;
}

}  // namespace

// Reconstructed from eboot.elf at 0x5D86E0. The binary calls the mesh's
// vertex accessors through the vtable on the const mesh.
Sphere RndMeshUtl::ComputeBoundingSphere(const RndMesh& mesh, const Transform& xfm) {
    auto& vertices = const_cast<RndMesh&>(mesh);
    const RndVertexInterpreter* interp =
        RndVertexInterpreter::GetInstance(vertices._GetVertexTypeImpl());
    BoundingHull hull;
    const unsigned long numVertices = vertices._GetNumVerticesImpl();
    for (unsigned long i = 0; i < numVertices; ++i) {
        Vector3 position = interp->GetAttribute<Vector3>(
            RndVertexInterpreter::kPositionAttribute, vertices._GetVertexVoidImpl(i));
        Multiply(position, xfm, position);
        hull.GrowToContain(&position, 1);
    }
    Sphere sphere = {};
    hull.GenerateSphere(sphere);
    return sphere;
}

// Reconstructed from eboot.elf at 0x5DB660.
RndMesh* RndMeshUtl::_CreateMeshPrelude(
    const CreateMeshParams& params,
    const char* defaultName) {
    RndMesh* mesh = RndMesh::New(
        params.mVertexType, params.mName != nullptr ? params.mName : defaultName);
    mesh->SetVertexUsageFlags(params.mVertexUsageFlags);
    mesh->SetFaceUsageFlags(params.mFaceUsageFlags);
    if ((params.mFlags & kCreateMeshKeepFaces) != 0) {
        mesh->mKeepFaces = true;
    }
    return mesh;
}

// Reconstructed from eboot.elf at 0x5DB6B0.
void RndMeshUtl::_CreateMeshCoda(RndMesh& mesh, const CreateMeshParams& params) {
    mesh.mBoundingSphere = ComputeBoundingSphere(mesh, Transform::sID);
    if ((params.mFlags & kCreateMeshNoSync) == 0) {
        mesh.SyncStatic();
    }
}

// Reconstructed from eboot.elf at 0x5DB930. The vertices run row by row
// along mAxisU, from -0.5 to +0.5 of each axis, with the V texture coordinate
// flipped. The second side's faces reuse the first side's vertices and
// winding, as in the binary.
void RndMeshUtl::_SetupQuadVertsAndFaces(
    RndMesh* mesh,
    const CreateQuadParams& params,
    unsigned long& vertex,
    unsigned long& face) {
    using Interp = RndVertexInterpreter;
    Vector3 normal = Normalize(Cross(params.mAxisU, params.mAxisV));
    const Vector3 tangent = Normalize(params.mAxisU);
    const Vector3 bitangent = Negate(Normalize(params.mAxisV));

    const RndVertexInterpreter* interp =
        RndVertexInterpreter::GetInstance(params.mVertexType);
    const unsigned long numU = static_cast<unsigned long>(params.mNumSegmentsU);
    const unsigned long numV = static_cast<unsigned long>(params.mNumSegmentsV);
    unsigned long nextVertex = vertex;
    for (unsigned long side = 0; side < 2; ++side) {
        if (side != 0) {
            if ((params.mFlags & kCreateMeshDoubleSided) == 0) {
                continue;
            }
            normal = Negate(normal);
        }
        for (unsigned long row = 0; row <= numV; ++row) {
            const float v = row / static_cast<float>(params.mNumSegmentsV);
            const float rowScale = v - 0.5F;
            const Vector3 rowOrigin = {
                rowScale * params.mAxisV.x + params.mOffset.x,
                rowScale * params.mAxisV.y + params.mOffset.y,
                rowScale * params.mAxisV.z + params.mOffset.z,
            };
            const float texV = 1.0F - v;
            for (unsigned long column = 0; column <= numU; ++column, ++nextVertex) {
                void* data = mesh->_GetVertexVoidImpl(nextVertex);
                const float u = column / static_cast<float>(params.mNumSegmentsU);
                const float columnScale = u - 0.5F;
                const Vector3 pos = {
                    columnScale * params.mAxisU.x + rowOrigin.x,
                    columnScale * params.mAxisU.y + rowOrigin.y,
                    columnScale * params.mAxisU.z + rowOrigin.z,
                };
                SetAttribute(interp, Interp::kPositionAttribute, data, pos);
                if (HasAttribute(interp, Interp::kNormalAttribute)) {
                    SetAttribute(interp, Interp::kNormalAttribute, data, normal);
                }
                if (HasAttribute(interp, Interp::kTangentAttribute)) {
                    SetAttribute(interp, Interp::kTangentAttribute, data, tangent);
                    SetAttribute(interp, Interp::kBitangentAttribute, data, bitangent);
                }
                SetDefaultAttributes(interp, data, {u, texV});
            }
        }
    }

    const bool alternate = (params.mFlags & kCreateMeshAlternateDiagonals) != 0;
    const unsigned int numColumns = static_cast<unsigned int>(params.mNumSegmentsU);
    const unsigned int stride = numColumns + 1;
    const unsigned long numRows = static_cast<unsigned long>(
        static_cast<long>(params.mNumSegmentsV));
    unsigned long nextFace = face;
    for (unsigned long side = 0; side < 2; ++side) {
        if (side != 0 && (params.mFlags & kCreateMeshDoubleSided) == 0) {
            break;
        }
        if (numRows == 0 || numColumns == 0) {
            break;
        }
        unsigned int rowStart = static_cast<unsigned int>(vertex);
        for (unsigned long row = 0; row < numRows; ++row, rowStart += stride) {
            for (unsigned long column = 0; column < numColumns; ++column) {
                const unsigned int v = rowStart + static_cast<unsigned int>(column);
                RndMesh::Face* faces = &mesh->mFaces[nextFace];
                nextFace += 2;
                const bool flip = alternate && ((column ^ row) & 1) != 0;
                SetFace(faces[0], v, v + 1, flip ? v + stride : v + stride + 1);
                SetFace(faces[1], flip ? v + 1 : v, v + stride + 1, v + stride);
            }
        }
    }
    vertex = nextVertex;
    face = nextFace;
}

// Reconstructed from eboot.elf at 0x5DB700.
RndMesh* RndMeshUtl::CreateQuad(const CreateQuadParams& params) {
    RndMesh* mesh = _CreateMeshPrelude(params, "Quad");
    mesh->_SetNumVerticesImpl(
        static_cast<unsigned long>(
            (params.mNumSegmentsU + 1L) * (params.mNumSegmentsV + 1L))
        << DoubleSidedShift(params));
    const int numFaces = 2 * params.mNumSegmentsV * params.mNumSegmentsU;
    mesh->mFaces.resize(static_cast<unsigned long>(static_cast<long>(numFaces))
                        << DoubleSidedShift(params));

    unsigned long vertex = 0;
    unsigned long face = 0;
    _SetupQuadVertsAndFaces(mesh, params, vertex, face);
    // The binary reads the vertex count and discards it.
    mesh->_GetNumVerticesImpl();
    _CreateMeshCoda(*mesh, params);
    return mesh;
}

// Reconstructed from eboot.elf at 0x5DC240. When the facing is out of range
// the quad keeps the unit axes, which the binary stores before the switch.
RndMesh* RndMeshUtl::CreateFacingQuad(const CreateFacingQuadParams& params) {
    CreateQuadParams quad;
    static_cast<CreateMeshParams&>(quad) = params;
    quad.mAxisU = {1.0F, 0.0F, 0.0F};
    quad.mAxisV = {0.0F, 1.0F, 0.0F};
    quad.mNumSegmentsU = params.mNumSegmentsU;
    quad.mNumSegmentsV = params.mNumSegmentsV;
    const float width = params.mWidth;
    const float height = params.mHeight;
    switch (params.mFacing) {
    case 0:
        quad.mAxisU = {0.0F, width, 0.0F};
        quad.mAxisV = {0.0F, 0.0F, height};
        break;
    case 1:
        quad.mAxisU = {-width, 0.0F, 0.0F};
        quad.mAxisV = {0.0F, 0.0F, height};
        break;
    case 2:
        quad.mAxisU = {width, 0.0F, 0.0F};
        quad.mAxisV = {0.0F, height, 0.0F};
        break;
    case 3:
        quad.mAxisU = {0.0F, -width, 0.0F};
        quad.mAxisV = {0.0F, 0.0F, height};
        break;
    case 4:
        quad.mAxisU = {width, 0.0F, 0.0F};
        quad.mAxisV = {0.0F, 0.0F, height};
        break;
    case 5:
        quad.mAxisU = {width, 0.0F, 0.0F};
        quad.mAxisV = {0.0F, -height, 0.0F};
        break;
    default:
        break;
    }
    return CreateQuad(quad);
}

// Reconstructed from eboot.elf at 0x5DC380. Vertex 0 is the center; the rim
// vertices run from axisU toward axisV. The normal is axisU x axisV, the
// tangent axisU and the bitangent -axisV. The UVs project each position,
// offset included, onto the two axes, scaled by the radius into 0 to 1.
// There is no second side.
RndMesh* RndMeshUtl::CreateTriangleFan(const CreateTriangleFanParams& params) {
    using Interp = RndVertexInterpreter;
    Vector3 axisU;
    Vector3 axisV;
    switch (params.mFacing) {
    case 0:
        axisU = Vector3::sY;
        axisV = Vector3::sZ;
        break;
    case 1:
        axisU = Negate(Vector3::sX);
        axisV = Vector3::sZ;
        break;
    case 2:
        axisU = Vector3::sX;
        axisV = Vector3::sY;
        break;
    case 3:
        axisU = Negate(Vector3::sY);
        axisV = Vector3::sZ;
        break;
    case 4:
        axisU = Vector3::sX;
        axisV = Vector3::sZ;
        break;
    case 5:
        axisU = Negate(Vector3::sX);
        axisV = Vector3::sY;
        break;
    default:
        axisU = {0.0F, 0.0F, 0.0F};
        axisV = {0.0F, 0.0F, 0.0F};
        break;
    }

    RndMesh* mesh = _CreateMeshPrelude(params, "TriangleFan");
    const RndVertexInterpreter* interp =
        RndVertexInterpreter::GetInstance(params.mVertexType);
    const unsigned long numVertices = params.mNumSegments + 1;
    mesh->_SetNumVerticesImpl(numVertices);
    const unsigned long numFaces = params.mNumSegments;
    mesh->mFaces.resize(numFaces);

    const Vector3 tangent = axisU;
    const Vector3 normal = Cross(axisU, axisV);
    const Vector3 bitangent = Negate(axisV);

    SetAttribute(
        interp, Interp::kPositionAttribute, mesh->_GetVertexVoidImpl(0), params.mOffset);
    for (unsigned long segment = 0; segment < params.mNumSegments;) {
        const float angle = segment / static_cast<float>(params.mNumSegments) * kTwoPi;
        const float sine = Sine(angle);
        const float cosine = Sine(angle + kHalfPi);
        ++segment;
        const Vector3 pos = {
            (cosine * axisU.x + sine * axisV.x) * params.mRadius + params.mOffset.x,
            (cosine * axisU.y + sine * axisV.y) * params.mRadius + params.mOffset.y,
            (cosine * axisU.z + sine * axisV.z) * params.mRadius + params.mOffset.z,
        };
        SetAttribute(
            interp, Interp::kPositionAttribute, mesh->_GetVertexVoidImpl(segment), pos);
    }

    for (unsigned long vertex = 0; vertex < numVertices; ++vertex) {
        void* data = mesh->_GetVertexVoidImpl(vertex);
        if (HasAttribute(interp, Interp::kNormalAttribute)) {
            SetAttribute(interp, Interp::kNormalAttribute, data, normal);
        }
        if (HasAttribute(interp, Interp::kTangentAttribute)) {
            SetAttribute(interp, Interp::kTangentAttribute, data, tangent);
            SetAttribute(interp, Interp::kBitangentAttribute, data, bitangent);
        }
        if (HasAttribute(interp, Interp::kColorAttribute)) {
            SetAttribute(
                interp,
                Interp::kColorAttribute,
                data,
                reinterpret_cast<const Vector4&>(Hmx::Color::GetWhite()));
        }
        if (HasAttribute(interp, Interp::kWeightsAttribute) &&
            HasAttribute(interp, Interp::kBonesAttribute)) {
            const Vector4 weights = {1.0F, 0.0F, 0.0F, 0.0F};
            SetAttribute(interp, Interp::kWeightsAttribute, data, weights);
            *static_cast<std::uint32_t*>(
                AttributeData(data, Attribute(interp, Interp::kBonesAttribute))) = 0;
        }
        const Vector3 pos =
            interp->GetAttribute<Vector3>(Interp::kPositionAttribute, data);
        const float scale = 1.0F / params.mRadius;
        const float x = pos.x * scale;
        const float y = pos.y * scale;
        const float z = scale * pos.z;
        const Vector2 uv = {
            (x * axisU.x + 1.0F + (y * axisU.y + z * axisU.z)) * 0.5F,
            1.0F - (x * axisV.x + 1.0F + (y * axisV.y + z * axisV.z)) * 0.5F,
        };
        for (unsigned long set = 0; set < interp->GetNumUVs(); ++set) {
            SetAttribute(
                interp, Interp::kFirstUVAttribute + static_cast<unsigned int>(set), data, uv);
        }
    }

    for (unsigned long face = 0; face < numFaces; ++face) {
        SetFace(
            mesh->mFaces[face],
            0,
            static_cast<unsigned int>(face + 1),
            static_cast<unsigned int>((face + 1) % params.mNumSegments + 1));
    }
    _CreateMeshCoda(*mesh, params);
    return mesh;
}

// Reconstructed from eboot.elf at 0x5DCBA0. The faces are built as six
// quads: +x, -x, +y, -y, +z, -z.
RndMesh* RndMeshUtl::CreateBox(const CreateBoxParams& params) {
    RndMesh* mesh = _CreateMeshPrelude(params, "Box");
    const int numX = params.mNumSegmentsX;
    const int numY = params.mNumSegmentsY;
    const int numZ = params.mNumSegmentsZ;
    const int numVerts =
        2 * ((numZ + 1) * (numY + 1) + (numX + 1) * (numZ + numY + 2));
    mesh->_SetNumVerticesImpl(static_cast<unsigned long>(static_cast<long>(numVerts))
                              << DoubleSidedShift(params));
    const int numFaces = 4 * (numZ * numY + (numZ + numY) * numX);
    mesh->mFaces.resize(static_cast<unsigned long>(static_cast<long>(numFaces))
                        << DoubleSidedShift(params));

    unsigned long vertex = 0;
    unsigned long face = 0;
    const Vector3& center = params.mOffset;
    const auto offset = [&center](const Vector3& axis, float scale) {
        return Vector3{
            center.x + scale * axis.x,
            center.y + scale * axis.y,
            center.z + scale * axis.z,
        };
    };

    CreateQuadParams quad;
    static_cast<CreateMeshParams&>(quad) = params;
    quad.mOffset = offset(params.mAxisX, 0.5F);
    quad.mAxisU = params.mAxisY;
    quad.mAxisV = params.mAxisZ;
    quad.mNumSegmentsU = numY;
    quad.mNumSegmentsV = numZ;
    _SetupQuadVertsAndFaces(mesh, quad, vertex, face);

    quad.mOffset = offset(params.mAxisX, -0.5F);
    quad.mAxisU = Negate(quad.mAxisU);
    _SetupQuadVertsAndFaces(mesh, quad, vertex, face);

    quad.mOffset = offset(params.mAxisY, 0.5F);
    quad.mAxisU = Negate(params.mAxisX);
    quad.mAxisV = params.mAxisZ;
    quad.mNumSegmentsU = numX;
    quad.mNumSegmentsV = numZ;
    _SetupQuadVertsAndFaces(mesh, quad, vertex, face);

    quad.mOffset = offset(params.mAxisY, -0.5F);
    quad.mAxisU = params.mAxisX;
    _SetupQuadVertsAndFaces(mesh, quad, vertex, face);

    quad.mOffset = offset(params.mAxisZ, 0.5F);
    quad.mAxisU = params.mAxisX;
    quad.mAxisV = params.mAxisY;
    quad.mNumSegmentsU = numX;
    quad.mNumSegmentsV = numY;
    _SetupQuadVertsAndFaces(mesh, quad, vertex, face);

    quad.mOffset = offset(params.mAxisZ, -0.5F);
    quad.mAxisU = Negate(quad.mAxisU);
    _SetupQuadVertsAndFaces(mesh, quad, vertex, face);

    _CreateMeshCoda(*mesh, params);
    return mesh;
}

// Reconstructed from eboot.elf at 0x5DD0E0. The profile runs from the -z pole
// to the +z pole. With kCreateMeshCircumscribe, the ring radius grows by
// 1/cos(pi/(2*rings)) and the profile's x by 1/cos(pi/segments), so the flat
// facets enclose the sphere.
RndMesh* RndMeshUtl::CreateSphere(const CreateSphereParams& params) {
    gTmpContour.clear();
    gTmpContour.reserve(params.mNumRings + 1);

    float segmentScale = 0.0F;
    float ringRadius = params.mRadius;
    if ((params.mFlags & kCreateMeshCircumscribe) != 0) {
        const float ringCos =
            Sine(kHalfPi / static_cast<float>(params.mNumRings) + kHalfPi);
        const float segmentCos =
            Sine(kPi / static_cast<float>(params.mNumSegments) + kHalfPi);
        segmentScale = 1.0F / segmentCos;
        ringRadius = params.mRadius * (1.0F / ringCos);
    }

    for (unsigned long ring = 0; ring <= params.mNumRings; ++ring) {
        Vector3 normal;
        if (ring == 0) {
            normal = Negate(Vector3::sZ);
        } else if (ring == params.mNumRings) {
            normal = Vector3::sZ;
        } else {
            const float angle =
                ring / static_cast<float>(params.mNumRings) * kPi;
            normal = {Sine(angle), 0.0F, -Sine(angle + kHalfPi)};
        }
        Vector3 pos = {
            normal.x * ringRadius, normal.y * ringRadius, normal.z * ringRadius};
        if ((params.mFlags & kCreateMeshCircumscribe) != 0) {
            pos.x *= segmentScale;
        }
        gTmpContour.push_back({pos, normal});
    }

    CreateRadialSurfaceParams radial = params;
    if (radial.mName == nullptr) {
        radial.mName = "Sphere";
    }
    return CreateRadialSurface(gTmpContour, radial);
}

// Reconstructed from eboot.elf at 0x5DD5E0. Each contour point becomes a ring
// of mNumSegments + 1 vertices. An edge between two equal points gets no
// faces, an edge with one end on the axis gets a fan of triangles, and any
// other edge gets a strip of quads. The second side's faces reuse the first
// side's vertices and winding, as in the binary.
RndMesh* RndMeshUtl::CreateRadialSurface(
    const eastl::vector<ContourVertex>& contour,
    const CreateRadialSurfaceParams& params) {
    RndMesh* mesh = _CreateMeshPrelude(params, "Radial Surface");
    const unsigned long numPoints = contour.size();
    const unsigned long numSegments = params.mNumSegments;

    unsigned long numFaces = 0;
    if (numPoints != 1) {
        for (unsigned long edge = 0; edge < numPoints - 1; ++edge) {
            const Vector3& start = contour[edge].mPos;
            const Vector3& end = contour[edge + 1].mPos;
            if (SamePoint(start, end)) {
                continue;
            }
            const bool startOnAxis = OnAxis(start);
            const bool endOnAxis = OnAxis(end);
            if (!startOnAxis && !endOnAxis) {
                numFaces += 2 * numSegments;
            } else if (!(startOnAxis && endOnAxis)) {
                numFaces += numSegments;
            }
        }
    }
    numFaces <<= DoubleSidedShift(params);
    mesh->mFaces.resize(numFaces);

    const bool alternate = (params.mFlags & kCreateMeshAlternateDiagonals) != 0;
    const unsigned int stride = static_cast<unsigned int>(numSegments + 1);
    unsigned long face = 0;
    for (unsigned long side = 0; side < 2; ++side) {
        if (side != 0 && (params.mFlags & kCreateMeshDoubleSided) == 0) {
            continue;
        }
        if (numPoints == 1) {
            continue;
        }
        for (unsigned long edge = 0; edge < numPoints - 1; ++edge) {
            const Vector3& start = contour[edge].mPos;
            const Vector3& end = contour[edge + 1].mPos;
            if (SamePoint(start, end) || numSegments == 0) {
                continue;
            }
            const unsigned int ringStart = static_cast<unsigned int>(edge) * stride;
            if (OnAxis(start)) {
                for (unsigned long segment = 0; segment < numSegments; ++segment) {
                    const unsigned int v = ringStart + static_cast<unsigned int>(segment);
                    SetFace(mesh->mFaces[face++], v, v + stride + 1, v + stride);
                }
            } else if (OnAxis(end)) {
                for (unsigned long segment = 0; segment < numSegments; ++segment) {
                    const unsigned int v = ringStart + static_cast<unsigned int>(segment);
                    SetFace(mesh->mFaces[face++], v, v + 1, v + stride + 1);
                }
            } else {
                for (unsigned long segment = 0; segment < numSegments; ++segment) {
                    const unsigned int v = ringStart + static_cast<unsigned int>(segment);
                    const bool flip = alternate && ((segment ^ edge) & 1) != 0;
                    SetFace(mesh->mFaces[face], v, v + 1, flip ? v + stride : v + stride + 1);
                    SetFace(mesh->mFaces[face + 1], flip ? v + 1 : v, v + stride + 1, v + stride);
                    face += 2;
                }
            }
        }
    }

    mesh->_SetNumVerticesImpl(((numSegments + 1) * numPoints) << DoubleSidedShift(params));
    ReshapeRadialSurface(*mesh, contour, params);
    _CreateMeshCoda(*mesh, params);
    return mesh;
}

// Reconstructed from eboot.elf at 0x5DDC00. The profile closes both ends with
// flat caps, repeating the rim points so the caps and the side get separate
// normals.
RndMesh* RndMeshUtl::CreateCylinder(const CreateCylinderParams& params) {
    gTmpContour.clear();
    gTmpContour.reserve(params.mNumHeightSegments + 5);

    const float radius = params.mRadius;
    const float bottom = params.mHeight * -0.5F;
    gTmpContour.push_back({{0.0F, 0.0F, bottom}, Negate(Vector3::sZ)});
    gTmpContour.push_back({{radius, 0.0F, bottom}, Negate(Vector3::sZ)});
    gTmpContour.push_back({{radius, 0.0F, bottom}, Vector3::sX});
    for (unsigned long segment = 1; segment < params.mNumHeightSegments; ++segment) {
        const float height =
            (segment / static_cast<float>(params.mNumHeightSegments) + -0.5F) *
            params.mHeight;
        gTmpContour.push_back({{radius, 0.0F, height}, Vector3::sX});
    }
    const float top = params.mHeight * 0.5F;
    gTmpContour.push_back({{radius, 0.0F, top}, Vector3::sX});
    gTmpContour.push_back({{radius, 0.0F, top}, Vector3::sZ});
    gTmpContour.push_back({{0.0F, 0.0F, top}, Vector3::sZ});

    CreateRadialSurfaceParams radial = params;
    if (radial.mName == nullptr) {
        radial.mName = "Cylinder";
    }
    return CreateRadialSurface(gTmpContour, radial);
}

// Reconstructed from eboot.elf at 0x5DE890.
RndMesh* RndMeshUtl::CreateCapsule(const CreateCapsuleParams& params) {
    _GenerateCapsuleContour(params, gTmpContour);
    CreateRadialSurfaceParams radial = params;
    if (radial.mName == nullptr) {
        radial.mName = "Capsule";
    }
    return CreateRadialSurface(gTmpContour, radial);
}

// Reconstructed from eboot.elf at 0x5DE910. The profile runs from the -z
// pole over the bottom hemisphere, up the side and over the top hemisphere
// to the +z pole. The hemispheres are centered at -mLength/2 and
// +mLength/2, and the side adds only its inner points.
void RndMeshUtl::_GenerateCapsuleContour(
    const CreateCapsuleParams& params,
    eastl::vector<ContourVertex>& contour) {
    contour.clear();
    contour.reserve(params.mNumSideSegments + 2 * params.mNumCapSegments + 1);

    const float length = params.mLength;
    const float bottom = length * -0.5F;
    for (unsigned long segment = 0; segment <= params.mNumCapSegments; ++segment) {
        Vector3 normal;
        if (segment == 0) {
            normal = Negate(Vector3::sZ);
        } else if (segment == params.mNumCapSegments) {
            normal = Vector3::sX;
        } else {
            const float angle =
                (1.0F - segment / static_cast<float>(params.mNumCapSegments)) * kHalfPi;
            normal = {Sine(angle + kHalfPi), 0.0F, Sine(angle + kPi)};
        }
        const float radius = params.mRadius;
        contour.push_back(
            {{radius * normal.x, radius * normal.y, radius * normal.z + bottom}, normal});
    }

    for (unsigned long segment = 1; segment < params.mNumSideSegments; ++segment) {
        const float height =
            segment / static_cast<float>(params.mNumSideSegments) * params.mLength + bottom;
        contour.push_back({{params.mRadius, 0.0F, height}, Vector3::sX});
    }

    const float top = length * 0.5F;
    for (unsigned long segment = 0; segment <= params.mNumCapSegments; ++segment) {
        Vector3 normal;
        if (segment == 0) {
            normal = Vector3::sX;
        } else if (segment == params.mNumCapSegments) {
            normal = Vector3::sZ;
        } else {
            const float angle =
                (1.0F - segment / static_cast<float>(params.mNumCapSegments)) * kHalfPi;
            normal = {Sine(angle), 0.0F, Sine(angle + kHalfPi)};
        }
        const float radius = params.mRadius;
        contour.push_back(
            {{radius * normal.x, radius * normal.y, radius * normal.z + top}, normal});
    }
}

// Reconstructed from eboot.elf at 0x5DF180. Sweeps each contour point around
// the z axis. U runs around the sweep; V runs from 1 at the start of the
// contour to 0 at its end, by arc length. The tangent follows the sweep and
// the bitangent is -(normal x tangent). The second side repeats the vertices
// with flipped normals.
void RndMeshUtl::ReshapeRadialSurface(
    RndMesh& mesh,
    const eastl::vector<ContourVertex>& contour,
    const CreateRadialSurfaceParams& params) {
    using Interp = RndVertexInterpreter;
    const unsigned long numPoints = contour.size();
    gTmpUVIntervals.resize(numPoints);
    gTmpUVIntervals[0] = 0.0F;
    float length = 0.0F;
    for (unsigned long point = 1; point < numPoints; ++point) {
        const Vector3& a = contour[point - 1].mPos;
        const Vector3& b = contour[point].mPos;
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        length += std::sqrt(dx * dx + (dy * dy + dz * dz));
        gTmpUVIntervals[point] = length;
    }
    for (unsigned long point = 0; point < numPoints; ++point) {
        const float t = gTmpUVIntervals[point] / length;
        float v = 1.0F - t;
        if (std::fabs(v) <= kEpsilon) {
            v = 0.0F;
        } else if (std::fabs(t) <= kEpsilon) {
            v = 1.0F;
        }
        gTmpUVIntervals[point] = v;
    }

    const RndVertexInterpreter* interp =
        RndVertexInterpreter::GetInstance(params.mVertexType);
    // The binary reads the vertex count and discards it.
    mesh._GetNumVerticesImpl();

    unsigned long vertex = 0;
    for (unsigned long side = 0; side < 2; ++side) {
        if (side != 0 && (params.mFlags & kCreateMeshDoubleSided) == 0) {
            continue;
        }
        for (unsigned long point = 0; point < contour.size(); ++point) {
            const ContourVertex& profile = contour[point];
            for (unsigned long segment = 0; segment <= params.mNumSegments;
                 ++segment, ++vertex) {
                void* data = mesh._GetVertexVoidImpl(vertex);
                const float u = segment / static_cast<float>(params.mNumSegments);
                float angle = u * params.mSweepAngle;
                if (std::fabs(angle + -kTwoPi) <= kEpsilon) {
                    angle = 0.0F;
                }
                const float sine = Sine(angle);
                const float cosine = Sine(angle + kHalfPi);

                const Vector3 pos = {
                    cosine * profile.mPos.x + params.mOffset.x,
                    sine * profile.mPos.x + params.mOffset.y,
                    profile.mPos.z + params.mOffset.z,
                };
                SetAttribute(interp, Interp::kPositionAttribute, data, pos);
                if (HasAttribute(interp, Interp::kNormalAttribute)) {
                    Vector3 normal = {
                        profile.mNormal.x * cosine,
                        profile.mNormal.x * sine,
                        profile.mNormal.z,
                    };
                    if (HasAttribute(interp, Interp::kTangentAttribute)) {
                        const Vector3 tangent = {-sine, cosine, 0.0F};
                        SetAttribute(interp, Interp::kTangentAttribute, data, tangent);
                        const Vector3 bitangent = Negate(Normalize(Cross(normal, tangent)));
                        SetAttribute(interp, Interp::kBitangentAttribute, data, bitangent);
                    }
                    if (side != 0) {
                        normal = Negate(normal);
                    }
                    SetAttribute(interp, Interp::kNormalAttribute, data, normal);
                }
                SetDefaultAttributes(interp, data, {u, gTmpUVIntervals[point]});
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x5E0240.
RndMesh* RndMeshUtl::CreateNestedCone(const CreateNestedConeParams& params) {
    _GenerateNestedConeContour(params, gTmpContour);
    CreateRadialSurfaceParams radial = params;
    if (radial.mName == nullptr) {
        radial.mName = "NestedCone";
    }
    return CreateRadialSurface(gTmpContour, radial);
}

// Reconstructed from eboot.elf at 0x5E02C0. The profile runs from the apex
// out along the outer cone to its rim, optionally across to the inner
// cone's rim, then back along the inner cone to the apex. The outer cone's
// normal faces away from the axis and the inner cone's toward it; the join
// uses the normalized midpoint of the two rims. The binary divides the
// slopes with a reciprocal estimate refined by one Newton step, and
// normalizes with a reciprocal square root estimate refined the same way.
void RndMeshUtl::_GenerateNestedConeContour(
    const CreateNestedConeParams& params,
    eastl::vector<ContourVertex>& contour) {
    contour.clear();
    const Vector2 radii = {
        params.mRadii.x < 0.0F ? 0.0F : params.mRadii.x,
        params.mRadii.y < 0.0F ? 0.0F : params.mRadii.y,
    };
    contour.reserve(
        2 * params.mNumConeSegments + 2 * static_cast<unsigned long>(params.mJoinRims) + 2);

    const float slopeX = params.mHeights.x / (radii.x > kEpsilon ? radii.x : kEpsilon);
    const float slopeY = params.mHeights.y / (radii.y > kEpsilon ? radii.y : kEpsilon);
    float outerRadius;
    float outerHeight;
    float innerRadius;
    float innerHeight;
    if (slopeX > slopeY) {
        innerRadius = radii.x;
        innerHeight = params.mHeights.x;
        outerRadius = radii.y;
        outerHeight = params.mHeights.y;
    } else {
        innerRadius = radii.y;
        innerHeight = params.mHeights.y;
        outerRadius = radii.x;
        outerHeight = params.mHeights.x;
    }

    const float outerScale =
        1.0F / std::sqrt(outerHeight * outerHeight + outerRadius * outerRadius);
    const Vector3 outerNormal = {
        outerScale * outerHeight, 0.0F, -(outerRadius * outerScale)};
    for (unsigned long segment = 0; segment <= params.mNumConeSegments; ++segment) {
        Vector3 pos = {0.0F, 0.0F, 0.0F};
        if (segment != 0) {
            pos = {outerRadius, 0.0F, outerHeight};
            if (segment != params.mNumConeSegments) {
                const float t = segment / static_cast<float>(params.mNumConeSegments);
                pos = {t * outerRadius, 0.0F, t * outerHeight};
            }
        }
        contour.push_back({pos, outerNormal});
    }

    if (params.mJoinRims) {
        const float midRadius = (outerRadius + innerRadius) * 0.5F;
        const float midHeight = (outerHeight + innerHeight) * 0.5F;
        const float length = std::sqrt(midHeight * midHeight + midRadius * midRadius);
        const float scale = length != 0.0F ? 1.0F / length : 0.0F;
        const Vector3 joinNormal = {scale * midRadius, 0.0F, scale * midHeight};
        contour.push_back({{outerRadius, 0.0F, outerHeight}, joinNormal});
        contour.push_back({{innerRadius, 0.0F, innerHeight}, joinNormal});
    }

    const float innerScale =
        1.0F / std::sqrt(innerHeight * innerHeight + innerRadius * innerRadius);
    const Vector3 innerNormal = {
        -(innerHeight * innerScale), 0.0F, innerScale * innerRadius};
    for (unsigned long segment = 0; segment <= params.mNumConeSegments; ++segment) {
        Vector3 pos = {innerRadius, 0.0F, innerHeight};
        if (segment != 0) {
            pos = {0.0F, 0.0F, 0.0F};
            if (segment != params.mNumConeSegments) {
                const float t =
                    1.0F - segment / static_cast<float>(params.mNumConeSegments);
                pos = {t * innerRadius, 0.0F, t * innerHeight};
            }
        }
        contour.push_back({pos, innerNormal});
    }
}

// Reconstructed from eboot.elf at 0x5E0C10.
RndMesh* RndMeshUtl::CreateTruncatedRoundedCone(
    const CreateTruncatedRoundedConeParams& params) {
    _GenerateTruncatedRoundedConeContour(params, gTmpContour);
    CreateRadialSurfaceParams radial = params;
    if (radial.mName == nullptr) {
        radial.mName = "TruncatedRoundedCone";
    }
    return CreateRadialSurface(gTmpContour, radial);
}

// Reconstructed from eboot.elf at 0x5E0C90. The profile runs from the bottom
// pole over the cap (half of mNumCapSegments), up the side to the top rim
// (mNumSideSegments), then across the top to the axis (mNumTopSegments).
void RndMeshUtl::_GenerateTruncatedRoundedConeContour(
    const CreateTruncatedRoundedConeParams& params,
    eastl::vector<ContourVertex>& contour) {
    const TruncatedRoundedCone& cone = params.mCone;
    contour.clear();
    const unsigned long numCapSegments = params.mNumCapSegments / 2;
    contour.reserve(
        numCapSegments + params.mNumTopSegments + params.mNumSideSegments + 3);

    const float bottomRadius = std::fmax(kEpsilon, cone.mBottomRadius);
    const float angle = std::fmax(kEpsilon, cone.mAngle);
    for (unsigned long segment = 0;; ++segment) {
        Vector3 pos;
        Vector3 normal;
        if (segment == 0) {
            pos = {0.0F, 0.0F, -cone.mLength};
            normal = {0.0F, 0.0F, -1.0F};
        } else {
            const float capAngle =
                segment / static_cast<float>(numCapSegments) * angle;
            const float cosine = Sine(capAngle + kHalfPi);
            const float sine = Sine(capAngle);
            normal = {sine, 0.0F, -cosine};
            if (segment == numCapSegments) {
                pos = {bottomRadius, 0.0F, -cone.mConeLength};
            } else {
                const float radius = cone.mCapRadius;
                pos = {
                    radius * sine,
                    0.0F,
                    (radius - cone.mLength) + radius * -cosine,
                };
            }
        }
        contour.push_back({pos, normal});
        if (segment == numCapSegments) {
            break;
        }
    }

    const float topRadius = std::fmax(kEpsilon, cone.mTopRadius);
    const Vector3 sideNormal = {Sine(angle + kHalfPi), 0.0F, Sine(angle)};
    const float radiusChange = topRadius - bottomRadius;
    for (unsigned long segment = 0; segment <= params.mNumSideSegments; ++segment) {
        Vector3 pos;
        if (segment == 0) {
            pos = contour.back().mPos;
        } else if (segment == params.mNumSideSegments) {
            pos = {topRadius, 0.0F, 0.0F};
        } else {
            const float t = segment / static_cast<float>(params.mNumSideSegments);
            pos = {t * radiusChange + bottomRadius, 0.0F, (t + -1.0F) * cone.mConeLength};
        }
        contour.push_back({pos, sideNormal});
    }

    const Vector3 topNormal = Vector3::sZ;
    for (unsigned long segment = 0; segment <= params.mNumTopSegments; ++segment) {
        float radius = topRadius;
        if (segment != 0) {
            radius = 0.0F;
            if (segment != params.mNumTopSegments) {
                radius = (1.0F -
                          segment / static_cast<float>(params.mNumTopSegments)) *
                         topRadius;
            }
        }
        contour.push_back({{radius, 0.0F, 0.0F}, topNormal});
    }
}
