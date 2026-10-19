#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class CharAnimRaw;

// The raw samples of a character animation, extension "charanimraw"
// (anim/CharAnimRawResource.o, 0x2DF930-0x2DFE8F): the CharAnimRaw a
// CharAnimResource is built from. Only revision 10 of the cached file holds
// one; reading the source file fails. The vtable at 0x18F5260 has 13
// slots. The object is 56 bytes.
class CharAnimRawResource : public Resource {
public:
    CharAnimRawResource();  // 0x2DF930
    // Slots 10-11: 0x2DF9A0, 0x2DF9F0.
    ~CharAnimRawResource() override;

    ResourceMetaData* GetMetaData() const override;    // slot 0: 0x2DFCC0
    Symbol GetId() const override;                     // slot 1: 0x2DFCD0
    bool IsA(Symbol type) const override;              // slot 2: 0x2DFD70
    bool Load(BinStream& stream, bool cached) override;  // slot 4: 0x2DFB60
    // Slot 5: writes the animation to a cached file only.
    void Save(BinStream& stream, bool cached) override;  // 0x2DFC50
    // Slot 6: without an animation.
    bool Fail() const override;  // 0x2DFDA0

    // The class id, created on first use and inlined into its users, such
    // as CharAnimCom::_Init (0x2D8350). The local static is at 0x19F7C98.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharAnimRawResource");
        }
        return id;
    }

    // Registers the class as a resource. The binary emits it in
    // anim/AnimInit.o at 0x2ADE80, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B5E30.
    static Resource* _Create();
    // Names the extension ("charanimraw") and the category ("Animations").
    static void _Init(ResourceMetaData& metaData);  // 0x2DFA40

    // Constructed by the static initializer at 0x2DFDB0, which first sets a
    // shared header's null object id to -1 (0x19F8680).
    static ResourceMetaData sMetaData;  // 0x19F8688

    // Field name not in the reference map.
    CharAnimRaw* mRaw;
};

static_assert(offsetof(CharAnimRawResource, mRaw) == 48);
static_assert(sizeof(CharAnimRawResource) == 56);
