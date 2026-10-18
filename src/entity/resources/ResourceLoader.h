#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/threading/ThreadCall.h"

// Loads a resource on the "Hmx ThreadCall" thread (the map's ResourceLoader
// in entity/Resource.o). A loader is acquired through AcquireLoader, which
// keeps it in a registry, and released through ReleaseLoader, which defers
// the deletion of a loader whose call is still running. The registry
// functions sit in the second engine block (0x1167F10 on). The inline
// members below are emitted in ui/UIMgr.o, their only user in this build.
// The vtable at 0x195E548 has four slots; the object is 32 bytes. Field
// names are not in the reference map.
class ResourceLoader : public ThreadCallback {
public:
    // Inlined into AcquireLoader (0x1167F10) and SceneLoader's constructor.
    explicit ResourceLoader(ResourcePath path)
        : mDone(false), mStarted(false), mPath(path), mResource() {}
    // Slots 0-1 at 0x8CC410 and 0x8CC450.
    ~ResourceLoader() override {}

    // Slot 2 at 0x8CC310: loads the resource on the call thread.
    int ThreadStart() override {
        mResource = Resource::GetOrLoad(mPath, Resource::Id(), false);
        return 0;
    }
    // Slot 3 at 0x8CAFD0.
    void ThreadDone(int result) override {
        static_cast<void>(result);
        mDone = true;
    }

    // Emitted at 0x8C9700. Starts the load once: a loaded resource that
    // has not changed on disk and needs no reload is taken at once, and any
    // other is loaded through ThreadCall. A loader without a path is done.
    void Start() {
        if (mStarted) {
            return;
        }
        mStarted = true;
        if (mPath.mPath == Symbol()) {
            mDone = true;
            return;
        }
        mResource = Resource::Get(mPath);
        if (mResource != nullptr && !mResource->mFileChangedOnDisk &&
            !mResource->NeedsReload()) {
            mDone = true;
            return;
        }
        mResource = nullptr;
        ThreadCall(this);
    }
    // Emitted at 0x8C9A40; no caller in this build. Name not in the
    // reference map.
    ResourcePtr<Resource> GetResource() const {
        return ResourcePtr<Resource>(mResource.Get());
    }

    // Creates a loader for the path, registers it and, when `start` is
    // set, starts it.
    static ResourceLoader* AcquireLoader(ResourcePath path, const bool& start);  // 0x1167F10
    // Clears the pointer and unregisters the loader. A loader whose call
    // is still running is marked and deleted once the call is done (Poll);
    // any other is deleted at once.
    static void ReleaseLoader(ResourceLoader*& loader);  // 0x1168010

    // Set by ThreadDone, or by Start when no load is needed.
    bool mDone;
    // Set by the first Start.
    bool mStarted;
    ResourcePath mPath;
    ResourcePtr<Resource> mResource;
};

static_assert(offsetof(ResourceLoader, mDone) == 8);
static_assert(offsetof(ResourceLoader, mStarted) == 9);
static_assert(offsetof(ResourceLoader, mPath) == 16);
static_assert(offsetof(ResourceLoader, mResource) == 24);
static_assert(sizeof(ResourceLoader) == 32);
