#pragma once

#include <cstddef>

// A song's metadata (rb_meta/SongMetadata.o, 0xF27B70-0xF29E3F): a plain
// 0x4B8-byte block of fixed-size strings and values that the song manager
// copies with memcpy and saves as raw bytes. Only the members the song
// manager uses are declared; the class is not reconstructed. The getters
// are matched to the map's by their order; names marked weak rest on less.
class SongMetadata {
public:
    SongMetadata();  // 0xF279E0 (a jump to the body at 0xF279F0)

    int GetID() const;           // 0xF28D00
    // The game the song comes from ("rb4", "rb4_rivals", ...).
    const char* GetGameOrigin() const;  // 0xF28D10
    const char* GetTitle() const;       // 0xF28D20
    const char* GetArtist() const;      // 0xF28D30
    bool IsTutorial() const;            // 0xF28DA0
    // The rank of the part (the part enumeration's values 0-6). The map has
    // GetRank(Symbol); this build takes the part's index.
    float GetRank(int part) const;  // 0xF292A0
    // Whether the part's bit is set in the part mask. The map has
    // HasPart(Symbol); this build takes the part's index.
    bool HasPart(int part) const;  // 0xF29310
    // Not IsDiscSong: the song comes from neither "rb4" nor "rb4_rivals".
    // The match to the map's IsDownload is weak.
    bool IsDownload() const;  // 0xF29590
    // The song comes from "rb4" or "rb4_rivals". Name not in the reference
    // map.
    bool IsDiscSong() const;  // 0xF295A0
    // The song comes from "rb4". Name not in the reference map.
    bool IsRB4Song() const;  // 0xF296D0
    // The song comes from "rb4_rivals". Name not in the reference map.
    bool IsRivalsSong() const;  // 0xF29790
    // A tutorial or a fake song (+0x3AC), which the song lists leave out.
    // The match to the map's IsPrivate is weak.
    bool IsPrivate() const;  // 0xF29850

    // The block's revision, saved with the song cache (12 in this build).
    static const int kRev;  // 0x133CA08

    // The fields before the short name are not modelled.
    unsigned char mOpaque0[0x3AD];
    // The song's short name: its folder and file base name. Name not in the
    // reference map.
    char mShortName[256];
    unsigned char mOpaque4AD[0x4B8 - 0x4AD];
};

static_assert(offsetof(SongMetadata, mShortName) == 0x3AD);
static_assert(sizeof(SongMetadata) == 0x4B8);
