#pragma once

#include <cstddef>

#include "utl/text/TextStream.h"

// The engine's debug channel (os/Debug.o): a TextStream that logs what is
// printed to it, plus the notify, warn and fail services. Only the stream
// interface is declared; the members and the services have not been
// reconstructed. The vtable is at 0x18FAF08.
class Debug : public TextStream {
public:
    // Inlined into the static initializer at 0x35CF70 in this build.
    Debug();
    ~Debug() override;                    // slots 0-1: 0x35BB60, 0x35BC00
    void Print(const char* str) override;  // slot 2: 0x35CA80

    // Appends the callback to the list run at exit. Inlined into its users,
    // for example Rnd::Init at 0x402CE0, where it pushes onto the list at
    // TheDebug + 0x48; the list is not modelled. Name not in the reference
    // map, which has RemoveExitCallback(void (*)()).
    void AddExitCallback(void (*callback)());

    // Reports a failure. This build's release body only runs the fail
    // callbacks on the "failure" heap, guarded against reentry.
    void Fail(const char* msg);  // 0x35C4C0

    // The byte at +8 is not read by the reconstructed code. The map's
    // SetDisabled(bool) suggests a disabled flag; this build has no such
    // setter, so the name is weakly supported. Name not in the reference
    // map.
    bool mDisabled;
    // Set by Exit (0x35CC50) before the exit callbacks run; resources are
    // no longer released once it is set (Resource::ReleaseRef, 0x1ADEF0).
    // Name not in the reference map.
    bool mExiting;
};

static_assert(offsetof(Debug, mExiting) == 9);

// The global debug stream, at 0x19FDB40.
extern Debug TheDebug;

// Reports a failure through TheDebug.
void HmxFail(const char* msg);  // 0x35CC30
