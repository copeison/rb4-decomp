#pragma once

#include <_pthread.h>
#include <functional>

#include "utl/streams/BinStream.h"

class DataNode;
class FixedString;
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

// A platform file (os/File.o). Every File* wrapper below is a thin call
// through one of these slots. Its constructor and destructor count the
// open files. The vtable is at 0x18FBBE8; slots 2-15 are pure.
class File {
public:
    File();  // 0x377060
    virtual ~File();  // slots 0-1: 0x377090, 0x3770B0
    virtual const char* Filename() const = 0;                          // slot 2
    virtual long Read(void* data, unsigned long size) = 0;             // slot 3
    virtual bool ReadAsync(void* data, unsigned long size) = 0;        // slot 4
    virtual long Write(const void* data, unsigned long size) = 0;      // slot 5
    virtual bool WriteAsync(const void* data, unsigned long size) = 0;  // slot 6
    virtual long Seek(long offset, int origin) = 0;                    // slot 7
    virtual void Flush() = 0;                                          // slot 8
    virtual int Tell() = 0;                                            // slot 9
    virtual bool Eof() = 0;                                            // slot 10
    virtual bool Fail() = 0;                                           // slot 11
    virtual long Size() = 0;                                           // slot 12
    // Whether the pending read or write finished, storing its byte count
    // when `bytes` is given. The map passes unsigned long&; the binary's
    // implementations test the pointer.
    virtual bool ReadDone(unsigned long* bytes) = 0;           // slot 13
    virtual bool WriteDone(unsigned long* bytes) = 0;          // slot 14
    virtual void WaitDoneReadWrite(unsigned long* bytes) = 0;  // slot 15
    // Empty in this build; the map has them inline in its Debug.o.
    virtual void SetThreadOwner();      // slot 16: 0x378D60
    virtual void ReleaseThreadOwner();  // slot 17: 0x378D70

    // Set up and shut down the platform files: Init forwards to the disk
    // files' class setup; Destroy is empty in this build.
    static void Init();     // 0x376CE0
    static void Destroy();  // 0x379B60

    // Opens the file from the archive or the disk. Reading modes localize
    // the path first; files on a local volume always come from the disk.
    // Null when the file fails to open.
    static File* NewFile(const char* path, int mode);  // 0x376D40

    // The thread that opened the file. Name not in the reference map.
    ScePthread mOwner;
};

static_assert(sizeof(File) == 16);

// A file in the game's archive. Not reconstructed. The binary allocates it
// with the label "ArchivedFile"; its slots match the map's ArkFile.
class ArkFile : public File {
public:
    ArkFile(const char* path, int mode);  // 0x3A8CB0
    ~ArkFile() override;  // slots 0-1: 0x3A8EA0, 0x3A90E0
    const char* Filename() const override;                          // slot 2: 0x3A9A40
    long Read(void* data, unsigned long size) override;             // slot 3: 0x3A9100
    bool ReadAsync(void* data, unsigned long size) override;        // slot 4: 0x3A91F0
    long Write(const void* data, unsigned long size) override;      // slot 5: 0x3A9A50
    bool WriteAsync(const void* data, unsigned long size) override;  // slot 6: 0x3A9A60
    long Seek(long offset, int origin) override;                    // slot 7: 0x3A9830
    void Flush() override;                                          // slot 8: 0x3A9A70
    int Tell() override;                                            // slot 9: 0x3A9960
    bool Eof() override;                                            // slot 10: 0x3A9A80
    bool Fail() override;                                           // slot 11: 0x3A9970
    long Size() override;                                           // slot 12: 0x3A9A90
    bool ReadDone(unsigned long* bytes) override;                   // slot 13: 0x3A9980
    bool WriteDone(unsigned long* bytes) override;                  // slot 14: 0x3A9AA0
    void WaitDoneReadWrite(unsigned long* bytes) override;          // slot 15: 0x3A8F70
};

// A file on the disk (os/AsyncFile.o). Not reconstructed.
class AsyncFile : public File {
public:
    // Allocates an AsyncFilePS4 and opens it.
    static File* New(const char* path, int mode);  // 0x3A9E20
    // Sets up the disk files' request pool.
    static void ClassInit();  // 0x3A9E10
};

// The map types these wrappers' handles as void*. Reconstructed from eboot.elf
// at 0x378940-0x378B00.
void* FileOpen(const char* path, FileMode mode);
bool FileFail(void* file);
void FileClose(void* file);
long FileRead(void* file, void* data, unsigned long size);
bool FileReadAsync(void* file, void* data, unsigned long size);
long FileWrite(void* file, const void* data, unsigned long size);
long FileSeek(void* file, long offset, SeekType origin);
int FileTell(void* file);
void FileFlush(void* file);
bool FileEof(void* file);
long FileSize(void* file);
const char* FileName(void* file);

// A file time: seconds and the fraction the platform reports, zero on the
// PS4. The map's FileStat is the larger FileGetStat result; this name is
// kept for the time the entity code compares. Field names are not in the
// reference map.
struct FileStat {
    long mSeconds;
    long mFraction;
};

// What FileGetStat reports. Name and field names not in the reference map,
// whose FileGetStat(char const*, FileStat*) fills it.
struct FileInfo {
    unsigned int mMode;
    int mSize;
    FileStat mAccessTime;
    FileStat mModifyTime;
    FileStat mChangeTime;
};

static_assert(sizeof(FileInfo) == 0x38);

BinStream& operator<<(BinStream& stream, const FileStat& stat);  // 0x377A30
BinStream& operator>>(BinStream& stream, FileStat& stat);        // 0x377AA0
BinStream& operator<<(BinStream& stream, const FileInfo& info);  // 0x377B10
BinStream& operator>>(BinStream& stream, FileInfo& info);        // 0x377C10

// Bytes 0x19E4558 and 0x19E4559, in entity/Resource.o: the map's
// Resource::sPrecached and Resource::sPrecaching. Resource::Init sets them
// from the "precached" and "precache" options. In precached mode cached
// files are trusted as shipped and never treated as stale. The names are
// kept here until their users move to the Resource members.
extern unsigned char gFileArchiveMode;
extern bool gResourcePrecacheMode;

// Limits the open file count. Returns the files open now.
int FileSetMaxFileInstances(int count);  // 0x376CF0
// The limit, the files open now, and whether at least half the limit is
// open. Names not in the reference map.
int FileMaxFileInstances();    // 0x376D00
int FileNumFileInstances();    // 0x376D10
bool FileManyFileInstances();  // 0x376D20

// The path with its "eng/" folder replaced by the system language's, in
// `buffer`, or the path itself for English.
const char* FileLocalize(const char* path, char* buffer);  // 0x376F20
void FileSetSystemRoot(const char* root);  // 0x3770D0
// Sets the root and the executable root, and the system root next to them.
void FileSetRoot(const char* root);  // 0x3770F0
// The engine's root folders.
const char* FileRoot();        // 0x377250
const char* FileExecRoot();    // 0x377260
const char* FileSystemRoot();  // 0x377270
// Always true in this build.
bool FileReadOnly(const char* path);  // 0x377280

// Switches the file root for its lifetime.
class ScopedFileRoot {
public:
    explicit ScopedFileRoot(const char* root);  // 0x377200
    ~ScopedFileRoot();                          // 0x377230

private:
    char mPrevRoot[512];  // Name not in the reference map.
};

// Writes the path, made absolute against the file root, to `out`.
void FileQualifiedFilename(FixedString& out, const char* path, bool normalize);  // 0x377290
// The path after the root, or the path itself outside the root.
const char* FileStripRoot(const char* path);  // 0x377350

// Calls `func` with each directory or file matching the pattern until it
// returns true; a wildcard directory in the pattern is expanded first. The
// map's versions take a further bool.
bool DirRecursePattern(const char* pattern, std::function<bool(const char*)> func);   // 0x3773B0
bool FileRecursePattern(const char* pattern, std::function<bool(const char*)> func);  // 0x3778D0
// The map has RecursePatternInternal(char const*, std::function<bool (char
// const*)>, bool, bool).
bool RecursePatternInternal(const char* pattern, std::function<bool(const char*)> func,
                            bool files);  // 0x377440

// Compares the files' modification times: bit 1 when only the first is
// missing, bit 2 when only the second, bit 0 when they differ and bit 31
// when the first is older.
int CompareFileTimes(const char* first, const char* second);  // 0x377960

// The sorted paths matching the pattern, as a string array.
DataNode MakeFileListFullPath(const char* pattern);  // 0x377CF0
// The sorted base names of the matching files that pass `filter`, as a
// symbol array, after an empty symbol when `withEmpty` is set.
DataNode MakeFileList(const char* pattern, bool withEmpty, bool (*filter)(char*));  // 0x3781D0

// Sets the roots from the executable's folder and registers file_root and
// file_exec_root.
void FileInit();  // 0x3786C0
// Clears the roots.
void FileTerminate();  // 0x378890

// Whether the file exists. kReadNoArk asks the disk directly; other modes
// open the file.
bool FileExists(const char* path, FileMode mode);  // 0x3788B0
// Whether the file exists or can be opened for appending.
bool FileWriteable(const char* path);  // 0x378980
// The file's modification time, or zero when it is missing. The map's
// FileTimestamp(char const*); its return type is not in the map.
FileStat FileTimestamp(const char* path);  // 0x378AA0
// Replaces `to` with `from`. Without `overwrite`, an existing `to` is kept.
bool FileRename(const char* from, const char* to, bool overwrite);  // 0x378B10
bool FileCopy(const char* from, const char* to, bool overwrite);  // 0x378C10

// os/FileUtl_PS4.o.
// Empty in this build; SystemInit calls it first. Name not in the
// reference map.
void FilePlatformPreInit();  // 0x379B50
// The PS4 volumes: the developer data volume, mounted on first use, the
// game, the Holmes host and the save data. The map's names.
const char* FileDevMount();     // 0x379B70
const char* FileGameMount();    // 0x379B90
const char* FileHolmesMount();  // 0x379BA0
const char* FileSaveMount();    // 0x379BB0
// Whether the path is on a mounted PS4 volume ("data:/", "download0:/",
// "savedata0:/") or starts with "addcont".
bool FileIsLocal(const char* path);  // 0x379BC0
// Stats the file; a negative result when it is missing. Holmes paths always
// fail.
int FileGetStat(const char* path, FileInfo* info);  // 0x379D30
// Deletes the file on a local volume.
int FileDelete(const char* path);  // 0x379E30
// Creates the directory on a local volume; an existing one is not an error.
int FileMkDir(const char* path);  // 0x379EC0
// Calls `func` with the matching entries of the directory until it returns
// true, descending into subdirectories when `recursive` is set, reporting
// directories instead of files when `dirs` is set.
bool FileEnumerate(const char* dir, std::function<bool(const char*)> func, bool recursive,
                   const char* pattern, bool dirs);  // 0x379F70
// The path under the game ("app0:/") or Holmes root, in `buffer`; a path
// with a drive is copied as it is.
void FileQualifiedFilename(char* buffer, int size, const char* path, bool game);  // 0x37A410
