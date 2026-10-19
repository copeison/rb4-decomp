#pragma once

class BinStream;

// The game's song manager (rb_meta/SongMgr.o): the song library and its
// cached song metadata, which the system options' save carries. Only the
// members the system options use are declared; the class is not
// reconstructed.
class SongMgr {
public:
    // Writes the song cache as a revision-12 block.
    void SaveSongCache(BinStream& stream);  // 0xF2CEE0
    // Reads the song cache written by SaveSongCache.
    void LoadSongCache(BinStream& stream);  // 0xF2D1E0
};

extern SongMgr* theSongMgr;  // 0x1B43A10
