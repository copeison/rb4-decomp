#include "render/drawing/RndDrawUtl.h"

#include "render/context/RndContext.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/meshes/RndVertex.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"

namespace {

// The shared meshes the drawing helpers use. Names not in the reference map;
// the map's object keeps them as unnamed file statics.
RndMesh* gSphereMesh = nullptr;                // 0x1A712E0
RndMesh* gDiscMesh = nullptr;                  // 0x1A712E8
RndMesh* gFacingQuadMeshes[6] = {};            // 0x1A712F0
RndMesh* gBoxMesh = nullptr;                   // 0x1A71320
RndMesh* gQuadMesh = nullptr;                  // 0x1A71328
RndMesh* gQuadGridMesh = nullptr;              // 0x1A71330
RndMesh* gStaticBoxMesh = nullptr;             // 0x1A71338
RndMesh* gCapsuleMesh = nullptr;               // 0x1A71340
RndMesh* gNestedConeMesh = nullptr;            // 0x1A71348
RndMesh* gTruncatedRoundedConeMesh = nullptr;  // 0x1A71350

constexpr float kQuarterPi = 0.78539819F;

// Name not in the reference map.
void SafeDelete(RndMesh*& mesh) {
    delete mesh;
    mesh = nullptr;
}

// A unit box on the axes. Name not in the reference map.
RndMeshUtl::CreateBoxParams UnitBoxParams() {
    RndMeshUtl::CreateBoxParams params;
    params.mAxisX = {1.0F, 0.0F, 0.0F};
    params.mAxisY = {0.0F, 1.0F, 0.0F};
    params.mAxisZ = {0.0F, 0.0F, 1.0F};
    params.mNumSegmentsX = 1;
    params.mNumSegmentsY = 1;
    params.mNumSegmentsZ = 1;
    return params;
}

// A unit quad in the xy plane. Name not in the reference map.
RndMeshUtl::CreateQuadParams UnitQuadParams(int numSegments) {
    RndMeshUtl::CreateQuadParams params;
    params.mVertexUsageFlags = 1;
    params.mAxisU = {1.0F, 0.0F, 0.0F};
    params.mAxisV = {0.0F, 1.0F, 0.0F};
    params.mNumSegmentsU = numSegments;
    params.mNumSegmentsV = numSegments;
    return params;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DF170. Every mesh uses unskinned
// vertices and the builders' default names.
void RndDrawUtl::Init() {
    {
        RndMeshUtl::CreateSphereParams params;
        params.mRadius = 1.0F;
        params.mNumRings = 16;
        gSphereMesh = RndMeshUtl::CreateSphere(params);
    }
    {
        RndMeshUtl::CreateTriangleFanParams params;
        params.mFacing = 2;
        params.mNumSegments = 64;
        params.mRadius = 1.0F;
        gDiscMesh = RndMeshUtl::CreateTriangleFan(params);
    }
    for (unsigned int facing = 0; facing < 6; ++facing) {
        RndMeshUtl::CreateFacingQuadParams params;
        params.mFacing = facing;
        params.mWidth = 1.0F;
        params.mHeight = 1.0F;
        params.mNumSegmentsU = 1;
        params.mNumSegmentsV = 1;
        gFacingQuadMeshes[facing] = RndMeshUtl::CreateFacingQuad(params);
    }
    gBoxMesh = RndMeshUtl::CreateBox(UnitBoxParams());
    gQuadMesh = RndMeshUtl::CreateQuad(UnitQuadParams(1));
    gQuadGridMesh = RndMeshUtl::CreateQuad(UnitQuadParams(3));
    {
        RndMeshUtl::CreateBoxParams params = UnitBoxParams();
        params.mVertexUsageFlags = 1;
        gStaticBoxMesh = RndMeshUtl::CreateBox(params);
    }
    {
        RndMeshUtl::CreateCapsuleParams params;
        params.mVertexUsageFlags = 1;
        params.mRadius = 1.0F;
        params.mLength = 2.0F;
        params.mNumCapSegments = 8;
        params.mNumSideSegments = 1;
        gCapsuleMesh = RndMeshUtl::CreateCapsule(params);
    }
    {
        RndMeshUtl::CreateNestedConeParams params;
        params.mVertexUsageFlags = 1;
        params.mUnknown72[0] = 0.5F;
        params.mUnknown72[1] = 1.0F;
        params.mUnknown80[0] = 2.0F;
        params.mUnknown80[1] = 2.0F;
        params.mUnknown88 = 1;
        params.mUnknown96 = true;
        gNestedConeMesh = RndMeshUtl::CreateNestedCone(params);
    }
    {
        RndMeshUtl::CreateTruncatedRoundedConeParams params;
        params.mCone.SetAngleTopRadiusAndLength(kQuarterPi, 0.5F, 2.0F);
        params.mVertexUsageFlags = 1;
        gTruncatedRoundedConeMesh = RndMeshUtl::CreateTruncatedRoundedCone(params);
    }
}

// Reconstructed from eboot.elf at 0x3DF820. The nested cone mesh is not
// released.
void RndDrawUtl::Terminate() {
    SafeDelete(gSphereMesh);
    SafeDelete(gDiscMesh);
    for (RndMesh*& mesh : gFacingQuadMeshes) {
        SafeDelete(mesh);
    }
    SafeDelete(gBoxMesh);
    SafeDelete(gQuadMesh);
    SafeDelete(gQuadGridMesh);
    SafeDelete(gStaticBoxMesh);
    SafeDelete(gCapsuleMesh);
    SafeDelete(gTruncatedRoundedConeMesh);
}

// Reconstructed from eboot.elf at 0x3E0C50. The left and top edges are
// converted to clip space by mode; the right and bottom edges are first
// brought to pixels.
void RndDrawUtl::DrawQuad2D(RndContext& context, Quad2DParams& params) {
    const bool identity = context.mUsingIdentityViewProjection;
    context.SetUsingIdentityViewProjection(true);

    const float width = context.mViewportSize.x;
    const float height = context.mViewportSize.y;
    const auto& rect = params.mRect;
    float left;
    float top;
    float right;
    float bottom;
    switch (params.mCoordinateMode) {
    case kCoordinatePixels:
        left = (rect.x + rect.x) / width + -1.0F;
        top = (rect.y + rect.y) / height + -1.0F;
        right = rect.x + rect.w;
        bottom = rect.y + rect.h;
        break;
    case kCoordinateNormalized:
        left = width * (rect.x + rect.x) / width + -1.0F;
        top = height * (rect.y + rect.y) / height + -1.0F;
        right = (rect.x + rect.w) * width;
        bottom = (rect.y + rect.h) * height;
        break;
    case kCoordinateAspectCorrected: {
        const float offset = (width - height) * 0.5F;
        const float x = offset + rect.x * height;
        left = (x + x) / width + -1.0F;
        top = height * (rect.y + rect.y) / height + -1.0F;
        right = (rect.x + rect.w) * height + offset;
        bottom = (rect.y + rect.h) * height;
        break;
    }
    case kCoordinateClip:
        left = width * (rect.x + 1.0F) / width + -1.0F;
        top = height * (rect.y + 1.0F) / height + -1.0F;
        right = width * 0.5F * (rect.x + rect.w + 1.0F);
        bottom = height * 0.5F * (rect.y + rect.h + 1.0F);
        break;
    default:
        left = -1.0F;
        top = -1.0F;
        right = 0.0F;
        bottom = 0.0F;
        break;
    }
    right = (right + right) / width + -1.0F;
    bottom = (bottom + bottom) / height + -1.0F;

    RndVertexColorTex vertices[4];
    unsigned long count;
    const float z = params.mZ;
    if (bottom - top == 2.0F && top == -1.0F && left == -1.0F && right - left == 2.0F) {
        vertices[0].mPos[0] = -1.0F;
        vertices[0].mPos[1] = 1.0F;
        vertices[0].mPos[2] = z;
        vertices[0].mTex[0] = 0.0F;
        vertices[0].mTex[1] = 0.0F;
        vertices[1].mPos[0] = -1.0F;
        vertices[1].mPos[1] = -3.0F;
        vertices[1].mPos[2] = z;
        vertices[1].mTex[0] = 0.0F;
        vertices[1].mTex[1] = 2.0F;
        vertices[2].mPos[0] = 3.0F;
        vertices[2].mPos[1] = 1.0F;
        vertices[2].mPos[2] = z;
        vertices[2].mTex[0] = 2.0F;
        vertices[2].mTex[1] = 0.0F;
        count = 3;
    } else {
        vertices[0].mPos[0] = left;
        vertices[0].mPos[1] = top;
        vertices[0].mPos[2] = z;
        vertices[0].mTex[0] = 0.0F;
        vertices[0].mTex[1] = 1.0F;
        vertices[1].mPos[0] = right;
        vertices[1].mPos[1] = top;
        vertices[1].mPos[2] = z;
        vertices[1].mTex[0] = 1.0F;
        vertices[1].mTex[1] = 1.0F;
        vertices[2].mPos[0] = left;
        vertices[2].mPos[1] = bottom;
        vertices[2].mPos[2] = z;
        vertices[2].mTex[0] = 0.0F;
        vertices[2].mTex[1] = 0.0F;
        vertices[3].mPos[0] = right;
        vertices[3].mPos[1] = bottom;
        vertices[3].mPos[2] = z;
        vertices[3].mTex[0] = 1.0F;
        vertices[3].mTex[1] = 0.0F;
        count = 4;
    }
    for (unsigned long index = 0; index < count; ++index) {
        vertices[index].mColor[0] = params.mColor.red;
        vertices[index].mColor[1] = params.mColor.green;
        vertices[index].mColor[2] = params.mColor.blue;
        vertices[index].mColor[3] = params.mColor.alpha;
    }

    if (!params.mKeepState) {
        context.mBlendMode = params.mBlendMode;
        context._SetBlendModeImpl(params.mBlendMode, Hmx::Color::GetWhite());
        context._SetDepthModeImpl(params.mDepthMode);
        context._SetCullModeImpl(kCullNone);
    }
    if (!params.mKeepShader) {
        const auto shadingMode = context.mShadingMode;
        context.SetShadingMode(kShadingModeStandard);
        RndShaderBasic::Params basic;
        basic.mAlphaCut = params.mAlphaCut;
        basic.mTexture = params.mTexture;
        basic.mRTSlicedTexture = params.mRTSlicedTexture;
        TheRndDevice()->mShaderMgr.mBasicShader->Select(context, basic);
        context.SetShadingMode(shadingMode);
    }
    context._DrawPrimitivesImpl(
        RndPrimitive::kTriangleStrip, RndVertexColorTex::kType, vertices, count);
    context.SetUsingIdentityViewProjection(identity);
}
