#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "math/geometry/Rect.h"
#include "math/geometry/Segment.h"
#include "math/vector/Vector2.h"
#include "render/materials/RndMaterialCom.h"
#include "utl/containers/VectorAdapter.h"

class RndContext;
class RndTextureBase;

// Immediate-mode drawing of debug and screen-space geometry.
class RndDrawUtl {
public:
    // The space a quad's rectangle is given in. Names not in the reference
    // map.
    enum CoordinateMode : int {
        kCoordinatePixels = 0,
        kCoordinateNormalized = 1,
        kCoordinateAspectCorrected = 2,  // Normalized height, centered.
        kCoordinateClip = 3,
    };

    // A 2D quad: where it is, its color and depth, and the state and shader
    // to draw it with. Field names are not in the reference map.
    struct Quad2DParams {
        CoordinateMode mCoordinateMode = kCoordinateNormalized;
        Hmx::Rect mRect = Hmx::Rect(0.0F, 0.0F, 1.0F, 1.0F);
        Hmx::Color mColor = Hmx::Color::GetWhite();
        RndBlendMode mBlendMode = RndBlendMode::kSource;
        unsigned int mDepthMode = 0;
        float mZ = 0.0F;
        bool mAlphaCut = false;
        RndTextureBase* mTexture = nullptr;
        RndTextureBase* mRTSlicedTexture = nullptr;
        bool mKeepShader = false;  // Draws with the selected shader.
        bool mKeepState = false;   // Keeps the blend, depth and cull state.
    };

    // A 2D line: its space, color and blend mode. Field names are not in
    // the reference map.
    struct Line2DParams {
        CoordinateMode mCoordinateMode = kCoordinateNormalized;
        Hmx::Color mColor = Hmx::Color::GetWhite();
        RndBlendMode mBlendMode = RndBlendMode::kSource;
        bool mUnknown24 = false;
    };

    // Debug text: where it is placed, its color and shadow, and how it
    // wraps. Only the fields the reconstructed callers set are named; the
    // others are named after their offsets. Field names are not in the
    // reference map.
    struct Text2DParams {
        CoordinateMode mCoordinateMode = kCoordinateNormalized;
        Hmx::Color mColor = Hmx::Color::GetWhite();
        bool mUnknown20 = false;
        Hmx::Color mShadowColor = Hmx::Color::GetBlack();
        RndBlendMode mBlendMode = RndBlendMode::kSource;
        unsigned int mUnknown44;  // Not set by the constructor.
        unsigned char mUnknown48[16] = {};
        void* mUnknown64 = nullptr;
        float mScale = 1.0F;
        unsigned int mUnknown76 = 0;
        unsigned long mUnknown80 = 0;
        // Nonzero wraps the text at mWrapWidth pixels.
        int mWrapMode = 0;
        float mWrapWidth = 0.0F;
        unsigned int mUnknown96 = 0;
        int mUnknown100 = 0;
    };

    // Builds the shared sphere, box and other meshes the drawing helpers use.
    static void Init();  // 0x3DF170
    static void Terminate();  // 0x3DF820

    // Draws the quad with an identity view-projection. A quad covering the
    // whole target is drawn as one oversized triangle.
    static void DrawQuad2D(RndContext& context, Quad2DParams& params);  // 0x3E0C50
    // Draws the segment through DrawLines2D.
    static void DrawLine2D(
        RndContext& context,
        const Segment2D& segment,
        const Line2DParams& params);  // 0x3DFC50
    // Draws the segments as one line list with an identity view-projection
    // and the basic shader.
    static void DrawLines2D(
        RndContext& context,
        const VectorAdapter<Segment2D>& segments,
        const Line2DParams& params);  // 0x3DFCB0
    // Draws the text at `position` within the context's viewport. `bounds`
    // and `end` receive the text's rectangle and the position after it
    // when given. Widens the text and draws it through the wide overload.
    static void DrawText2D(
        RndContext& context,
        const char* text,
        const Vector2& position,
        Text2DParams& params,
        Hmx::Rect* bounds,
        Vector2* end);  // 0x3E49C0
    // The wide overload lays the text out in `viewportSize`; the map's
    // signature is DrawText2D(RndContext&, unsigned short const*, Vector2
    // const&, RndDrawUtl::Text2DParams&, Hmx::Rect*, Vector2*).
    static void DrawText2D(
        RndContext& context,
        const unsigned short* text,
        const Vector2& position,
        const Vector2& viewportSize,
        Text2DParams& params,
        Hmx::Rect* bounds,
        Vector2* end);  // 0x3E4A70
    // Lays the text out as DrawText2D would in a viewport of
    // `viewportSize`, without drawing it. Name not in the reference map.
    static void MeasureText2D(
        const char* text,
        const Vector2& position,
        const Vector2& viewportSize,
        Text2DParams& params,
        Hmx::Rect* bounds,
        Vector2* end);  // 0x3E5F60
};

static_assert(sizeof(RndDrawUtl::Line2DParams) == 28);
static_assert(offsetof(RndDrawUtl::Text2DParams, mShadowColor) == 24);
static_assert(offsetof(RndDrawUtl::Text2DParams, mBlendMode) == 40);
static_assert(offsetof(RndDrawUtl::Text2DParams, mUnknown64) == 64);
static_assert(offsetof(RndDrawUtl::Text2DParams, mScale) == 72);
static_assert(offsetof(RndDrawUtl::Text2DParams, mWrapMode) == 88);
static_assert(offsetof(RndDrawUtl::Text2DParams, mWrapWidth) == 92);
static_assert(sizeof(RndDrawUtl::Text2DParams) == 104);
