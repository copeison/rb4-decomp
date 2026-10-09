#pragma once

#include "utl/streams/BinStream.h"

class Symbol;
class String;

enum FileMode {
    kRead = 0,
};

// Platform file. Every File* wrapper below is a thin call through one of
// these slots.
class File {
public:
    virtual ~File();                                              // slots 0-1
    virtual const char* Filename() const;                         // slot 2
    virtual long Read(void* data, unsigned long size) = 0;        // slot 3
    virtual bool ReadAsync(void* data, unsigned long size) = 0;   // slot 4
    virtual long Write(const void* data, unsigned long size) = 0; // slot 5
    virtual bool WriteAsync(const void* data, unsigned long size) = 0;  // slot 6
    virtual long Seek(long offset, int origin) = 0;               // slot 7
    virtual void Flush() = 0;                                     // slot 8
    virtual int Tell() = 0;                                       // slot 9
    virtual bool Eof() = 0;                                       // slot 10
    virtual bool Fail() = 0;                                      // slot 11
    virtual long Size() = 0;                                      // slot 12

    // File-system open routine at 0x376D40.
    static File* NewFile(const char* path, int mode);
};

// The map types these wrappers' handles as void*. Reconstructed from eboot.elf
// at 0x378940-0x378A90.
void* FileOpen(const char* path, FileMode mode);
bool FileFail(void* file);
void FileClose(void* file);
long FileRead(void* file, void* data, unsigned long size);
long FileWrite(void* file, const void* data, unsigned long size);
long FileSeek(void* file, long offset, SeekType origin);
int FileTell(void* file);
void FileFlush(void* file);
bool FileEof(void* file);
long FileSize(void* file);

// 16-byte file timestamp filled by generated-file lookups.
struct FileStat {
    long mSeconds;   // Name not in the reference map.
    long mFraction;  // Name not in the reference map.
};

// Byte at 0x19E4558. When set, generated files are trusted as shipped and
// never treated as stale; several file-system paths consult it. Name not in
// the reference map.
extern unsigned char gFileArchiveMode;

// Byte at 0x19E4559, set while the runtime is generating or consuming its
// precached data set. Name not in the reference map.
extern bool gResourcePrecacheMode;

// Path-to-symbol resolution at 0x1AF950. Any "::" suffix is ignored while the
// path is normalized, then restored. Name not in the reference map.
void FileResolvePath(Symbol& symbol, const char* path);

// Generated-file lookup at 0x1AD8B0. Resolves the generated file for a
// source path and extension, reports whether it must be rebuilt because the
// source is newer, and returns false when neither file is usable. Name not in
// the reference map.
bool FileFindGenerated(
    const Symbol& source,
    const char* extension,
    String& generatedPath,
    bool& rebuildNeeded,
    FileStat& stat);
