#include <cstdio>
#include <cstring>
#include <fios2.h>
#include <kernel.h>

#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/system/System.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

// Reconstructed from eboot.elf at 0x379B50.
void FilePlatformPreInit() {}

// Reconstructed from eboot.elf at 0x379B60.
void File::Destroy() {}

// Reconstructed from eboot.elf at 0x379B70.
const char* FileDevMount() {
    ThePlatformMgr->CheckDataWritable();
    return "data:/";
}

// Reconstructed from eboot.elf at 0x379B90.
const char* FileGameMount() {
    return "app0:/";
}

// Reconstructed from eboot.elf at 0x379BA0.
const char* FileHolmesMount() {
    return "holmes:/";
}

// Reconstructed from eboot.elf at 0x379BB0.
const char* FileSaveMount() {
    return "savedata0:/";
}

// Reconstructed from eboot.elf at 0x379BC0.
bool FileIsLocal(const char* path) {
    if (strncmp(path, "data:/", 6) == 0 || strncmp(path, "download0:/", 11) == 0 ||
        strncmp(path, "savedata0:/", 11) == 0) {
        return true;
    }
    static unsigned long sAddContLength = strlen("addcont");
    StackString<512> text(path);
    return text.compare(0, sAddContLength, "addcont") == 0;
}

// Reconstructed from eboot.elf at 0x379D30.
int FileGetStat(const char* path, FileInfo* info) {
    String qualified;
    FileQualifiedFilename(qualified, path, true);
    if (strncmp(qualified.c_str(), "holmes:/", 8) == 0) {
        return -1;
    }
    DriveColonToSlash(qualified, true);
    SceKernelStat stat;
    const int result = sceKernelStat(qualified.c_str(), &stat);
    info->mChangeTime.mSeconds = stat.st_ctim.tv_sec;
    info->mChangeTime.mFraction = 0;
    info->mAccessTime.mSeconds = stat.st_atim.tv_sec;
    info->mAccessTime.mFraction = 0;
    info->mModifyTime.mSeconds = stat.st_mtim.tv_sec;
    info->mModifyTime.mFraction = 0;
    info->mMode = stat.st_mode;
    info->mSize = static_cast<int>(stat.st_size);
    return result;
}

// Reconstructed from eboot.elf at 0x379E30.
int FileDelete(const char* path) {
    String qualified;
    FileQualifiedFilename(qualified, path, true);
    int result = -1;
    if (FileIsLocal(qualified.c_str())) {
        DriveColonToSlash(qualified, true);
        result = sceKernelUnlink(qualified.c_str());
    }
    return result;
}

// Reconstructed from eboot.elf at 0x379EC0.
int FileMkDir(const char* path) {
    String qualified;
    FileQualifiedFilename(qualified, path, !gHostLogging);
    int result = -1;
    if (FileIsLocal(qualified.c_str())) {
        DriveColonToSlash(qualified, true);
        const int error = sceKernelMkdir(qualified.c_str(), 0777);
        result = 0;
        if (error != SCE_KERNEL_ERROR_EEXIST && error != 0) {
            result = error;
        }
    }
    return result;
}

// Reconstructed from eboot.elf at 0x379F70.
bool FileEnumerate(const char* dir, std::function<bool(const char*)> func, bool recursive,
                   const char* pattern, bool dirs) {
    String qualified;
    FileQualifiedFilename(qualified, dir, true);
    DriveColonToSlash(qualified, false);
    SceFiosDH handle = 0;
    SceFiosBuffer empty = {nullptr, 0};
    const SceFiosOp op = sceFiosDHOpen(nullptr, &handle, qualified.c_str(), empty);
    const int error = sceFiosOpWait(op);
    void* buffer = nullptr;
    if (error == SCE_FIOS_ERROR_BAD_SIZE) {
        // The listing needs a buffer of the size the open reports.
        const long size = sceFiosOpGetActualCount(op);
        sceFiosOpDelete(op);
        if (size != 0) {
            buffer = MemAllocTemp(size, "FileEnumerate_PS4", 0);
            SceFiosBuffer listing = {buffer, static_cast<size_t>(size)};
            if (sceFiosDHOpenSync(nullptr, &handle, qualified.c_str(), listing) == SCE_FIOS_ERROR_BAD_PATH) {
                return false;
            }
        }
    } else if (error != 0) {
        return false;
    } else {
        sceFiosOpDelete(op);
    }

    bool found = false;
    SceFiosDirEntry entry;
    memset(&entry, 0, sizeof(entry));
    while (sceFiosDHReadSync(nullptr, handle, &entry) == 0) {
        const char* name = entry.fullPath + entry.offsetToName;
        char path[520];
        sprintf(path, "%s/%s", dir, name);
        if ((entry.statFlags & SCE_FIOS_STATUS_DIRECTORY) != 0) {
            if (strcmp(name, "CVS") != 0 && strcmp(name, "gen") != 0 && strcmp(name, ".") != 0 &&
                strcmp(name, "..") != 0) {
                bool done = false;
                if (dirs && FileMatch(path, pattern, false)) {
                    char result[512];
                    FormatString format("%s/%s");
                    format << dir << name;
                    strcpy(result, format.Str());
                    done = func(result);
                }
                if (!done && recursive) {
                    done = FileEnumerate(path, func, true, pattern, dirs);
                }
                if (done) {
                    found = true;
                    break;
                }
            }
        } else if (!dirs && (pattern == nullptr || FileMatch(path, pattern, false))) {
            char result[512];
            FormatString format("%s/%s");
            format << dir << name;
            strcpy(result, format.Str());
            if (func(result)) {
                found = true;
                break;
            }
        }
        memset(&entry, 0, sizeof(entry));
    }
    sceFiosDHCloseSync(nullptr, handle);
    if (buffer != nullptr) {
        MemFree(buffer);
    }
    return found;
}

// Reconstructed from eboot.elf at 0x37A410.
void FileQualifiedFilename(char* buffer, int, const char* path, bool game) {
    char drive[512] = {};
    if (FileGetDrive(path, drive)[0] != '\0') {
        buffer[0] = '\0';
    } else {
        strcpy(buffer, game ? "app0:/" : "holmes:/");
        if (*path == '/') {
            ++path;
        }
    }
    strcat(buffer, path);
}
