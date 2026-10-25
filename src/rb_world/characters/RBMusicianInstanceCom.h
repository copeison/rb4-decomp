#pragma once

class ResourceLoader;

// A band musician placed in a venue or a shell scene
// (rb_world/RBMusicianInstanceCom.o). Only what the reconstructed code
// calls is declared; the class is not reconstructed.
class RBMusicianInstanceCom {
public:
    // Starts loading the player's musician character
    // ("char/musician/player_<n>.char", 0 to 3) once and returns its
    // loader; null for another player. The map's PreloadMusician(int)
    // returns nothing.
    static ResourceLoader* PreloadMusician(int player);  // 0xFF5B00
};
