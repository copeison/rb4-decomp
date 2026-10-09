#pragma once

#include "utl/containers/VectorAdapter.h"

class RndContext;
class RndShader;
enum Rnd2DCoord : int;

namespace Hmx {
class Rect;
}

// Buffer inspection views, in the order of ToString's name table; the first
// is "None". Enumerator names are not in the reference map.
enum RndBufferInspectionMode : unsigned int {
    kBufferInspectionNone = 0,
};

// Debug display of the renderer's intermediate buffers, either one buffer
// full screen or a layout of thumbnails. Only the mode names are
// reconstructed; the drawing methods are declared with the map's signatures.
class RndBufferInspection {
public:
    // Name not in the reference map.
    static constexpr unsigned int kNumModes = 74;

    // One thumbnail of a layout. The layout is not recovered.
    struct ThumbnailLayoutEntry;

    // Creates and registers the inspection shader.
    static void Init();       // 0x6B54A0
    static void Terminate();  // 0x6B54E0
    static RndShader* GetShader();
    static void Draw(RndContext& context, RndBufferInspectionMode mode);

    static const char* ToString(RndBufferInspectionMode mode);  // 0x6B4EE0
    // Matches the names ignoring ASCII case and returns -1 when none does.
    static RndBufferInspectionMode FromString(const char* name);  // 0x6B5510

private:
    static void _DrawOneBuffer(
        RndContext& context,
        RndBufferInspectionMode mode,
        unsigned long index,
        const Hmx::Rect& rect,
        Rnd2DCoord coord);
    static void _DrawThumbnails(
        RndContext& context,
        RndBufferInspectionMode first,
        RndBufferInspectionMode last);
    static void _DrawThumbnailLayout(
        RndContext& context,
        VectorAdapter<ThumbnailLayoutEntry> entries);
};
