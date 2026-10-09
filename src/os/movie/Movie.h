#pragma once

// The movie player (os/Movie.o, os/Movie_PS4.o). Only the members the
// system services call are declared; the class is not reconstructed.
class MovieMgr {
public:
    // Registers the movie resource and the PS4 decoders and creates
    // TheMovieMgr.
    static void Init();  // 0x3A4400
    static void Terminate();  // 0x3A45F0
    void Poll();  // 0x3A46C0
};

// At 0x1A03BE0.
extern MovieMgr* TheMovieMgr;
