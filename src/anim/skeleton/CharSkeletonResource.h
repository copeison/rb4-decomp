#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class CharSkeleton;

// A skeleton file, extension "skel" (anim/CharSkeletonResource.o,
// 0x2FB6D0-0x2FBBFB): the CharSkeleton and the FBX file it came from. The
// skeleton exists only in the cached file; reading the source fails. The
// vtable at 0x18F6F38 has 13 slots. The object is 64 bytes.
class CharSkeletonResource : public Resource {
public:
    CharSkeletonResource();  // 0x2FB6D0
    // Slots 10-11: 0x2FB7A0, 0x2FB7F0.
    ~CharSkeletonResource() override;

    ResourceMetaData* GetMetaData() const override;    // slot 0: 0x2FBAD0
    Symbol GetId() const override;                     // slot 1: 0x2FBAE0
    bool IsA(Symbol type) const override;              // slot 2: 0x2FBB80
    bool Load(BinStream& stream, bool cached) override;  // slot 4: 0x2FB960
    void Save(BinStream& stream, bool cached) override;  // slot 5: 0x2FBA50
    // Slot 6: without a skeleton.
    bool Fail() const override;  // 0x2FBBB0

    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19F7CA8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharSkeletonResource");
        }
        return id;
    }

    // Registers the class as a resource. The binary emits it in
    // anim/AnimInit.o at 0x2ADFF0, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B5E60.
    static Resource* _Create();

    // Sets the FBX path and replaces the skeleton with an empty one.
    void SetFBXPath(const char* path);  // 0x2FB750
    static void _Init(ResourceMetaData& metaData);  // 0x2FB840

    // Constructed by the static initializer at 0x2FBBC0, which first sets a
    // shared header's null object id to -1 (0x19F9678).
    static ResourceMetaData sMetaData;  // 0x19F9680

    // Field names are not in the reference map.
    CharSkeleton* mSkeleton;
    ResourcePath mFBXPath;
};

static_assert(offsetof(CharSkeletonResource, mSkeleton) == 48);
static_assert(offsetof(CharSkeletonResource, mFBXPath) == 56);
static_assert(sizeof(CharSkeletonResource) == 64);
