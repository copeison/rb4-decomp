#pragma once

#include "math/color/Color.h"
#include "math/geometry/Rect.h"
#include "render/system/render_runtime_adapters.h"

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

    // Draws the quad with an identity view-projection. A quad covering the
    // whole target is drawn as one oversized triangle.
    static void DrawQuad2D(RndContext& context, Quad2DParams& params);  // 0x3E0C50
};
