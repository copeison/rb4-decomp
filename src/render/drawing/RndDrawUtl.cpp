#include "render/drawing/RndDrawUtl.h"

#include "render/context/RndContext.h"
#include "render/meshes/RndVertex.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"

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
