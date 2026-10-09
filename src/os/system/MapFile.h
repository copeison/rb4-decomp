#pragma once

// The symbol map readers (os/MapFile.o, os/MapFile_Sce.o). Only the members
// other reconstructed code calls are declared; the classes are not
// reconstructed.
class SceMapFile {
public:
    // Registers the PS4 map file reader as the PS4 platform's factory in
    // MapFile::sFactories.
    static void Init();  // 0x36D9E0
};

class FixedString;

// The symbol map lookups (os/MapFile.o). Not reconstructed.
class MapFile {
public:
    // Appends the symbolized stack to `out` using the platform's map file
    // reader; false when the file cannot be read. The map has
    // Parse(char const*, StackData*, unsigned long, unsigned long,
    // FixedString&, HxPlatform).
    static bool Parse(const char* file, void* stack, unsigned long reference, FixedString& out,
                      int platform);  // 0x35FF50
    // A return address inside this function's caller, which the map file
    // locates by the name "MapFile::GetReferenceAddress" to find the load
    // offset.
    static unsigned long GetReferenceAddress();  // 0x35FC60
};
