#include "render/drawing/RndDrawUtl.h"

#include <cstring>
#include <limits>

#include "math/scalar/Trig.h"
#include "math/vector/Vector2i.h"
#include "render/context/RndContext.h"
#include "render/debug/RndDebugFont.h"
#include "render/fonts/RndFont.h"
#include "render/fonts/RndFontPage.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/meshes/RndVertex.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture2D.h"
#include "utl/containers/FixedVector.h"
#include "utl/text/UTF8.h"

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
constexpr float kHalfPi = 1.5707964F;

// Text rotations and their sines below this are treated as zero. Name not
// in the reference map.
constexpr float kRotationEpsilon = 1e-4F;

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

// The state DrawLines2D and DrawQuadWireframe2D draw with: an identity
// view-projection, the line's blend mode without depth testing or culling,
// and the basic shader. Returns the shading mode to restore. Name not in
// the reference map.
RndShadingMode BeginLines2D(RndContext& context, const RndDrawUtl::Line2DParams& params) {
    context.SetUsingIdentityViewProjection(true);
    context.mBlendMode = params.mBlendMode;
    context._SetBlendModeImpl(params.mBlendMode, Hmx::Color::GetWhite());
    context._SetDepthModeImpl(0);
    context._SetCullModeImpl(kCullNone);
    context._SetThickLinesImpl(params.mThickLines);
    const RndShadingMode shadingMode = context.mShadingMode;
    context.SetShadingMode(kShadingModeStandard);
    TheRndDevice()->mShaderMgr.mBasicShader->Select(context, RndShaderBasic::Params());
    return shadingMode;
}

// A point in the mode's space converted straight to clip space. Name not
// in the reference map.
Vector2 ToClip(
    RndDrawUtl::CoordinateMode mode,
    const Vector2& viewportSize,
    const Vector2& point) {
    const float width = viewportSize.x;
    const float height = viewportSize.y;
    switch (mode) {
    case RndDrawUtl::kCoordinatePixels:
        return {point.x * (2.0F / width) + -1.0F, point.y * (2.0F / height) + -1.0F};
    case RndDrawUtl::kCoordinateNormalized:
        return {point.x * width * (2.0F / width) + -1.0F,
                point.y * height * (2.0F / height) + -1.0F};
    case RndDrawUtl::kCoordinateAspectCorrected:
        return {((width - height) * 0.5F + point.x * height) * (2.0F / width) + -1.0F,
                point.y * height * (2.0F / height) + -1.0F};
    case RndDrawUtl::kCoordinateClip:
        return {(point.x + 1.0F) * 0.5F * width * (2.0F / width) + -1.0F,
                (point.y + 1.0F) * 0.5F * height * (2.0F / height) + -1.0F};
    default:
        return {-1.0F, -1.0F};
    }
}

// A point in the mode's space converted to pixels. Name not in the
// reference map.
Vector2 ToPixels(
    RndDrawUtl::CoordinateMode mode,
    const Vector2& viewportSize,
    const Vector2& point) {
    const float width = viewportSize.x;
    const float height = viewportSize.y;
    switch (mode) {
    case RndDrawUtl::kCoordinatePixels:
        return point;
    case RndDrawUtl::kCoordinateNormalized:
        return {point.x * width, point.y * height};
    case RndDrawUtl::kCoordinateAspectCorrected:
        return {(width - height) * 0.5F + point.x * height, point.y * height};
    case RndDrawUtl::kCoordinateClip:
        return {(point.x + 1.0F) * (width * 0.5F), (point.y + 1.0F) * (height * 0.5F)};
    default:
        return {0.0F, 0.0F};
    }
}

// An x or y coordinate in the mode's space converted to pixels. Names not
// in the reference map.
float ToPixelsX(RndDrawUtl::CoordinateMode mode, const Vector2& viewportSize, float x) {
    switch (mode) {
    case RndDrawUtl::kCoordinatePixels:
        return x;
    case RndDrawUtl::kCoordinateNormalized:
        return x * viewportSize.x;
    case RndDrawUtl::kCoordinateAspectCorrected:
        return x * viewportSize.y + (viewportSize.x - viewportSize.y) * 0.5F;
    case RndDrawUtl::kCoordinateClip:
        return (x + 1.0F) * (viewportSize.x * 0.5F);
    default:
        return 0.0F;
    }
}
float ToPixelsY(RndDrawUtl::CoordinateMode mode, const Vector2& viewportSize, float y) {
    switch (mode) {
    case RndDrawUtl::kCoordinatePixels:
        return y;
    case RndDrawUtl::kCoordinateNormalized:
    case RndDrawUtl::kCoordinateAspectCorrected:
        return y * viewportSize.y;
    case RndDrawUtl::kCoordinateClip:
        return (y + 1.0F) * (viewportSize.y * 0.5F);
    default:
        return 0.0F;
    }
}

// A point in pixels converted to the mode's space. Name not in the
// reference map.
Vector2 FromPixels(
    RndDrawUtl::CoordinateMode mode,
    const Vector2& viewportSize,
    const Vector2& point) {
    switch (mode) {
    case RndDrawUtl::kCoordinatePixels:
        return point;
    case RndDrawUtl::kCoordinateNormalized:
        return {point.x / viewportSize.x, point.y / viewportSize.y};
    case RndDrawUtl::kCoordinateAspectCorrected:
        return {(point.x + (viewportSize.y - viewportSize.x) * 0.5F) / viewportSize.y,
                point.y / viewportSize.y};
    case RndDrawUtl::kCoordinateClip:
        return {point.x * (2.0F / viewportSize.x) + -1.0F,
                point.y * (2.0F / viewportSize.y) + -1.0F};
    default:
        return {0.0F, 0.0F};
    }
}

// Rounds half away from zero, saturating at the int range. Name not in the
// reference map.
int RoundToInt(float value) {
    if (value > 0.0F) {
        value += 0.5F;
        return value < 2147483648.0F ? static_cast<int>(value)
                                     : std::numeric_limits<int>::max();
    }
    value -= 0.5F;
    return value > -2147483648.0F ? static_cast<int>(value)
                                  : std::numeric_limits<int>::min();
}

// Name not in the reference map.
Vector2 PixelsToClip(const Vector2& viewportSize, const Vector2& point) {
    return {(point.x + point.x) / viewportSize.x + -1.0F,
            (point.y + point.y) / viewportSize.y + -1.0F};
}

// Name not in the reference map.
void SetLineVertex(RndVertexColor& vertex, const Vector2& position, const Hmx::Color& color) {
    vertex.mPos[0] = position.x;
    vertex.mPos[1] = position.y;
    vertex.mPos[2] = 0.0F;
    vertex.mColor[0] = color.red;
    vertex.mColor[1] = color.green;
    vertex.mColor[2] = color.blue;
    vertex.mColor[3] = color.alpha;
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
        params.mRadii = {0.5F, 1.0F};
        params.mHeights = {2.0F, 2.0F};
        params.mNumConeSegments = 1;
        params.mJoinRims = true;
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

// Reconstructed from eboot.elf at 0x3DFC50.
void RndDrawUtl::DrawLine2D(
    RndContext& context,
    const Segment2D& segment,
    const Line2DParams& params) {
    FixedVector<Segment2D, 1> segments;
    segments.push_back(segment);
    DrawLines2D(context, VectorAdapter<Segment2D>{segments.begin(), segments.size()}, params);
}

// Reconstructed from eboot.elf at 0x3DFCB0. The vertices live on the stack.
void RndDrawUtl::DrawLines2D(
    RndContext& context,
    const VectorAdapter<Segment2D>& segments,
    const Line2DParams& params) {
    const bool identity = context.mUsingIdentityViewProjection;
    const RndShadingMode shadingMode = BeginLines2D(context, params);
    const unsigned long count = segments.mSize * 2;
    RndVertexColor* vertices = nullptr;
    if (count != 0) {
        vertices = static_cast<RndVertexColor*>(
            __builtin_alloca(count * sizeof(RndVertexColor)));
    }
    const Vector2 viewportSize = context.mViewportSize;
    for (unsigned long index = 0; index < segments.mSize; ++index) {
        const Segment2D& segment = segments.mData[index];
        SetLineVertex(
            vertices[index * 2],
            ToClip(params.mCoordinateMode, viewportSize, segment.start),
            params.mColor);
        SetLineVertex(
            vertices[index * 2 + 1],
            PixelsToClip(viewportSize, ToPixels(params.mCoordinateMode, viewportSize, segment.end)),
            params.mColor);
    }
    if (count != 0) {
        context._DrawPrimitivesImpl(
            RndPrimitive::kLines, RndVertexColor::kType, vertices, count);
    }
    context.SetShadingMode(shadingMode);
    context.SetUsingIdentityViewProjection(identity);
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

// Reconstructed from eboot.elf at 0x3E16B0. The outline is one closed
// line strip; the corners alternate between the direct conversion to clip
// space and the one through pixels.
void RndDrawUtl::DrawQuadWireframe2D(
    RndContext& context,
    const Hmx::Rect& rect,
    const Line2DParams& params) {
    const bool identity = context.mUsingIdentityViewProjection;
    const RndShadingMode shadingMode = BeginLines2D(context, params);
    const Vector2 viewportSize = context.mViewportSize;
    const CoordinateMode mode = params.mCoordinateMode;
    const float right = rect.x + rect.w;
    const float bottom = rect.y + rect.h;
    RndVertexColor vertices[5];
    SetLineVertex(vertices[0], ToClip(mode, viewportSize, {rect.x, rect.y}), params.mColor);
    SetLineVertex(
        vertices[1],
        PixelsToClip(viewportSize, ToPixels(mode, viewportSize, {right, rect.y})),
        params.mColor);
    SetLineVertex(vertices[2], ToClip(mode, viewportSize, {right, bottom}), params.mColor);
    SetLineVertex(
        vertices[3],
        PixelsToClip(viewportSize, ToPixels(mode, viewportSize, {rect.x, bottom})),
        params.mColor);
    vertices[4] = vertices[0];
    context._DrawPrimitivesImpl(RndPrimitive::kLineStrip, RndVertexColor::kType, vertices, 5);
    context.SetShadingMode(shadingMode);
    context.SetUsingIdentityViewProjection(identity);
}

// Reconstructed from eboot.elf at 0x3E49C0. The wide copy lives on the
// stack.
void RndDrawUtl::DrawText2D(
    RndContext& context,
    const char* text,
    const Vector2& position,
    Text2DParams& params,
    Hmx::Rect* bounds,
    Vector2* end) {
    const unsigned long length = std::strlen(text);
    auto* buffer = static_cast<unsigned short*>(
        __builtin_alloca((length + 1) * sizeof(unsigned short)));
    const unsigned short* wide = CharToWideChar(text, buffer, length + 1);
    const Vector2 viewportSize = context.mViewportSize;
    DrawText2D(&context, wide, position, viewportSize, params, bounds, end);
}

// Reconstructed from eboot.elf at 0x3E4A70. The text is laid out in font
// pixels, scaled and placed at the position's pixels; the bounds and end
// are converted back to the coordinate mode. Without a context nothing is
// drawn. Each style's font draws its glyphs page by page, as two triangles
// a glyph, after eight copies offset by one pixel around them in the
// shadow color. A rotation within 1e-4 of zero is ignored, and the
// rotation's sines and cosines snap to whole numbers within 1e-4.
void RndDrawUtl::DrawText2D(
    RndContext* context,
    const unsigned short* text,
    const Vector2& position,
    const Vector2& viewportSize,
    Text2DParams& params,
    Hmx::Rect* bounds,
    Vector2* end) {
    const CoordinateMode mode = params.mCoordinateMode;
    const Vector2 origin = ToPixels(mode, viewportSize, position);

    RndTypesetter::Params layout = {};
    layout.mText = text;
    layout.mAlignment = params.mAlignment;
    layout.mJustification = params.mJustification;
    layout.mFitMode = params.mFitMode;
    layout.mStyleSize = params.mStyleSize;
    layout.mMarkup = params.mNumStyles != 0;
    layout.mToken = Symbol();
    if (params.mFitMode != kTextFitModeNone) {
        const float left = ToPixelsX(mode, viewportSize, Vector2::sZero.x);
        const float right = ToPixelsX(mode, viewportSize, params.mWrapWidth);
        layout.mMaxWidth = RoundToInt((right - left) / params.mScale);
        if (params.mFitMode == kTextFitModeWrapAndShrink) {
            const float top = ToPixelsY(mode, viewportSize, Vector2::sZero.y);
            const float bottom = ToPixelsY(mode, viewportSize, params.mWrapHeight);
            layout.mMaxHeight = RoundToInt((bottom - top) / params.mScale);
        }
    }
    RndTypesetter::Style style;
    style.mAlignment = kTextAlignBottom;
    if (params.mNumStyles != 0) {
        layout.mStyles = params.mStyles;
        layout.mNumStyles = params.mNumStyles;
    } else {
        RndFont* font = params.mFont;
        if (font == nullptr) {
            font = RndDebugFont::GetInstance();
        }
        style.mFonts.push_back(RndTypesetter::StyleFont{font, 0});
        layout.mStyles = &style;
        layout.mNumStyles = 1;
    }

    const unsigned long capacity = RndTypesetter::CalcResultGlyphsCapacity(
        RndTypesetter::CalcNumGlyphs(text, layout.mMarkup), 0, layout.mFitMode);
    RndTypesetter::Result result = {};
    result.mStyleSize = static_cast<RndFontStyleSize>(-1);
    if (capacity != 0) {
        result.mGlyphs.mData = static_cast<RndTypesetter::Glyph*>(
            __builtin_alloca(capacity * sizeof(RndTypesetter::Glyph)));
    }
    result.mGlyphs.mCapacity = capacity;
    RndTypesetter::ProcessText(layout, result);

    const float scale = params.mScale;
    if (bounds != nullptr) {
        const Vector2 min = FromPixels(
            mode,
            viewportSize,
            {static_cast<float>(result.mMin.x) * scale + origin.x,
             static_cast<float>(result.mMin.y) * scale + origin.y});
        const Vector2 max = FromPixels(
            mode,
            viewportSize,
            {static_cast<float>(result.mMax.x) * scale + origin.x,
             static_cast<float>(result.mMax.y) * scale + origin.y});
        bounds->x = min.x;
        bounds->y = min.y;
        bounds->w = max.x - min.x;
        bounds->h = max.y - min.y;
    }
    if (end != nullptr) {
        *end = FromPixels(
            mode,
            viewportSize,
            {static_cast<float>(result.mEnd.x) * scale + origin.x,
             static_cast<float>(result.mEnd.y) * scale + origin.y});
    }
    if (context == nullptr || result.mGlyphs.size() == 0) {
        return;
    }

    float rotation[4] = {1.0F, 0.0F, 0.0F, 1.0F};
    const bool rotated = __builtin_fabsf(params.mRotation) > kRotationEpsilon;
    if (rotated) {
        const float sine = Sine(params.mRotation);
        const float cosine = Sine(params.mRotation + kHalfPi);
        rotation[0] = cosine;
        rotation[1] = sine;
        rotation[2] = -sine;
        rotation[3] = cosine;
        for (float& value : rotation) {
            const float rounded = static_cast<float>(RoundToInt(value));
            if (!(__builtin_fabsf(rounded - value) > kRotationEpsilon)) {
                value = rounded;
            }
        }
    }

    const bool identity = context->mUsingIdentityViewProjection;
    context->SetUsingIdentityViewProjection(true);
    const unsigned long maxVertices = result.mGlyphs.size() * 6;
    const unsigned long maxShadowVertices = params.mShadow ? maxVertices * 8 : 0;
    RndVertexColorTex* vertices = nullptr;
    if (maxVertices != 0) {
        vertices = static_cast<RndVertexColorTex*>(
            __builtin_alloca(maxVertices * sizeof(RndVertexColorTex)));
    }
    RndVertexColorTex* shadowVertices = nullptr;
    if (maxShadowVertices != 0) {
        shadowVertices = static_cast<RndVertexColorTex*>(
            __builtin_alloca(maxShadowVertices * sizeof(RndVertexColorTex)));
    }
    context->mBlendMode = params.mBlendMode;
    context->_SetBlendModeImpl(params.mBlendMode, Hmx::Color::GetWhite());
    context->_SetDepthModeImpl(0);
    context->_SetCullModeImpl(kCullNone);

    const Vector2i& resolution = TheRndDevice()->mSettings->mOutputResolution;
    const Vector2 toClip = {2.0F / viewportSize.x, 2.0F / viewportSize.y};
    for (unsigned long styleIndex = 0; styleIndex < layout.mNumStyles; ++styleIndex) {
        RndFont* font = layout.mStyles[styleIndex].mFonts[result.mStyleSize].mFont;
        const RndFont::Size* size = font->GetSize(resolution);
        for (unsigned long page = 0; page < size->mNumPages; ++page) {
            unsigned long numVertices = 0;
            for (unsigned long i = 0; i < result.mGlyphs.size(); ++i) {
                const RndTypesetter::Glyph& glyph = result.mGlyphs[i];
                if (glyph.mStyle != styleIndex || glyph.mPage != page) {
                    continue;
                }
                const float left = static_cast<float>(glyph.mMin.x) * scale;
                const float bottom = static_cast<float>(glyph.mMin.y) * scale;
                const float right = static_cast<float>(glyph.mMax.x) * scale;
                const float top = static_cast<float>(glyph.mMax.y) * scale;
                Vector2 corners[4];  // Min, (max x, min y), (min x, max y), max.
                if (rotated) {
                    corners[0] = {left * rotation[0] + bottom * rotation[2],
                                  left * rotation[1] + bottom * rotation[3]};
                    corners[1] = {right * rotation[0] + bottom * rotation[2],
                                  right * rotation[1] + bottom * rotation[3]};
                    corners[2] = {left * rotation[0] + top * rotation[2],
                                  left * rotation[1] + top * rotation[3]};
                    corners[3] = {right * rotation[0] + top * rotation[2],
                                  right * rotation[1] + top * rotation[3]};
                } else {
                    corners[0] = {left, bottom};
                    corners[1] = {right, bottom};
                    corners[2] = {left, top};
                    corners[3] = {right, top};
                }

                RndVertexColorTex* quad = &vertices[numVertices];
                for (unsigned long corner = 0; corner < 4; ++corner) {
                    RndVertexColorTex& vertex = quad[corner];
                    vertex.mPos[0] = (corners[corner].x + origin.x) * toClip.x + -1.0F;
                    vertex.mPos[1] = (corners[corner].y + origin.y) * toClip.y + -1.0F;
                    vertex.mPos[2] = 0.0F;
                    vertex.mColor[0] = params.mColor.red;
                    vertex.mColor[1] = params.mColor.green;
                    vertex.mColor[2] = params.mColor.blue;
                    vertex.mColor[3] = params.mColor.alpha;
                }
                const float u = glyph.mUV[0];
                const float v = glyph.mUV[1];
                quad[0].mTex[0] = u;
                quad[0].mTex[1] = v + glyph.mUV[3];
                quad[1].mTex[0] = u + glyph.mUV[2];
                quad[1].mTex[1] = v + glyph.mUV[3];
                quad[2].mTex[0] = u;
                quad[2].mTex[1] = v;
                quad[3].mTex[0] = u + glyph.mUV[2];
                quad[3].mTex[1] = v;
                // The second triangle reuses two corners of the first.
                quad[4] = quad[2];
                quad[5] = quad[1];
                numVertices += 6;
            }
            if (numVertices == 0) {
                continue;
            }

            const RndShadingMode shadingMode = context->mShadingMode;
            context->SetShadingMode(kShadingModeStandard);
            RndShaderBasic::Params basic;
            basic.mAlphaCut = true;
            basic.mUseTexRedAsAlpha = true;
            basic.mTexture = size->mPages[page].mTexture;
            TheRndDevice()->mShaderMgr.mBasicShader->Select(*context, basic);
            if (params.mShadow) {
                const float dx = (Vector2::sUnitAll.x - Vector2::sZero.x) * 2.0F / viewportSize.x;
                const float dy = (Vector2::sUnitAll.y - Vector2::sZero.y) * 2.0F / viewportSize.y;
                unsigned long block = 0;
                for (int row = -1; row <= 1; ++row) {
                    const float offsetY = static_cast<float>(row) * dy;
                    for (int column = -1; column <= 1; ++column) {
                        if (row == 0 && column == 0) {
                            continue;
                        }
                        RndVertexColorTex* copy = &shadowVertices[block * numVertices];
                        for (unsigned long i = 0; i < numVertices; ++i) {
                            copy[i] = vertices[i];
                            copy[i].mPos[0] += static_cast<float>(column) * dx;
                            copy[i].mPos[1] += offsetY;
                            copy[i].mColor[0] = params.mShadowColor.red;
                            copy[i].mColor[1] = params.mShadowColor.green;
                            copy[i].mColor[2] = params.mShadowColor.blue;
                            copy[i].mColor[3] = params.mShadowColor.alpha;
                        }
                        ++block;
                    }
                }
                context->_DrawPrimitivesImpl(
                    RndPrimitive::kTriangles,
                    RndVertexColorTex::kType,
                    shadowVertices,
                    numVertices * 8);
            }
            context->_DrawPrimitivesImpl(
                RndPrimitive::kTriangles, RndVertexColorTex::kType, vertices, numVertices);
            context->SetShadingMode(shadingMode);
        }
    }
    context->SetUsingIdentityViewProjection(identity);
}

// Reconstructed from eboot.elf at 0x3E5F60. The wide copy lives on the
// stack.
void RndDrawUtl::MeasureText2D(
    const char* text,
    const Vector2& position,
    const Vector2& viewportSize,
    Text2DParams& params,
    Hmx::Rect* bounds,
    Vector2* end) {
    const unsigned long length = std::strlen(text);
    auto* buffer = static_cast<unsigned short*>(
        __builtin_alloca((length + 1) * sizeof(unsigned short)));
    const unsigned short* wide = CharToWideChar(text, buffer, length + 1);
    DrawText2D(nullptr, wide, position, viewportSize, params, bounds, end);
}
