#pragma once

// The in-game guitar solo tutorials (rb_meta/RBIGSOnboarding.o). Only what
// the meta state calls is declared; the class is not reconstructed.
class RBIGSOnboarding {
public:
    // Whether the intro should lead to the guitar solo tutorial invite
    // rather than the main menu.
    static bool ShouldShowInvite();  // 0xB53110
};
