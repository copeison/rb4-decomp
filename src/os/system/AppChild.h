#pragma once

// The pipe to a tool process that drives the game (os/AppChild.o). Only the
// members its users call are declared; the class is not reconstructed.
class AppChild {
public:
    // Opens the pipe named by the app_child options and registers the
    // app_child script functions.
    static void Init();  // 0x386D00
    static void Terminate();  // 0x386FA0

    // Writes the code to the pipe and flushes it.
    void Sync(unsigned short code);  // 0x3871B0
};

// The connected tool's pipe, or null. At 0x1A01CA8.
extern AppChild* TheAppChild;
