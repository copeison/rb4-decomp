#include "os/files/File.h"

#include <atomic>
#include <cstring>
#include <new>

#include "os/files/Archive.h"
#include "os/memory/MemMgr.h"
#include "os/system/System.h"
#include "utl/containers/List.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFunc.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The most files open at once and the files open now, at 0x19BDA54 and
// 0x19FEC04. Names not in the reference map.
int gMaxFileInstances = 100;
std::atomic<int> gNumFileInstances(0);

// The file root, the executable's root and the system data root, at
// 0x19FEE10, 0x19FF010 and 0x19FEC10. Names not in the reference map.
char gRoot[512];
char gExecRoot[512];
char gSystemRoot[512];

// The script functions file_root and file_exec_root. Names not in the
// reference map.
DataNode OnFileRoot(DataArray*) {  // 0x378850
    return DataNode(gRoot);
}

DataNode OnFileExecRoot(DataArray*) {  // 0x378870
    return DataNode(gExecRoot);
}

// The array of the strings, after an empty symbol when `withEmpty` is set.
// Name not in the reference map.
DataArray* MakeStringArray(eastl::list<String>& strings, bool withEmpty) {  // 0x377F60
    auto* array = new DataArray(strings.size() + withEmpty);
    if (withEmpty) {
        array->Node(0) = DataNode(Symbol(""));
    }
    unsigned long index = withEmpty ? 1 : 0;
    for (auto it = strings.begin(); it != strings.end(); ++it, ++index) {
        array->Node(index) = DataNode(*it);
    }
    return array;
}

// As MakeStringArray, for symbols. Name not in the reference map.
DataArray* MakeSymbolArray(eastl::list<Symbol>& symbols, bool withEmpty) {  // 0x378470
    auto* array = new DataArray(symbols.size() + withEmpty);
    if (withEmpty) {
        array->Node(0) = DataNode(Symbol(""));
    }
    unsigned long index = withEmpty ? 1 : 0;
    for (auto it = symbols.begin(); it != symbols.end(); ++it, ++index) {
        array->Node(index) = DataNode(*it);
    }
    return array;
}

}  // namespace

// Reconstructed from eboot.elf at 0x376CE0.
void File::Init() {
    AsyncFile::ClassInit();
}

// Reconstructed from eboot.elf at 0x376CF0.
int FileSetMaxFileInstances(int count) {
    gMaxFileInstances = count;
    return gNumFileInstances;
}

// Reconstructed from eboot.elf at 0x376D00.
int FileMaxFileInstances() {
    return gMaxFileInstances;
}

// Reconstructed from eboot.elf at 0x376D10.
int FileNumFileInstances() {
    return gNumFileInstances;
}

// Reconstructed from eboot.elf at 0x376D20.
bool FileManyFileInstances() {
    return 2 * gNumFileInstances >= gMaxFileInstances;
}

// Reconstructed from eboot.elf at 0x376D40.
File* File::NewFile(const char* path, int mode) {
    static_cast<void>(scePthreadSelf());
    if (path == nullptr || *path == '\0') {
        return nullptr;
    }
    const bool reading = mode == kRead || mode == kReadNoArk || mode == kReadNoBuffer;
    if (static_cast<unsigned int>(mode) <= kReadNoBuffer) {
        char buffer[512] = {};
        path = FileLocalize(path, buffer);
    }
    // Reads from a local volume never go through the archive.
    int openMode = mode;
    if (FileIsLocal(path) && reading) {
        openMode = kReadNoArk;
    }
    File* file;
    if (openMode == kReadNoArk || !(openMode == kRead || openMode == kReadNoBuffer)) {
        file = AsyncFile::New(path, openMode);
        if (file == nullptr) {
            return nullptr;
        }
    } else {
        unsigned int temp;
        MemPushTemp(temp, true, true);
        // ArkFile's 608 bytes; the class is not reconstructed.
        void* storage = MemAlloc(0x260, "ArchivedFile", 0);
        MemPopTemp(temp);
        file = new (storage) ArkFile(path, openMode);
        if (file == nullptr) {
            return nullptr;
        }
    }
    if (file->Fail()) {
        delete file;
        return nullptr;
    }
    return file;
}

// Reconstructed from eboot.elf at 0x376F20.
const char* FileLocalize(const char* path, char* buffer) {
    static Symbol sNoLanguage;
    if (sNoLanguage == Symbol()) {
        sNoLanguage = Symbol("qqq");
    }
    if (SystemLanguage() == Symbol() || SystemLanguage() == sNoLanguage) {
        return path;
    }
    // Find the "/eng/" folder.
    const char* folder = path;
    for (;;) {
        const char* slash = folder;
        while (*slash != '/') {
            if (*slash == '\0') {
                return path;
            }
            ++slash;
        }
        folder = slash + 1;
        if (slash[1] == 'e' && slash[2] == 'n' && slash[3] == 'g' && slash[4] == '/') {
            break;
        }
    }
    strcpy(buffer, path);
    const long offset = folder - path;
    const char* language = SystemLanguage().Str();
    buffer[offset + 2] = language[2];
    buffer[offset] = language[0];
    buffer[offset + 1] = language[1];
    return buffer;
}

// Reconstructed from eboot.elf at 0x377060.
File::File() : mOwner(scePthreadSelf()) {
    ++gNumFileInstances;
}

// Reconstructed from eboot.elf at 0x377090 and 0x3770B0.
File::~File() {
    --gNumFileInstances;
}

// Reconstructed from eboot.elf at 0x3770D0.
void FileSetSystemRoot(const char* root) {
    strcpy(gSystemRoot, root);
}

// Reconstructed from eboot.elf at 0x3770F0.
void FileSetRoot(const char* root) {
    strcpy(gRoot, root);
    strcpy(gExecRoot, gRoot);
    char buffer[512] = {};
    strcpy(gSystemRoot, FileMakePath(gRoot, "../../system/data", buffer));
}

// Reconstructed from eboot.elf at 0x377200.
ScopedFileRoot::ScopedFileRoot(const char* root) {
    strcpy(mPrevRoot, gRoot);
    strcpy(gRoot, root);
}

// Reconstructed from eboot.elf at 0x377230.
ScopedFileRoot::~ScopedFileRoot() {
    strcpy(gRoot, mPrevRoot);
}

// Reconstructed from eboot.elf at 0x377250.
const char* FileRoot() {
    return gRoot;
}

// Reconstructed from eboot.elf at 0x377260.
const char* FileExecRoot() {
    return gExecRoot;
}

// Reconstructed from eboot.elf at 0x377270.
const char* FileSystemRoot() {
    return gSystemRoot;
}

// Reconstructed from eboot.elf at 0x377280.
bool FileReadOnly(const char*) {
    return true;
}

// Reconstructed from eboot.elf at 0x377290.
void FileQualifiedFilename(FixedString& out, const char* path, bool normalize) {
    char buffer[512];
    FileQualifiedFilename(buffer, sizeof(buffer), path, normalize);
    out = buffer;
}

// Reconstructed from eboot.elf at 0x377350.
const char* FileStripRoot(const char* path) {
    const unsigned long length = strlen(gRoot);
    if (strncmp(gRoot, path, length) != 0) {
        return path;
    }
    return gRoot[length - 1] == '/' ? path + length : path + length + 1;
}

// Reconstructed from eboot.elf at 0x3773B0.
bool DirRecursePattern(const char* pattern, std::function<bool(const char*)> func) {
    return RecursePatternInternal(pattern, func, false);
}

// Reconstructed from eboot.elf at 0x377440.
bool RecursePatternInternal(const char* pattern, std::function<bool(const char*)> func,
                            bool files) {
    if (*pattern == '\0') {
        return false;
    }
    String path(pattern);
    bool recursive;
    unsigned long start;
    unsigned long recurse = path.find_first_of("&", 0);
    if (recurse == FixedString::npos) {
        unsigned long end = strlen(path.c_str()) - 1;
        const unsigned long wildcard = path.find_first_of("?*(", 0);
        if (wildcard < end) {
            end = wildcard;
        }
        start = end + 1;
        const unsigned long slash = path.find_first_of("/\\", start);
        recursive = slash != FixedString::npos;
        if (slash != FixedString::npos && path[wildcard] != '(') {
            // A wildcard directory: enumerate the matching directories and
            // apply the rest of the pattern in each.
            String tail(path.substr(slash, strlen(path.c_str()) - slash));
            path = path.substr(0, slash);
            char buffer[512] = {};
            String dir(FileGetPath(path.c_str(), buffer));
            return RecursePatternInternal(
                path.c_str(),
                [&dir, &tail, &func, &files](const char* found) {  // 0x378E00
                    const char* name = FileGetName(found);
                    FormatString format("%s/%s%s");
                    format << dir << name << tail;
                    return RecursePatternInternal(format.Str(), func, files);
                },
                true);
        }
    } else {
        const unsigned long wildcard = path.find_first_of("?*(", 0);
        recursive = true;
        if (wildcard < recurse) {
            recurse = wildcard;
        }
        start = recurse + 1;
    }
    const unsigned long lastSlash = path.find_last_of("/\\", start);
    String dir;
    dir = lastSlash == FixedString::npos ? String(".") : path.substr(0, lastSlash);
    if (FileIsLocal(dir.c_str())) {
        return FileEnumerate(dir.c_str(), func, recursive, path.c_str(), files);
    }
    return TheArchive->Enumerate(dir.c_str(), func, recursive, path.c_str(), files);
}

// Reconstructed from eboot.elf at 0x3778D0.
bool FileRecursePattern(const char* pattern, std::function<bool(const char*)> func) {
    return RecursePatternInternal(pattern, func, true);
}

// Reconstructed from eboot.elf at 0x377960.
int CompareFileTimes(const char* first, const char* second) {
    FileInfo firstInfo = {};
    FileInfo secondInfo = {};
    const int firstResult = FileGetStat(first, &firstInfo);
    const int secondResult = FileGetStat(second, &secondInfo);
    int flags = 0;
    if (firstResult != 0) {
        firstInfo.mModifyTime = FileStat();
    } else {
        flags = 2;
    }
    if (secondResult != 0) {
        secondInfo.mModifyTime = FileStat();
    } else {
        flags |= 4;
    }
    const FileStat& a = firstInfo.mModifyTime;
    const FileStat& b = secondInfo.mModifyTime;
    if (a.mSeconds != b.mSeconds) {
        return (a.mSeconds < b.mSeconds ? flags | 0x80000000 : flags) | 1;
    }
    int result = a.mFraction < b.mFraction ? flags | 0x80000000 : flags;
    if (a.mFraction != b.mFraction) {
        result |= 1;
    }
    return result;
}

// Reconstructed from eboot.elf at 0x377A30.
BinStream& operator<<(BinStream& stream, const FileStat& stat) {
    long value = stat.mSeconds;
    stream.WriteEndian(&value, sizeof(value));
    value = stat.mFraction;
    stream.WriteEndian(&value, sizeof(value));
    return stream;
}

// Reconstructed from eboot.elf at 0x377AA0.
BinStream& operator>>(BinStream& stream, FileStat& stat) {
    stream.ReadEndian(&stat.mSeconds, sizeof(stat.mSeconds));
    long value;
    stream.ReadEndian(&value, sizeof(value));
    stat.mFraction = value;
    return stream;
}

// Reconstructed from eboot.elf at 0x377B10.
BinStream& operator<<(BinStream& stream, const FileInfo& info) {
    unsigned int mode = info.mMode;
    stream.WriteEndian(&mode, sizeof(mode));
    int size = info.mSize;
    stream.WriteEndian(&size, sizeof(size));
    long value = info.mChangeTime.mSeconds;
    stream.WriteEndian(&value, sizeof(value));
    value = info.mChangeTime.mFraction;
    stream.WriteEndian(&value, sizeof(value));
    value = info.mAccessTime.mSeconds;
    stream.WriteEndian(&value, sizeof(value));
    value = info.mAccessTime.mFraction;
    stream.WriteEndian(&value, sizeof(value));
    value = info.mModifyTime.mSeconds;
    stream.WriteEndian(&value, sizeof(value));
    value = info.mModifyTime.mFraction;
    stream.WriteEndian(&value, sizeof(value));
    return stream;
}

// Reconstructed from eboot.elf at 0x377C10.
BinStream& operator>>(BinStream& stream, FileInfo& info) {
    stream.ReadEndian(&info.mMode, sizeof(info.mMode));
    stream.ReadEndian(&info.mSize, sizeof(info.mSize));
    stream.ReadEndian(&info.mChangeTime.mSeconds, sizeof(info.mChangeTime.mSeconds));
    long value;
    stream.ReadEndian(&value, sizeof(value));
    info.mChangeTime.mFraction = value;
    stream.ReadEndian(&info.mAccessTime.mSeconds, sizeof(info.mAccessTime.mSeconds));
    stream.ReadEndian(&value, sizeof(value));
    info.mAccessTime.mFraction = value;
    stream.ReadEndian(&info.mModifyTime.mSeconds, sizeof(info.mModifyTime.mSeconds));
    stream.ReadEndian(&value, sizeof(value));
    info.mModifyTime.mFraction = value;
    return stream;
}

// Reconstructed from eboot.elf at 0x377CF0.
DataNode MakeFileListFullPath(const char* pattern) {
    char buffer[512];
    strcpy(buffer, pattern);
    eastl::list<String> files;
    std::function<bool(const char*)> collect = [&files](const char* found) {  // 0x379000
        files.push_back(String(found));
        return false;
    };
    RecursePatternInternal(buffer, collect, false);
    files.sort();
    files.unique();
    DataArrayPtr array(MakeStringArray(files, false));
    return DataNode(array);
}

// Reconstructed from eboot.elf at 0x3781D0.
DataNode MakeFileList(const char* pattern, bool withEmpty, bool (*filter)(char*)) {
    char buffer[512];
    strcpy(buffer, pattern);
    eastl::list<Symbol> files;
    std::function<bool(const char*)> collect = [&filter, &files](const char* found) {  // 0x379570
        char path[512];
        strcpy(path, found);
        FileNormalizePath(path);
        if (filter != nullptr && !filter(path)) {
            return false;
        }
        char base[512] = {};
        files.push_back(Symbol(FileGetBase(path, base)));
        return false;
    };
    RecursePatternInternal(buffer, collect, false);
    files.sort([](const Symbol& a, const Symbol& b) { return strcmp(a.Str(), b.Str()) < 0; });
    files.unique();
    DataArrayPtr array(MakeSymbolArray(files, withEmpty));
    return DataNode(array);
}

// Reconstructed from eboot.elf at 0x3786C0.
void FileInit() {
    File::Init();
    FileQualifiedFilename(gRoot, sizeof(gRoot), "/", true);
    FileQualifiedFilename(gExecRoot, sizeof(gExecRoot), "/", true);
    FileNormalizePath(gRoot);
    FileNormalizePath(gExecRoot);
    char buffer[512] = {};
    strcpy(gSystemRoot, FileMakePath(gExecRoot, "../../system/data", buffer));
    DataRegisterFunc(Symbol("file_root"), OnFileRoot);
    DataRegisterFunc(Symbol("file_exec_root"), OnFileExecRoot);
}

// Reconstructed from eboot.elf at 0x378890.
void FileTerminate() {
    gRoot[0] = '\0';
    gExecRoot[0] = '\0';
    gSystemRoot[0] = '\0';
}

// Reconstructed from eboot.elf at 0x3788B0.
bool FileExists(const char* path, FileMode mode) {
    if (mode == kReadNoArk) {
        FileInfo info = {};
        return FileGetStat(path, &info) == 0;
    }
    File* file = File::NewFile(path, mode);
    if (file == nullptr) {
        return false;
    }
    const bool exists = !file->Fail();
    delete file;
    return exists;
}

// Reconstructed from eboot.elf at 0x378940.
void* FileOpen(const char* path, FileMode mode) {
    return File::NewFile(path, mode);
}

// Reconstructed from eboot.elf at 0x378950. A missing file reports failure.
bool FileFail(void* file) {
    if (file == nullptr) {
        return true;
    }
    return static_cast<File*>(file)->Fail();
}

// Reconstructed from eboot.elf at 0x378960.
void FileClose(void* file) {
    delete static_cast<File*>(file);
}

// Reconstructed from eboot.elf at 0x378980.
bool FileWriteable(const char* path) {
    FileInfo info = {};
    if (FileGetStat(path, &info) == 0) {
        return true;
    }
    File* file = File::NewFile(path, kAppend);
    if (file == nullptr) {
        return false;
    }
    const bool writeable = !file->Fail();
    delete file;
    return writeable;
}

// Reconstructed from eboot.elf at 0x378A10.
long FileRead(void* file, void* data, unsigned long size) {
    return static_cast<File*>(file)->Read(data, size);
}

// Reconstructed from eboot.elf at 0x378A20.
bool FileReadAsync(void* file, void* data, unsigned long size) {
    return static_cast<File*>(file)->ReadAsync(data, size);
}

// Reconstructed from eboot.elf at 0x378A40.
long FileWrite(void* file, const void* data, unsigned long size) {
    return static_cast<File*>(file)->Write(data, size);
}

// Reconstructed from eboot.elf at 0x378A50.
long FileSeek(void* file, long offset, SeekType origin) {
    return static_cast<File*>(file)->Seek(offset, origin);
}

// Reconstructed from eboot.elf at 0x378A60.
int FileTell(void* file) {
    return static_cast<File*>(file)->Tell();
}

// Reconstructed from eboot.elf at 0x378A70.
void FileFlush(void* file) {
    static_cast<File*>(file)->Flush();
}

// Reconstructed from eboot.elf at 0x378A80.
bool FileEof(void* file) {
    return static_cast<File*>(file)->Eof();
}

// Reconstructed from eboot.elf at 0x378A90.
long FileSize(void* file) {
    return static_cast<File*>(file)->Size();
}

// Reconstructed from eboot.elf at 0x378AA0.
FileStat FileTimestamp(const char* path) {
    FileInfo info = {};
    if (FileGetStat(path, &info) >= 0) {
        return info.mModifyTime;
    }
    return FileStat();
}

// Reconstructed from eboot.elf at 0x378B00.
const char* FileName(void* file) {
    return static_cast<File*>(file)->Filename();
}

// Reconstructed from eboot.elf at 0x378B10.
bool FileRename(const char* from, const char* to, bool overwrite) {
    File* file = File::NewFile(from, kReadNoBuffer);
    if (file == nullptr) {
        return false;
    }
    bool failed = file->Fail();
    delete file;
    if (failed) {
        return false;
    }
    FileInfo info = {};
    if (FileGetStat(from, &info) == 0) {
        file = File::NewFile(from, kAppend);
        if (file == nullptr) {
            return false;
        }
        failed = file->Fail();
        delete file;
        if (failed) {
            return false;
        }
    }
    FileCopy(from, to, overwrite);
    return FileDelete(from) == 0;
}

// Reconstructed from eboot.elf at 0x378C10.
bool FileCopy(const char* from, const char* to, bool overwrite) {
    const bool fromLocal = FileIsLocal(from);
    const bool toLocal = FileIsLocal(to);
    if (!fromLocal && !toLocal) {
        return false;
    }
    if (!overwrite) {
        File* existing = File::NewFile(to, kReadNoBuffer);
        if (existing != nullptr) {
            const bool failed = existing->Fail();
            delete existing;
            if (!failed) {
                return false;
            }
        }
    }
    File* input = File::NewFile(from, kRead);
    File* output = File::NewFile(to, kWrite);
    if (input != nullptr) {
        input->Fail();
    }
    if (output != nullptr) {
        output->Fail();
    }
    char buffer[4096];
    for (long bytes = input->Read(buffer, sizeof(buffer)); bytes != 0;
         bytes = input->Read(buffer, sizeof(buffer))) {
        output->Write(buffer, bytes);
    }
    delete input;
    if (output != nullptr) {
        delete output;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x378D60.
void File::SetThreadOwner() {}

// Reconstructed from eboot.elf at 0x378D70.
void File::ReleaseThreadOwner() {}
