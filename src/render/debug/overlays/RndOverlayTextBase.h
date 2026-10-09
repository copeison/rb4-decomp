#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "render/debug/RndOverlay.h"
#include "utl/text/TextStream.h"

// An overlay that prints text. Draw renders what _Print writes upwards from
// the given line, one background band per line, and closes it with a band
// of the vertical spacing. The vtable is at 0x19398A0.
class RndOverlayTextBase : public RndOverlay {
public:
    // The stream _Print writes to. It collects one line at a time and
    // draws it when the line ends or the buffer fills. The vtable is at
    // 0x1939908.
    class OverlayTextStream : public TextStream {
    public:
        OverlayTextStream(RndOverlayTextBase* overlay, RndContext& context, int y);  // 0x6E5DB0
        // Slots 0-1 at 0x6E5D80 and 0x6E6240.
        ~OverlayTextStream() override {}

        void Print(const char* str) override;  // slot 2: 0x6E5E10

        // Draws the buffered line above the current one and empties the
        // buffer.
        void Flush();  // 0x6E5EA0

        // Field names are not in the reference map.
        RndOverlayTextBase* mOverlay;
        RndContext* mContext;
        int mX;  // The left margin.
        int mY;  // The bottom of the next line; lines are drawn upwards.
        char mBuffer[1024];
        char* mCursor;
        unsigned long mLineCount;
    };

    // The map's signature is RndOverlayTextBase(char const*, char const*,
    // unsigned int); this build has no second string.
    RndOverlayTextBase(const char* name, unsigned int flags);  // 0x6E5B00
    // Slots 0-1: 0x6E5B30, 0x6E5B40.
    ~RndOverlayTextBase() override;

    int Draw(RndContext& context, int y) override;  // slot 2: 0x6E5B60

    // Slot 8: writes the overlay's text.
    virtual void _Print(TextStream& stream) = 0;
    // Slot 9 at 0x6D1720, emitted among RndOverlay's inline defaults.
    virtual Hmx::Color _GetTextColor() const {
        return Hmx::Color::GetWhite();
    }
    // Slot 10 at 0x6E5D90: a translucent grey.
    virtual Hmx::Color _GetBackgroundColor() const;
};

static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mOverlay) == 8);
static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mContext) == 16);
static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mX) == 24);
static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mY) == 28);
static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mBuffer) == 32);
static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mCursor) == 1056);
static_assert(offsetof(RndOverlayTextBase::OverlayTextStream, mLineCount) == 1064);
static_assert(sizeof(RndOverlayTextBase::OverlayTextStream) == 1072);
static_assert(sizeof(RndOverlayTextBase) == 64);
