#pragma once

#include "utl/streams/BinStream.h"

class Symbol;
class String;

// The modes File::NewFile opens a file in. Enumerator names not in the
// reference map: 1 reads outside the archive (FileExists uses it to test the
// disk), 3 opens for appending, which Resource::Save uses to test that the
// file is writable, and 4 writes.
enum FileMode {
    kRead = 0,
    kReadNoArk = 1,
    kReadNoBuffer = 2,
    kAppend = 3,
    kWrite = 4,
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

// Bytes 0x19E4558 and 0x19E4559, in entity/Resource.o: the map's
// Resource::sPrecached and Resource::sPrecaching. Resource::Init sets them
// from the "precached" and "precache" options. In precached mode cached
// files are trusted as shipped and never treated as stale. The names are
// kept here until their users move to the Resource members.
extern unsigned char gFileArchiveMode;
extern bool gResourcePrecacheMode;

// Whether the file exists. kReadNoArk asks the disk directly; other modes
// open the file.
bool FileExists(const char* path, FileMode mode);  // 0x3788B0
// The file's modification time, or zero when it is missing. The map's
// FileTimestamp(char const*); its return type is not in the map.
FileStat FileTimestamp(const char* path);  // 0x378AA0
// Replaces `to` with `from`. Without `overwrite`, an existing `to` is kept.
bool FileRename(const char* from, const char* to, bool overwrite);  // 0x378B10
bool FileCopy(const char* from, const char* to, bool overwrite);  // 0x378C10
// The engine's root folder.
const char* FileRoot();  // 0x377250
// The path with its "eng/" folder replaced by the system language's, in
// `buffer`, or the path itself for English.
const char* FileLocalize(const char* path, char* buffer);  // 0x376F20
// Whether the path is on a mounted PS4 volume ("data:/", "download0:/",
// "savedata0:/") or starts with "addcont". In os/FileUtl_PS4.o.
bool FileIsLocal(const char* path);  // 0x379BC0
