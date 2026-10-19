#pragma once

#include "utl/containers/List.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class Resource;
class ResourcePath;

// The downloadable content manager (os/ContentMgr.o, os/ContentMgr_PS4.o).
// Only the members the reconstructed code calls are declared; the class is
// not reconstructed. The PS4 vtable is at 0x18FB618.
class ContentMgr : public MsgSink {
public:
    // A receiver of the content events, registered with RegisterCallback.
    // The typeinfo is emitted with its users (rb_meta/SongMgr.o). Only
    // ContentMountBegun and ContentUnmounted have defaults the users emit
    // (0xAE6560 and 0xAE6570 in this build); the other slots are declared
    // pure because no default was recovered. The signatures follow this
    // build's SongMgr overrides; the map's build differs where noted.
    class Callback {
    public:
        virtual ~Callback() {}  // slots 0-1
        // Slot 2. The map's signature is ContentRefreshStarted().
        virtual void ContentRefreshStarted(int refreshType) = 0;
        // Slot 3: whether to mount the discovered content. The map's
        // signature is ContentDiscovered(Symbol, bool&).
        virtual void ContentDiscovered(int location, Symbol content, bool& mount) = 0;
        // Slot 4.
        virtual void ContentMountBegun(unsigned long count) {
            static_cast<void>(count);
        }
        // Slot 5.
        virtual void ContentMounted(Symbol content, const char* root) = 0;
        // Slot 6.
        virtual void ContentUnmounted(Symbol content) {
            static_cast<void>(content);
        }
        // Slot 7.
        virtual void ContentFailed(Symbol content) = 0;
        // Slot 8: the path a found file is loaded from.
        virtual const char* ContentFileFound(Symbol content, const char* file) = 0;
        // Slot 9.
        virtual void ContentLoaded(Symbol content, Resource* resource) = 0;
        // Slot 10. The map's signature is ContentRefreshDone().
        virtual void ContentRefreshDone(int refreshType) = 0;
        // Slot 11: the pattern of the files to look for.
        virtual const char* ContentPattern() = 0;
        // Slot 12: the folders to look in.
        virtual eastl::list<String> ContentDirs() = 0;
    };

    ~ContentMgr() override;  // slots 0-1
    DataNode Handle(DataArray* msg, bool warn) override;  // slot 2
    virtual void Init();       // slot 4: 0x36C370
    virtual void Poll();       // slot 5: 0x36C870
    virtual void Terminate();  // slot 6: 0x36C810

    // Whether the cheat that fails every content mount is on. The binary
    // inlines it as a read of the flag at +0x30 (StorageRef_PS4's mount at
    // 0x3B1CF7); the flag is not modelled.
    bool AreAllMountsGoingToFailDueToCheat();

    // True while a refresh is under way (states 2-4 and above 5). The
    // evidence for the name is weak.
    bool IsRefreshInProgress() const;  // 0x393010
    // Asks for the content to be mounted, holding a mount reference. False
    // when it cannot be mounted.
    bool MountContent(Symbol content);  // 0x3949B0
    // The evidence for the name is weak.
    bool IsContentMounted(Symbol content) const;  // 0x394B50
    void ReleaseMountRef(Symbol content);  // 0x394CE0
    // The flag is unused in this build. The map's signature is
    // IsContentAvailable(Symbol) const.
    bool IsContentAvailable(Symbol content, bool unused) const;  // 0x394D70
    // The root folder of the content.
    const char* ContentRoot(Symbol content) const;  // 0x394EB0
    void RegisterCallback(Callback* callback, bool notify);    // 0x394F50
    void UnregisterCallback(Callback* callback, bool notify);  // 0x394FC0
};

// The PS4 content manager, a ContentMgr_PS4 at 0x19FE7A0.
extern ContentMgr TheContentMgr;

// Replaces the function that turns a found content file into the path of
// the resource it is loaded as; null restores the default (0x3926B0), which
// keeps the path. SongMgr::Init installs its own. Name not in the reference
// map.
void SetContentFilePathFunc(ResourcePath (*func)(const char* file));  // 0x392690
