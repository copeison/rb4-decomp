#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "math/geometry/Rect.h"
#include "math/geometry/Segment.h"
#include "math/vector/Vector2.h"
#include "render/fonts/RndTypesetter.h"
#include "render/materials/RndMaterialCom.h"
#include "utl/containers/VectorAdapter.h"

class RndContext;
class RndFont;
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
        bool mThickLines = false;
    };

    // Debug text: where it is placed, its color and shadow, its fonts, and
    // how it is laid out. Field names are not in the reference map.
    struct Text2DParams {
        CoordinateMode mCoordinateMode = kCoordinateNormalized;
        Hmx::Color mColor = Hmx::Color::GetWhite();
        // Draws the text over a shadow one pixel out in every direction.
        bool mShadow = false;
        Hmx::Color mShadowColor = Hmx::Color::GetBlack();
        RndBlendMode mBlendMode = RndBlendMode::kSource;
        unsigned int mUnknown44;  // Not set by the constructor.
        // The font, or the debug font, unless styles are given.
        RndFont* mFont = nullptr;
        const RndTypesetter::Style* mStyles = nullptr;
        unsigned long mNumStyles = 0;  // Styles also enable markup.
        float mScale = 1.0F;     // Pixels per font pixel.
        float mRotation = 0.0F;  // Radians.
        // Where the text sits against the position, vertically and
        // horizontally.
        RndTextAlignment mAlignment = kTextAlignTop;
        RndTextJustification mJustification = kTextJustifyLeft;
        // How the text fits mWrapWidth, and for wrap and shrink also
        // mWrapHeight. Both are in the coordinate mode's units.
        RndTextFitMode mFitMode = kTextFitModeNone;
        float mWrapWidth = 0.0F;
        float mWrapHeight = 0.0F;
        RndFontStyleSize mStyleSize = kFontStyleSizeRegular;  // The first size tried.
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
    // and the basic shader, without depth testing or culling. The start
    // points are converted to clip space by mode; the end points are first
    // brought to pixels.
    static void DrawLines2D(
        RndContext& context,
        const VectorAdapter<Segment2D>& segments,
        const Line2DParams& params);  // 0x3DFCB0
    // Draws the rectangle's outline.
    static void DrawQuadWireframe2D(
        RndContext& context,
        const Hmx::Rect& rect,
        const Line2DParams& params);  // 0x3E16B0
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
    // Lays the text out in a viewport of `viewportSize` and, given a
    // context, draws it page by page with the basic shader, its shadow
    // first. The map's signature is DrawText2D(RndContext&, unsigned short
    // const*, Vector2 const&, RndDrawUtl::Text2DParams&, Hmx::Rect*,
    // Vector2*); this build takes the viewport size and a context that may
    // be null.
    static void DrawText2D(
        RndContext* context,
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

static_assert(offsetof(RndDrawUtl::Line2DParams, mBlendMode) == 20);
static_assert(offsetof(RndDrawUtl::Line2DParams, mThickLines) == 24);
static_assert(sizeof(RndDrawUtl::Line2DParams) == 28);
static_assert(offsetof(RndDrawUtl::Text2DParams, mShadowColor) == 24);
static_assert(offsetof(RndDrawUtl::Text2DParams, mBlendMode) == 40);
static_assert(offsetof(RndDrawUtl::Text2DParams, mFont) == 48);
static_assert(offsetof(RndDrawUtl::Text2DParams, mStyles) == 56);
static_assert(offsetof(RndDrawUtl::Text2DParams, mNumStyles) == 64);
static_assert(offsetof(RndDrawUtl::Text2DParams, mScale) == 72);
static_assert(offsetof(RndDrawUtl::Text2DParams, mRotation) == 76);
static_assert(offsetof(RndDrawUtl::Text2DParams, mAlignment) == 80);
static_assert(offsetof(RndDrawUtl::Text2DParams, mJustification) == 84);
static_assert(offsetof(RndDrawUtl::Text2DParams, mFitMode) == 88);
static_assert(offsetof(RndDrawUtl::Text2DParams, mWrapWidth) == 92);
static_assert(offsetof(RndDrawUtl::Text2DParams, mWrapHeight) == 96);
static_assert(offsetof(RndDrawUtl::Text2DParams, mStyleSize) == 100);
static_assert(sizeof(RndDrawUtl::Text2DParams) == 104);
